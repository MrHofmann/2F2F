#ifndef FILTAH_H
#define FILTAH_H

#include <vector>
#include <iostream>
#include <complex>

class Filter
{
public:
    enum Type {EQUALIZER, LOW_PASS, HIGH_PASS, BAND_PASS, BAND_STOP};
    enum Method {NONE, CONV, FFT, OA_CONV, OA_FFT};

    Filter(){}
    Filter(unsigned kernel_size, double sample_rate);
    virtual ~Filter(){std::cout << "DELETE F" << std::endl;}

    virtual Type type() const = 0;
    virtual void update_kernel() = 0;

    std::vector<double> kernel() const;
    double sample_rate() const;
    void set_kernel_size(unsigned kernel_size);
    void set_sample_rate(double sample_rate);
    void update_params(unsigned order, double cutoff, double cutoff_lp, double cutoff_hp, double gain);

    template<typename T>
    const std::vector<T> convolve(const std::vector<T> signal, std::vector<std::complex<double> > &freqs, Method method) const;

private:


protected:
    std::vector<double> _kernel;
    unsigned _kernel_size;
    double _sample_rate;
};

class Equalizer : public Filter
{
public:
    Equalizer(){}
    Equalizer(unsigned size, double sample_rate, std::vector<int> *f_gain);
    virtual ~Equalizer(){ std::cout << "DELETE EQ" << std::endl;}

    virtual Type type() const;
    virtual void update_kernel();

    void set_f_gain(const std::vector<int> &f_gain);

private:
    std::vector<int> *_f_gain;
};

class LowPass : public Filter
{
public:
    LowPass(){}
    LowPass(unsigned kernel_size, double sample_rate, unsigned order, double cutoff, double gain);
    virtual ~LowPass(){ std::cout << "DELETE LP" << std::endl;}

    virtual Type type() const;
    virtual void update_kernel();

    void set_params(unsigned order, double cutoff, double gain);

private:
    unsigned _order;
    double _cutoff;
    double _gain;
};

class HighPass : public Filter
{
public:
    HighPass(){}
    HighPass(unsigned size, double sample_rate, unsigned order, double cutoff, double gain);
    virtual ~HighPass(){std::cout << "DELETE HP" << std::endl;}

    virtual Type type() const;
    virtual void update_kernel();

    void set_params(unsigned order, double cutoff, double gain);

private:
    unsigned _order;
    double _cutoff;
    double _gain;
};

class BandPass : public Filter
{
public:
    BandPass(){}
    BandPass(unsigned size, double sample_rate,  unsigned order,
             double cutoff_lp, double cutoff_hp, double gain);
    virtual ~BandPass(){ std::cout << "DELETE BP" << std::endl;}

    virtual Type type() const;
    virtual void update_kernel();

    void set_params(unsigned order, double cutoff_lp, double cutoff_hp, double gain);

private:
    unsigned _order;
    double _cutoff_lp;
    double _cutoff_hp;
    double _gain;
};

class BandStop : public Filter
{
public:
    BandStop(){}
    BandStop(unsigned size, double sample_rate,  unsigned order,
             double cutoff1, double cutoff2, double gain);
    virtual ~BandStop(){ std::cout << "DELETE BS" << std::endl;}

    virtual Type type() const;
    virtual void update_kernel();

    void set_params(unsigned order, double cutoff_lp, double cutoff_hp, double gain);

private:
    unsigned _order;
    double _cutoff_lp;
    double _cutoff_hp;
    double _gain;
};

#endif // FILTAH_H
