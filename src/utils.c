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

#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Function took from https://git.kernel.org/pub/scm/linux/kernel/git/jberg/iw.git/tree/util.c#n164 */
int ieee80211_channel_to_frequency(int chan, enum nl80211_band band) {
    /* see 802.11 17.3.8.3.2 and Annex J
     * there are overlapping channel numbers in 5GHz and 2GHz bands */
    if (chan <= 0)
        return 0; /* not supported */
    switch (band) {
        case NL80211_BAND_2GHZ:
            if (chan == 14)
                return 2484;
            else if (chan < 14)
                return 2407 + chan * 5;
            break;
        case NL80211_BAND_5GHZ:
            if (chan >= 182 && chan <= 196)
                return 4000 + chan * 5;
            else
                return 5000 + chan * 5;
            break;
        case NL80211_BAND_6GHZ:
            /* see 802.11ax D6.1 27.3.23.2 */
            if (chan == 2)
                return 5935;
            if (chan <= 253)
                return 5950 + chan * 5;
            break;
        case NL80211_BAND_60GHZ:
            if (chan < 7)
                return 56160 + chan * 2160;
            break;
        default:;
    }
    return 0; /* not supported */
}

/* Function took from https://git.kernel.org/pub/scm/linux/kernel/git/jberg/iw.git/tree/util.c#n200 */
int ieee80211_frequency_to_channel(int freq) {
    /* see 802.11-2007 17.3.8.3.2 and Annex J */
    if (freq == 2484)
        return 14;
        /* see 802.11ax D6.1 27.3.23.2 and Annex E */
    else if (freq == 5935)
        return 2;
    else if (freq < 2484)
        return (freq - 2407) / 5;
    else if (freq >= 4910 && freq <= 4980)
        return (freq - 4000) / 5;
    else if (freq < 5950)
        return (freq - 5000) / 5;
    else if (freq <= 45000) /* DMG band lower limit */
        /* see 802.11ax D6.1 27.3.23.2 */
        return (freq - 5950) / 5;
    else if (freq >= 58320 && freq <= 70200)
        return (freq - 56160) / 2160;
    else
        return 0;
}

size_t parse_channels_string(int **dst, char *src) {
    size_t length = strlen(src);

    *dst = malloc(sizeof(int) * length);
    if (!*dst) {
        return -1;
    }

    size_t idx = 0;
    char *ptr = strtok(src, ",");
    while (ptr != NULL) {
        (*dst)[idx] = (int) strtol(ptr, NULL, 10);
        ptr = strtok(NULL, ",");
        idx++;
    }

    return idx;
}

char *int_array_as_string(int *src, size_t size) {
    char *ret = malloc(3 + 14 * size);
    if (!ret) {
        return NULL;
    }

    sprintf(ret, "[%d", src[0]);
    for (size_t i = 1; i < size; i++) {
        sprintf(ret + strlen(ret), ", %d", src[i]);
    }
    sprintf(ret + strlen(ret), "]");

    return ret;
}
