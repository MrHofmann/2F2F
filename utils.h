#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <iostream>
#include "datastructures.h"


template<typename T>
void split_channels(T *audio_buf, int len, unsigned num_channels, std::vector<std::vector<T> > &splitted_channels);

template<typename T>
void merge_channels(const std::vector<std::vector<T> > &splitted_channels, T *audio_buf, int len);

void update_volume(std::vector<std::vector<int16_t> > &splitted_channels, int balance, int master_volume);


void audio_callback(void *userdata, Uint8 *stream, int len);

int audio_decode_frame(TrackState *track_state, uint8_t *audio_buf, int buf_size);


void audio_callback2(void *userdata, Uint8 *stream, int len);

int audio_decode_frame2(TrackState *track_state, uint8_t *audio_buf, int buf_size);

#endif // UTILS_H
