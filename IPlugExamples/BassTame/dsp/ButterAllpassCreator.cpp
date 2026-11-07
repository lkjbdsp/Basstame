#include "ButterAllpassCreator.h"
#include <cmath>

namespace ButterAllpassCreator
{

void createSecondOrderAllpass(Biquad& bq, double freq, double Q, double sampleRate)
{
    BaseBiquad bbq;
    const double pi = 3.14159265358979323846;
    const double w0 = 2*pi*freq / sampleRate;
    const double alpha = std::sin(w0) / (2 * Q);

    const double a0 =   1 + alpha;
    bbq.b0 =   (1 - alpha) / a0;
    bbq.b1 =  -2*std::cos(w0) / a0;
    bbq.b2 =   (1 + alpha) / a0;
    bbq.a1 =  -2*std::cos(w0) / a0;
    bbq.a2 =   (1 - alpha) / a0;
    bq.set(bbq);
}

void createFirstOrderAllpass(Biquad& bq, double freq, double sampleRate)
{
    BaseBiquad bbq;
    const double pi = 3.14159265358979323846;
    const double w0 = 2*pi*freq / sampleRate;
    const double c = (std::tan(0.5 * w0) - 1) / (std::tan(0.5 * w0) + 1);
    bbq.b0 = c;
    bbq.b1 = 1;
    bbq.b2 = 0;
    bbq.a1 = c;
    bbq.a2 = 0;
    bq.set(bbq);
}

void addButterAllpass(BiquadCascade& cascade, double freq, size_t order, double sampleRate)
{
    const size_t numBiquads = (order+1) / 2;

    const size_t startIdx = cascade.biquads.size();
    cascade.biquads.resize(startIdx + numBiquads);
    const double pi = 3.14159265358979323846;

    for (size_t i=0; i<numBiquads; ++i)
    {
        const double Q = -0.5 / std::cos(pi * (2.*i + order + 1.) / (2.*order));
        createSecondOrderAllpass(cascade.biquads[startIdx + i], freq, Q, sampleRate);
    }

    if ((order & 1) == 1)
        createFirstOrderAllpass(cascade.biquads[startIdx + numBiquads - 1], freq, sampleRate);
}


}