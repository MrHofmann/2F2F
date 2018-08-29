#ifndef FILTAH_H
#define FILTAH_H

#include <vector>


class BaseFilter
{
public:
    enum Type {EQUALIZER, LOW_PASS, HIGH_PASS, BAND_PASS, BAND_STOP};
    enum Method {NONE, CONV, FFT, OA_CONV, OA_FFT};

    BaseFilter(){}
    BaseFilter(unsigned kernel_size, double sample_rate);

    virtual Type type() const = 0;
    virtual void update_kernel() = 0;
    virtual ~BaseFilter(){}

    void set_kernel_size(unsigned kernel_size);
    void set_sample_rate(double sample_rate);

    template<typename T>
    std::vector<T> convolve(const std::vector<T> signal, Method method) const;

protected:
    std::vector<double> _kernel;
    unsigned _kernel_size;
    double _sample_rate;
};

class Equalizer : public BaseFilter
{
public:
    Equalizer(){}
    Equalizer(unsigned size, double sample_rate, std::vector<int> *f_gain);

    virtual Type type() const;
    virtual void update_kernel();

private:
    std::vector<int> *_f_gain;
};

class LowPass : public BaseFilter
{
public:
    LowPass(){}
    LowPass(unsigned kernel_size, double sample_rate, unsigned *order, double *cutoff, double *gain);

    virtual Type type() const;
    virtual void update_kernel();

private:
    unsigned *_order;
    double *_cutoff;
    double *_gain;
};

class HighPass : public BaseFilter
{
public:
    HighPass(){}
    HighPass(unsigned size, double sample_rate, unsigned *order, double *cutoff, double *gain);

    virtual Type type() const;
    virtual void update_kernel();

private:
    unsigned *_order;
    double *_cutoff;
    double *_gain;
};

class BandPass : public BaseFilter
{
public:
    BandPass(){}
    BandPass(unsigned size, double sample_rate,  unsigned *order,
             double *mean, double *width, double *gain);

    virtual Type type() const;
    virtual void update_kernel();

private:
    unsigned *_order;
    double *_mean;
    double *_width;
    double *_gain;

    double _cutoff_lp;
    double _cutoff_hp;
};

#endif // FILTAH_H
