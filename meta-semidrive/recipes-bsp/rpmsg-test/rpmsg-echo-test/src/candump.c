#include <can_config.h>

#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <fcntl.h>

#include <net/if.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>

#define SOCKETCAN_SUPPORT   (0)

extern int optind, opterr, optopt;

static int	s = -1;
static int	running = 1;

enum {
	VERSION_OPTION = CHAR_MAX + 1,
	FILTER_OPTION,
};

static void print_usage(char *prg)
{
        fprintf(stderr, "Usage: %s [<can-interface>] [Options]\n"
		"Options:\n"
		" -f, --family=FAMILY\t"	"protocol family (default PF_CAN = %d)\n"
		" -t, --type=TYPE\t"		"socket type, see man 2 socket (default SOCK_RAW = %d)\n"
		" -p, --protocol=PROTO\t"	"CAN protocol (default CAN_RAW = %d)\n"
		"     --filter=id:mask[:id:mask]...\n"
		"\t\t\t"			"apply filter\n"
		" -e, --error\t\t"		"dump error frames along with data frames\n"
		" -h, --help\t\t"		"this help\n"
		" -o <filename>\t\t"		"output into filename\n"
		" -d\t\t\t"			"daemonize\n"
		"     --version\t\t"		"print version information and exit\n",
		prg, PF_CAN, SOCK_RAW, CAN_RAW);
}

static void sigterm(int signo)
{
	running = 0;
}

static struct can_filter *filter = NULL;
static int filter_count = 0;

int add_filter(u_int32_t id, u_int32_t mask)
{
	filter = realloc(filter, sizeof(struct can_filter) * (filter_count + 1));
	if(!filter)
		return -1;

	filter[filter_count].can_id = id;
	filter[filter_count].can_mask = mask;
	filter_count++;

	printf("id: 0x%08x mask: 0x%08x\n",id,mask);
	return 0;
}

#define BUF_SIZ	(255)

int main(int argc, char **argv)
{
	struct canfd_frame frame;

#if SOCKETCAN_SUPPORT
	struct ifreq ifr;
	struct sockaddr_can addr;
#endif
	FILE *out = stdout;
	char *interface = "can0";
	char *optout = NULL;
	char *ptr;
	char buf[BUF_SIZ];
#if SOCKETCAN_SUPPORT
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
#endif
	int n = 0, err;
	int nbytes, i;
	int opt, optdaemon = 0;
	uint32_t id, mask;
	int error = 0;
	can_err_mask_t err_mask = (CAN_ERR_TX_TIMEOUT | CAN_ERR_LOSTARB |
					CAN_ERR_CRTL | CAN_ERR_PROT |
					CAN_ERR_TRX | CAN_ERR_ACK | CAN_ERR_BUSOFF |
					CAN_ERR_BUSERROR);

	signal(SIGPIPE, SIG_IGN);

	struct option		long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "family", required_argument, 0, 'f' },
		{ "protocol", required_argument, 0, 'p' },
		{ "type", required_argument, 0, 't' },
		{ "filter", required_argument, 0, FILTER_OPTION },
		{ "error", no_argument, 0, 'e' },
		{ "version", no_argument, 0, VERSION_OPTION},
		{ 0, 0, 0, 0},
	};

	while ((opt = getopt_long(argc, argv, "f:t:p:o:d", long_options, NULL)) != -1) {
		switch (opt) {
		case 'd':
			optdaemon++;
			break;

		case 'h':
			print_usage(basename(argv[0]));
			exit(0);

#if SOCKETCAN_SUPPORT
		case 'f':
			family = strtoul(optarg, NULL, 0);
			break;

		case 't':
			type = strtoul(optarg, NULL, 0);
			break;

		case 'p':
			proto = strtoul(optarg, NULL, 0);
			break;
#endif
		case 'e':
			error = 1;
			break;

		case 'o':
			optout = optarg;
			break;

		case FILTER_OPTION:
			ptr = optarg;
			while(1) {
				id = strtoul(ptr, NULL, 0);
				ptr = strchr(ptr, ':');
				if(!ptr) {
					fprintf(stderr, "filter must be applied in the form id:mask[:id:mask]...\n");
					exit(1);
				}
				ptr++;
				mask = strtoul(ptr, NULL, 0);
				ptr = strchr(ptr, ':');
				add_filter(id,mask);
				if(!ptr)
					break;
				ptr++;
			}
			break;

		case VERSION_OPTION:
			printf("candump %s\n",VERSION);
			exit(0);

		default:
			fprintf(stderr, "Unknown option %c\n", opt);
			break;
		}
	}

	if (optind != argc)
		interface = argv[optind];

#if SOCKETCAN_SUPPORT
	printf("interface = %s, family = %d, type = %d, proto = %d\n",
	       interface, family, type, proto);

	if ((s = open(family, type, proto)) < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = family;
	strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFINDEX, &ifr)) {
		perror("ioctl");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (filter) {
		if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, filter,
			       filter_count * sizeof(struct can_filter)) != 0) {
			perror("setsockopt");
			exit(1);
		}
	}

	if (error) {
		if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask,
			       sizeof(err_mask)) != 0) {
			perror("setsockopt");
			exit(1);
		}
	}
#else
	printf("can interface = %s\n", interface);

    s= open(interface, O_RDWR );
    if (s < 0) {
        perror("Failed to open vircan device.");
        return -1;
    }

	if (filter) {
		perror("not support filter in vircan");
		exit(1);
	}

	if (error) {
		fprintf(stderr, "not support err_mask=%x in vircan", err_mask);
		exit(1);
	}

#endif

	if (optdaemon)
		daemon(1, 0);
	else {
		signal(SIGTERM, sigterm);
		signal(SIGHUP, sigterm);
	}

	if (optout) {
		out = fopen(optout, "a");
		if (!out) {
			perror("fopen");
			exit (EXIT_FAILURE);
		}
	}

	while (running) {
		if ((nbytes = read(s, &frame, sizeof(struct canfd_frame))) < 0) {
			perror("read");
			return 1;
		} else {
			if (frame.can_id & CAN_EFF_FLAG)
				n = snprintf(buf, BUF_SIZ, "<0x%08x> ", frame.can_id & CAN_EFF_MASK);
			else
				n = snprintf(buf, BUF_SIZ, "<0x%03x> ", frame.can_id & CAN_SFF_MASK);

			n += snprintf(buf + n, BUF_SIZ - n, "[%d] ", frame.len);
			for (i = 0; i < frame.len; i++) {
				n += snprintf(buf + n, BUF_SIZ - n, "%02x ", frame.data[i]);
			}
			if (frame.can_id & CAN_RTR_FLAG)
				n += snprintf(buf + n, BUF_SIZ - n, "remote request");

			fprintf(out, "%s\n", buf);

			do {
				err = fflush(out);
				if (err == -1 && errno == EPIPE) {
					err = -EPIPE;
					fclose(out);
					out = fopen(optout, "a");
					if (!out)
						exit (EXIT_FAILURE);
				}
			} while (err == -EPIPE);

			n = 0;
		}
	}

	exit (EXIT_SUCCESS);
}
