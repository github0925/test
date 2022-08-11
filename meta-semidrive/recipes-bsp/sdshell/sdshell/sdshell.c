#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdio_ext.h>
#include "rpmsg_socket.h"
#include "linenoise/linenoise.h"
#include "sdshell.h"

const static target_desc_t target_list[] = {
	{"safety", SDRV_SAFETY, DP_CR5_SAF},
	{"secure", SDRV_SECURE, DP_CR5_SEC},
	{"mp"    , SDRV_MP,	DP_CR5_MPC},
	{"ap1"   , SDRV_AP1,	DP_CR5_SAF}, /* DP_CA_AP1 */
	{"ap2"   , SDRV_AP2,	DP_CR5_SAF}  /* DP_CA_AP2 */
};

int is_empty(char *name)
{
	return (strlen(name) == 0);
}

static void usage(void)
{
	fprintf(stdout, "Usage: sdshell [safety | secure]\n");
}

static const target_desc_t *sdshell_find_target(char *name)
{
	int i;

	if (is_empty(name))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(target_list); i++) {
		if (strcasecmp(target_list[i].name, name) == 0) {
			return &target_list[i];
		}
	}

	return NULL;
}

static int sdshell_is_serial_tty(void)
{
	FILE *f;
	char *str;
	char tbuf[32] = {0};

	f = popen("tty", "r");
	if (!f) {
		debug(LOG_ERROR, "failed to popen\n");
		return FAIL;
	}
	fread(tbuf, 1, 32, f);
	fclose(f);

	str = strstr(tbuf, "ttyS");
	if (str)
		return 1;

	return 0;
}

static int sdshell_kernel_printk(sdshell_ctl_t *ctl, int off)
{
	int ffd;
	char level[8] = {0};
	char fbuf[32] = {0};
	int llv;

	if (ctl->ttys) {
		ffd = open(PRINTK_DIR, O_RDWR);
		if (ffd < 0) {
			debug(LOG_ERROR, "failed to fopen %s\n", PRINTK_DIR);
			return FAIL;
		}
		if (off) {
			read(ffd, fbuf, 32);
			sscanf(fbuf, "%d", &llv);
			ctl->dlevel = (uint8_t)llv; /* save the last log level */
			snprintf(level, sizeof(level), "%d\n", 0);
		} else {
			snprintf(level, sizeof(level), "%d\n", ctl->dlevel);
		}
		lseek(ffd, 0, SEEK_SET);
		write(ffd, level, sizeof(level));
		close(ffd);
	}

	return 0;
}

static int sdshell_skt_init(sdshell_ctl_t *ctl)
{
	int ret;
	struct sockaddr_rpmsg addr;

	ctl->fd = socket(PF_RPMSG, SOCK_SEQPACKET, 0);
	if (ctl->fd < 0) {
		debug(LOG_ERROR, "failed to open socket\n");
		return FAIL;
	}

	addr.family = AF_RPMSG;
	addr.vproc_id =  ctl->tgt->rpmsg_id;
	addr.addr = SDSHELL_SERVICE_EPT;

	ret = connect(ctl->fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		debug(LOG_ERROR, "failed to connect socket\n");
		return FAIL;
	}

	return 0;
}

static int sdshell_skt_send(int fd, char *buf, int len)
{
	int ret;

	if (len > MAX_RPMSG_BUFF_SIZE) {
		debug(LOG_ERROR, "length of sending data is invalid");
		return FAIL;
	}

	ret = write(fd, buf, len);
	if (ret < 0) {
		debug(LOG_ERROR, "Error sending data\n");
		return FAIL;
	}

	return 0;
}

static int sdshell_go_direct_mode(sdshell_ctl_t *ctl, int argc, char **argv)
{
	return 0;
}

static int sdshell_send_cmd(sdshell_ctl_t *ctl, int mtype, char *buf, int len)
{
	sdshell_msg_t *msg;
	int size = sizeof(*msg) + len;

	msg = malloc(size);
	if (!msg) {
		debug(LOG_ERROR, "failed to alloc memory for msg\n");
		return FAIL;
	}
	memset((void *)msg, 0, size);
	msg->type = mtype;
	msg->size = len;
	if (buf)
		memcpy((char *)msg->data, buf, len);

	sdshell_skt_send(ctl->fd, (char *)msg, size);
	free(msg);

	return 0;
}

static int sdshell_quit(sdshell_ctl_t *ctl)
{
	sdshell_send_cmd(ctl, SDSHELL_MSG_QUIT, NULL, 0);
	usleep(100000); /* delay 100ms */
	close(ctl->fd);
	usleep(100000); /* delay 100ms */
	sdshell_kernel_printk(ctl, KERNEL_LOG_ON);
	exit(0);
}

static int sdshell_parse_interactive_cmd(sdshell_ctl_t *ctl, char *buf, int len)
{
	if (strcasecmp(buf, "logshow") == 0)
		sdshell_send_cmd(ctl, SDSHELL_MSG_READ, NULL, 0);
	else if (strcasecmp(buf, "logclear") == 0)
		sdshell_send_cmd(ctl, SDSHELL_MSG_CLEAR, NULL, 0);
	else if (strcasecmp(buf, "xon") == 0)
		sdshell_send_cmd(ctl, SDSHELL_MSG_PRINTON, NULL, 0);
	else if (strcasecmp(buf, "xoff") == 0)
		sdshell_send_cmd(ctl, SDSHELL_MSG_PRINTOFF, NULL, 0);
	else if (strcasecmp(buf, "quit") == 0) {
		sdshell_quit(ctl);
	} else {
		sdshell_send_cmd(ctl, SDSHELL_MSG_RUN_CMD, buf, len);
	}

	return 0;
}

static void *sdshell_thread_handler(void *arg)
{
	sdshell_ctl_t *ctl = (sdshell_ctl_t *)arg;
	char *buf;
	int rbytes;
	int i;

	buf = malloc(MAX_RPMSG_BUFF_SIZE);
	if (!buf) {
		debug(LOG_ERROR, "failed to alloc memory for recv buffer\n");
		exit(EXIT_FAILURE);
	}
	memset(buf, 0, MAX_RPMSG_BUFF_SIZE);

	while(1) {
		rbytes = read(ctl->fd, buf, MAX_RPMSG_BUFF_SIZE);

		for(i = 0; i < rbytes; i++) {
			putchar(buf[i]);
			if (buf[i] == '\n') {
				putchar('\r');
				fprintf(stdout, "%s ] ", ctl->tgt->name);
				fflush(stdout);
			}
		}
	}
	free(buf);

	return 0;
}

static int sdshell_go_interactive_mode(sdshell_ctl_t *ctl, int argc, char **argv)
{
	pthread_t tid;
	pthread_attr_t attr;
	char prompt[SDSHELL_PROMPT_LEN] = {0};
	char *line;

	ctl->tgt = sdshell_find_target(argv[1]);
	if (!ctl->tgt) {
		debug(LOG_ERROR, "invalid target: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (sdshell_skt_init(ctl))
		exit(EXIT_FAILURE);

	/* declare the core type */
	sdshell_send_cmd(ctl, SDSHELL_MSG_TARGET_SET, (char *)&ctl->tgt->core_id,
		       sizeof(ctl->tgt->core_id));

	/* create thread to get the log from ring buffer */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&tid, &attr, sdshell_thread_handler, ctl)) {
		debug(LOG_ERROR, "create thread failed\n");
		exit(EXIT_FAILURE);
	}

	snprintf(prompt, SDSHELL_PROMPT_LEN, "%s ] ", ctl->tgt->name);

	/* input shell console */
	while((line = linenoise(prompt)) != NULL) {
		if (line[0] != '\0') {
			linenoiseHistoryAdd(line); /* add to the history. */
			sdshell_parse_interactive_cmd(ctl, line, strlen(line));
		}
		free(line);
	}
	return 0;
}

int main(int argc, char **argv)
{
	sdshell_ctl_t *ctl;
	int run_mode;

	if (argc == 2) {
		if ((strcasecmp(argv[1], "safety") != 0) &&
		    (strcasecmp(argv[1], "secure") != 0)) {
			usage();
			exit(EXIT_FAILURE);
		}
		run_mode = 1;
	} else { /* to do */
		run_mode = 0;
		usage();
		exit(0);
	}

	ctl = malloc(sizeof(*ctl));
	if (!ctl) {
		debug(LOG_ERROR, "out of memory (malloc)");
		exit(EXIT_FAILURE);
	}
	memset(ctl, 0, sizeof(*ctl));

	ctl->ttys = sdshell_is_serial_tty();
	if (ctl->ttys < 0)
		ctl->ttys = 0;

	sdshell_kernel_printk(ctl, KERNEL_LOG_OFF);

	if (run_mode) {
		sdshell_go_interactive_mode(ctl, argc, argv);
	} else {
		sdshell_go_direct_mode(ctl, argc, argv);
	}

	sdshell_kernel_printk(ctl, KERNEL_LOG_ON);

	return 0;
}
