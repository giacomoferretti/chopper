#ifndef __CHOPPER_UTILS_H__
#define __CHOPPER_UTILS_H__

#include <stddef.h>
#include <linux/nl80211.h>

int ieee80211_channel_to_frequency(int chan, enum nl80211_band band);
int ieee80211_frequency_to_channel(int freq);
size_t parse_channels_string(int **dst, char *src);
char *int_array_as_string(int *src, size_t size);

#endif // __CHOPPER_UTILS_H__
