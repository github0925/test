/*
 * Copyright (c) 2013 Corey Tabaka
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

#include <stdio.h>
#include <string.h>
#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/stats.h>

static int net_cmd(int argc, const cmd_args *argv)
{
	if (argc < 2) {
		printf("%s commands:\n", argv[0].str);
usage:
		printf("%s lookup <hostname>\n", argv[0].str);
		printf("%s ifconfig\n", argv[0].str);
		printf("%s stats\n", argv[0].str);
		printf("%s ping  <ip>\n", argv[0].str);
		printf("%s iperf <ip>\n", argv[0].str);
		goto out;
	}

#if LWIP_DNS
	if (!strcmp(argv[1].str, "lookup")) {
		if (argc < 3)
			goto usage;

		ip_addr_t ip_addr;
		const char *hostname = argv[2].str;
		err_t err;

		err = netconn_gethostbyname(hostname, &ip_addr);
		if (err != ERR_OK) {
			printf("Failed to resolve host: %d\n", err);
		} else {
			printf("%s: %u.%u.%u.%u\n", hostname,
					ip4_addr1_16(&ip_addr),
					ip4_addr2_16(&ip_addr),
					ip4_addr3_16(&ip_addr),
					ip4_addr4_16(&ip_addr));
		}
	}
#endif

    if (!strcmp(argv[1].str, "ifconfig")) {
        struct netif *netif = NULL;
        int i;
        ip_addr_t ip_addr;
        const char *hostname = argv[2].str;

        do {
            netif = netif_find_next(netif);
            if (netif) {
                printf("%s\n", netif->name);
                printf("\tHwaddr:\t%02X:%02X:%02X:%02X:%02X:%02X\n",
                        netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2],
                        netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
                printf("\tIPaddr:\t%u.%u.%u.%u\n",
                        ip4_addr1_16(&netif->ip_addr),
                        ip4_addr2_16(&netif->ip_addr),
                        ip4_addr3_16(&netif->ip_addr),
                        ip4_addr4_16(&netif->ip_addr));
                printf("\tMask:\t%u.%u.%u.%u\n",
                        ip4_addr1_16(&netif->netmask),
                        ip4_addr2_16(&netif->netmask),
                        ip4_addr3_16(&netif->netmask),
                        ip4_addr4_16(&netif->netmask));
                printf("\tMTU:\t%d\n", netif->mtu);
            }
        } while (netif);
        printf("End of Netif List\n");
    }
#if LWIP_STATS
    if (!strcmp(argv[1].str, "stats")) {
        stats_display();
    }
#endif
    if (!strcmp(argv[1].str, "ping")) {
        ip_addr_t ip_addr;
        const char *ipadd_str = argv[2].str;
        if (argc < 3) {
            printf("net ping <IPaddress>\n");
            printf("for example:\n");
            printf("net ping 192.168.10.11\n");
        }
        /* TODO: */
        printf("TODO: not implemented yet\n");
    }
    if (!strcmp(argv[1].str, "iperf")) {
        ip_addr_t ip_addr;
        const char *ipadd_str = argv[2].str;
        if (argc < 3) {
            printf("net iperf <IPaddress>\n");
            printf("lunch iperf as client, for example:\n");
            printf("net iperf 192.168.10.11\n");
        }
        /* TODO: */
       printf("TODO: not implemented yet\n");
    }

out:
	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("net", "net toolbox", &net_cmd)
STATIC_COMMAND_END(net);

#endif

