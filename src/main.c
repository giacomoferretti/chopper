/*
 * Copyright 2021 Giacomo Ferretti
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <net/if.h>
#include <stdbool.h>
#include <linux/nl80211.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

//#include "cJSON.h"
#include "config.h"
#include "utils.h"

static struct option long_options[] = {
        {"help",      no_argument,       NULL, 'h'},
        {"version",   no_argument,       NULL, 'V'},
        //{"verbose",   required_argument, NULL, 'v'},
        //{"quiet",     required_argument, NULL, 'q'},
        //{"config",    required_argument, NULL, 'f'},
        {"interface", required_argument, NULL, 'i'},
        {"channels",  required_argument, NULL, 'c'},
        {"delay",     required_argument, NULL, 'd'},
        {"timeout",   required_argument, NULL, 't'},
        {0, 0, 0, 0}
};

static void print_usage(const char *executable) {
    printf(PROGRAM_NAME " v" VERSION " (C) 2021 Giacomo Ferretti\n\n");
    printf("Usage: %s [OPTION [ARG]] ...\n\n", executable);
    printf("Options:\n");
    printf("  -h, --help                 show this help message\n");
    printf("  -V, --version              show version\n");
    //printf("  -v, --verbose              verbose\n");
    //printf("  -q, --quiet                quiet\n");
    //printf("  -f, --config <file>        read config file\n");
    printf("  -i, --interface <string>   interface name (must be in monitor mode)\n");
    printf("  -c, --channels <list>      comma-separated list of channels\n");
    printf("                             (default: 1,8,2,9,3,10,4,11,5,12,6,13,7)\n");
    printf("  -d, --delay <int>          delay between each hop (default: 200)\n");
    printf("  -t, --timeout <int>        exit the program after X seconds (default: 0)\n");
}

static const int default_channels[] = {1, 8, 2, 9, 3, 10, 4, 11, 5, 12, 6, 13, 7};
static volatile bool running = true;

void catch_signal() {
    running = false;
}

int main(int argc, char *argv[]) {
    signal(SIGALRM, catch_signal);
    signal(SIGINT, catch_signal);

    int nl_res = 0;

    int delay = 200;
    int timeout = 0;
    char *iface = NULL;
    int *channels = NULL;
    size_t channels_num = 0;

    // Parse options
    int ch;
    while ((ch = getopt_long(argc, argv, "hVvqf:i:c:d:t:", long_options, NULL)) != -1) {
        switch (ch) {
            case 'V':
                printf(PROGRAM_NAME " v" VERSION "\n");
                return EXIT_SUCCESS;
            /*case 'v':
                break;
            case 'q':
                break;
            case 'f':
                break;*/
            case 'i':
                iface = strdup(optarg);
                break;
            case 'c':
                channels_num = parse_channels_string(&channels, optarg);
                if (channels != NULL) {
                    for (size_t i = 0; i < channels_num; i++) {
                        printf("%d\n", channels[i]);
                    }
                }
                break;
            case 'd':
                delay = (int) strtoul(optarg, NULL, 10);
                if (delay < 10) {
                    fprintf(stderr, "WARNING: the delay is very small, why are you doing this?\n");
                }
                break;
            case 't':
                timeout = (int) strtoul(optarg, NULL, 10);
                if (timeout <= 0) {
                    fprintf(stderr, "WARNING: timeout cannot be 0, running until SIGINT.\n");
                    break;
                }
                alarm(timeout);
                break;
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Check necessary arguments
    if (!iface) {
        fprintf(stderr, "ERROR: You need to specify an interface.\n");
        return EXIT_FAILURE;
    }

    // Setup default values
    if (!channels) {
        channels_num = sizeof(default_channels) / sizeof(int);
        channels = malloc(sizeof(default_channels));
        for (size_t i = 0; i < channels_num; i++) {
            channels[i] = default_channels[i];
        }
    }

    // Connect generic netlink socket
    struct nl_sock *sckt = nl_socket_alloc();
    if ((nl_res = genl_connect(sckt)) != 0) {
        fprintf(stderr, "ERROR: Cannot connect to Netlink: %s\n", nl_geterror(nl_res));
        return EXIT_FAILURE;
    }

    // Resolve nl80211
    int nl80211_family = 0;
    if ((nl_res = genl_ctrl_resolve(sckt, "nl80211")) < 0) {
        fprintf(stderr, "ERROR: Cannot resolve nl80211: %s\n", nl_geterror(nl_res));
        return EXIT_FAILURE;
    }
    nl80211_family = nl_res;

    struct nl_msg *mesg = NULL;
    size_t idx = 0;
    while (running) {
        int frequencyMhz = ieee80211_channel_to_frequency(channels[idx], NL80211_BAND_2GHZ);

        // Prepare message
        mesg = nlmsg_alloc();
        enum nl80211_commands command = NL80211_CMD_SET_CHANNEL;
        genlmsg_put(mesg, 0, 0, nl80211_family, 0, 0, command, 0);
        NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(iface));
        NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_FREQ, frequencyMhz);

        // TODO: Add support for HT20, HT40+, HT40-
        NLA_PUT_U32(mesg, NL80211_ATTR_CHANNEL_WIDTH, NL80211_CHAN_WIDTH_20_NOHT);
        NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_CHANNEL_TYPE, NL80211_CHAN_NO_HT);

        // Send message
        if ((nl_res = nl_send_auto(sckt, mesg)) < 0) {
            fprintf(stderr, "ERROR: Something went wrong while sending: %s\n", nl_geterror(nl_res));
        }
        nlmsg_free(mesg);

        // Check for errors
        if ((nl_res = nl_recvmsgs_default(sckt)) != 0) {
            fprintf(stderr, "ERROR: Something went wrong while receiving: %s\n", nl_geterror(nl_res));
        }

        // Cycle channels
        idx++;
        if (idx >= channels_num) {
            idx = 0;
        }

        // Delay
        struct timespec ts;
        ts.tv_sec = delay / 1000;
        ts.tv_nsec = (delay % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }

    free(iface);
    free(channels);
    nl_socket_free(sckt);
    return EXIT_SUCCESS;

    nla_put_failure:
    free(iface);
    free(channels);
    nlmsg_free(mesg);
    nl_socket_free(sckt);
    fprintf(stderr, "NLA_PUT_U32 error\n");
    return EXIT_FAILURE;
}
