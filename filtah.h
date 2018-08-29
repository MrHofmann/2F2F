#ifndef FILTAH_H
#define FILTAH_H

#include <vector>


class BaseFilter
{
public:
    enum Type {EQUALIZER, LOW_PASS, HIGH_PASS, BAND_PASS, BAND_STOP};
    enum Method {NONE, CONV, FFT, OA_CONV, OA_FFT};

    virtual Type type() const = 0;
    virtual void set_kernel() = 0;
    virtual ~BaseFilter(){}

    template<typename T>
    std::vector<T> convolve(const std::vector<T> signal, Method method) const;
};

class Equalizer : public BaseFilter
{
public:
    Equalizer(const std::vector<double> kernel, unsigned size);

    virtual Type type() const;
    virtual void set_kernel();

private:
    std::vector<double> _kernel;
    unsigned _kernel_size;
};

class LowPass : public BaseFilter
{
public:
    LowPass(double gain, double cutoff);

    virtual Type type() const;
    virtual void set_kernel();

private:
    std::vector<double> _kernel;
    unsigned _kernel_size;

    double _gain;
    double _cutoff;
};

class HighPass : public BaseFilter
{
public:
    HighPass(double gain, double cutoff);

    virtual Type type() const;
    virtual void set_kernel();

private:
    std::vector<double> _kernel;
    unsigned _kernel_size;

    double _gain;
    double _cutoff;
};

class BandPass : public BaseFilter
{
public:
    BandPass(double gain, double mean, double width);

    virtual Type type() const;
    virtual void set_kernel();

private:
    std::vector<double> _kernel;
    unsigned _kernel_size;

    double _gain;
    double _mean;
    double _width;
};

#endif // FILTAH_H
