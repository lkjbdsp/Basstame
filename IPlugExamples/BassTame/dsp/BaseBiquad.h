#pragma once

#include <vector>
#include <cmath>

struct BaseBiquad
{
    double b0 = 1;
    double b1 = 0;
    double b2 = 0;
    double a1 = 0;
    double a2 = 0;
};

struct Biquad
{
    void invertPhase()
    {
        t0 = -t0;
        t1 = -t1;
        t2 = -t2;
    }    
    
    void set(const BaseBiquad& bq)
    {
        f = bq.a2;
        e = std::sqrt(1 + bq.a1 + bq.a2);

        t0 = bq.b0 / e;
        t1 = -bq.b2 / e;
        t2 = (bq.b2 + bq.b1 + bq.b0) / (e * e);
    }

    double f = 0;
    double e = 1;
    double t0 = 1;
    double t1 = 0;
    double t2 = 1;
};

struct State
{
    void clear()
    {
        z1=0; z2=0;
    }

    double z1=0;
    double z2=0;
};

struct BiquadCascade
{
    std::vector<Biquad> biquads;
};

struct StateCascade
{
    std::vector<State> states;

    void clear()
    {
        for (auto s:states)
            s.clear();
    }

    void matchToBiquadCascade(const BiquadCascade& cascade)
    {
        states.resize(cascade.biquads.size());
        clear();
    }
};

inline void basicStateProcessing(const Biquad& bq, State& s, double* out, const double* in, int numSamples)
{
    for (int i=0; i<numSamples; ++i)
    {
        const double x0 = in[i];

        const double tmp = s.z1 * bq.f + (x0 - s.z2) * bq.e;
        out[i] = tmp * bq.t0 + s.z1 * bq.t1 + s.z2 * bq.t2;

        s.z2 = tmp * bq.e + s.z2;
        s.z1 = tmp;
    }
}

inline void basicCascadeProcessing(const BiquadCascade& bqs, StateCascade& s, double* out, const double* in, const int numSamples)
{
    if (bqs.biquads.size() != s.states.size() || bqs.biquads.size() == 0)
        return;

    basicStateProcessing(bqs.biquads[0], s.states[0], out, in, numSamples);

    for (size_t i=1; i<bqs.biquads.size(); ++i)
        basicStateProcessing(bqs.biquads[i], s.states[i], out, out, numSamples);
}
