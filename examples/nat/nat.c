/*********************************************************************
 *                     openNetVM
 *              https://sdnfv.github.io
 *
 *   BSD LICENSE
 *
 *   Copyright(c)
 *            2015-2019 George Washington University
 *            2015-2019 University of California Riverside
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * The name of the author may not be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * forward.c - an example using onvm. Forwards packets to a DST NF.
 ********************************************************************/

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <rte_common.h>
#include <rte_ip.h>
#include <rte_mbuf.h>

#include "onvm_nflib.h"
#include "onvm_pkt_helper.h"

#define NF_TAG "NAT"

/* number of package between each print */
static uint32_t print_delay = 1000000;

static uint32_t destination;
static int num_sh_entries = 0;
static struct {
        struct in_addr ip;
        uint32_t destination;
} sh_entries[10];


#define MAX_NAT_ENTRIES 100

struct nat_table_entry {
    struct in_addr src_ip;
    struct in_addr new_src_ip;
};
static struct nat_table_entry nat_table[MAX_NAT_ENTRIES];
static int num_nat_entries = 0;

/*
 * Print a usage message
 */
static void
usage(const char *progname) {
        printf("Usage:\n");
        printf("%s [EAL args] -- [NF_LIB args] -- -d <destination> -p <print_delay> -sh <ip1> <nf1> <ip2> <nf2> ...\n", progname);
        printf("%s -F <CONFIG_FILE.json> [EAL args] -- [NF_LIB args] -- [NF args]\n\n", progname);
        printf("Flags:\n");
        printf(" - `-d <dst>`: destination service ID to forward to\n");
        printf(" - `-p <print_delay>`: number of packets between each print, e.g. `-p 1` prints every packets.\n");
        printf(" - `-sh <ip1> <nf1> <ip2> <nf2> ...`: specify source IP and destination NF pairs\n");
}


/*
 * Parse the application arguments.
 */
/*
 * Parse the application arguments.
 */
static int
parse_app_args(int argc, char *argv[], const char *progname) {
        int c, dst_flag = 0;
        char *endptr;

        while ((c = getopt(argc, argv, "d:p:s:")) != -1) {
                switch (c) {
                        case 'd':
                                destination = strtoul(optarg, NULL, 10);
                                dst_flag = 1;
                                break;
                        case 'p':
                                print_delay = strtoul(optarg, NULL, 10);
                                break;
                        case 's':
                                while (optind < argc && argv[optind][0] != '-') {
                                        if (num_sh_entries >= 10) {
                                                RTE_LOG(INFO, APP, "Too many -sh entries, maximum is 10\n");
                                                return -1;
                                        }
                                        if (inet_aton(argv[optind], &sh_entries[num_sh_entries].ip) == 0) {
                                                RTE_LOG(INFO, APP, "Invalid IP address %s\n", argv[optind]);
                                                return -1;
                                        }
                                        optind++;
                                        if (optind < argc) {
                                                sh_entries[num_sh_entries].destination = strtoul(argv[optind], &endptr, 10);
                                                if (*endptr != '\0') {
                                                        RTE_LOG(INFO, APP, "Invalid NF destination %s\n", argv[optind]);
                                                        return -1;
                                                }
                                                num_sh_entries++;
                                        }
                                        optind++;
                                }
                                break;
                        case '?':
                                usage(progname);
                                if (optopt == 'd')
                                        RTE_LOG(INFO, APP, "Option -%c requires an argument.\n", optopt);
                                else if (optopt == 'p')
                                        RTE_LOG(INFO, APP, "Option -%c requires an argument.\n", optopt);
                                else if (isprint(optopt))
                                        RTE_LOG(INFO, APP, "Unknown option `-%c'.\n", optopt);
                                else
                                        RTE_LOG(INFO, APP, "Unknown option character `\\x%x'.\n", optopt);
                                return -1;
                        default:
                                usage(progname);
                                return -1;
                }
        }

        if (!dst_flag && num_sh_entries == 0) {
                RTE_LOG(INFO, APP, "Simple Forward NF requires destination flag -d or -sh.\n");
                return -1;
        }

        return optind;
}



/*
 * This function displays stats. It uses ANSI terminal codes to clear
 * screen when called. It is called from a single non-master
 * thread in the server process, when the process is run with more
 * than one lcore enabled.
 */
static void
do_stats_display(struct rte_mbuf *pkt) {
        const char clr[] = {27, '[', '2', 'J', '\0'};
        const char topLeft[] = {27, '[', '1', ';', '1', 'H', '\0'};
        static uint64_t pkt_process = 0;
        struct rte_ipv4_hdr *ip;

        pkt_process += print_delay;

        /* Clear screen and move to top left */
        printf("%s%s", clr, topLeft);

        printf("PACKETS\n");
        printf("-----\n");
        printf("Port : %d\n", pkt->port);
        printf("Size : %d\n", pkt->pkt_len);
        printf("N°   : %" PRIu64 "\n", pkt_process);
        printf("\n\n");

        ip = onvm_pkt_ipv4_hdr(pkt);
        if (ip != NULL) {
                onvm_pkt_print(pkt);
        } else {
                printf("No IP4 header found\n");
        }
}

static struct in_addr *find_nat_entry(struct in_addr src_ip) {
    for (int i = 0; i < num_nat_entries; i++) {
        if (memcmp(&nat_table[i].src_ip, &src_ip, sizeof(src_ip)) == 0) {
            return &nat_table[i].new_src_ip;
        }
    }
    return NULL;
}

static void add_nat_entry(struct in_addr src_ip, struct in_addr new_src_ip) {
    if (num_nat_entries < MAX_NAT_ENTRIES) {
        memcpy(&nat_table[num_nat_entries].src_ip, &src_ip, sizeof(src_ip));
        memcpy(&nat_table[num_nat_entries].new_src_ip, &new_src_ip, sizeof(new_src_ip));
        num_nat_entries++;
    } else {
        printf("NAT table is full, cannot add new entry\n");
    }
}

static int packet_handler(struct rte_mbuf *pkt, struct onvm_pkt_meta *meta, __attribute__((unused)) struct onvm_nf_local_ctx *nf_local_ctx) {
    static uint32_t counter = 0;
    static uint32_t saved_destination = 0;
    struct rte_ipv4_hdr *ip_hdr;
    int i = 0;

    if (++counter == print_delay) {
        do_stats_display(pkt);
        counter = 0;
    }

    if (saved_destination == 0) {
        saved_destination = destination;
    }

    ip_hdr = onvm_pkt_ipv4_hdr(pkt);
    if (ip_hdr != NULL) {
        struct in_addr src_ip;
        src_ip.s_addr = ip_hdr->src_addr;

        for (i = 0; i < num_sh_entries; i++) {
                if (ip_hdr->src_addr == sh_entries[i].ip.s_addr) {
                        meta->destination = sh_entries[i].destination;
                        break;
                }
        }
        if (i == num_sh_entries) {
                meta->destination = destination;
        }

        struct in_addr *new_src_ip = find_nat_entry(src_ip);
        if (new_src_ip == NULL) {
            static uint32_t new_ip_suffix = 200;
            struct in_addr new_ip;
            new_ip.s_addr = htonl(ntohl(ip_hdr->src_addr) | (new_ip_suffix++));
            add_nat_entry(src_ip, new_ip);
            new_src_ip = &new_ip;
        }
        ip_hdr->src_addr = new_src_ip->s_addr;
    }

    if (meta->destination == 0) {
        meta->destination = saved_destination;
    }
    
    meta->action = ONVM_NF_ACTION_TONF;
    return 0;
}






int
main(int argc, char *argv[]) {
        struct onvm_nf_local_ctx *nf_local_ctx;
        struct onvm_nf_function_table *nf_function_table;
        int arg_offset;

        const char *progname = argv[0];

        nf_local_ctx = onvm_nflib_init_nf_local_ctx();
        onvm_nflib_start_signal_handler(nf_local_ctx, NULL);

        nf_function_table = onvm_nflib_init_nf_function_table();
        nf_function_table->pkt_handler = &packet_handler;

        if ((arg_offset = onvm_nflib_init(argc, argv, NF_TAG, nf_local_ctx, nf_function_table)) < 0) {
                onvm_nflib_stop(nf_local_ctx);
                if (arg_offset == ONVM_SIGNAL_TERMINATION) {
                        printf("Exiting due to user termination\n");
                        return 0;
                } else {
                        rte_exit(EXIT_FAILURE, "Failed ONVM init\n");
                }
        }

        argc -= arg_offset;
        argv += arg_offset;

        if (parse_app_args(argc, argv, progname) < 0) {
                onvm_nflib_stop(nf_local_ctx);
                rte_exit(EXIT_FAILURE, "Invalid command-line arguments\n");
        }

        onvm_nflib_run(nf_local_ctx);

        onvm_nflib_stop(nf_local_ctx);
        printf("If we reach here, program is ending\n");
        return 0;
}
