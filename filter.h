#ifndef FILTER_H
#define FILTER_H

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <complex>
#include "fft.h"

class Filter
{
protected:
    double f_min;
    double f_max;
    double gain;
    double *kernel;
    int kernel_size;
public:
    Filter(int len);
    ~Filter();
    Filter(double min, double max, double g, int len);
    void set_kernel();
    void dft_filter(uint8_t *samples, int len);
    void dft_filter16(int16_t *samples, int len);
};

#endif // FILTER_H
