#include "ButterworthCreator.h"
#include <cmath>

namespace ButterworthCreator    
{

void createSecondOrderPass(Biquad& bq, double freq, double Q, bool hipass, double sampleRate)
{
    BaseBiquad bbq;
    const double pi = 3.14159265358979323846;
    const double w0 = 2*pi*freq / sampleRate;
    const double alpha = std::sin(w0) / (2 * Q);

    const double a0 =   1 + alpha;
    bbq.a1 =  -2*std::cos(w0) / a0;
    bbq.a2 =   (1 - alpha) / a0;

    if (hipass)
    {
        bbq.b0 =  (1 + cos(w0)) / (2*a0);
        bbq.b1 = -(1 + cos(w0)) / a0;
        bbq.b2 =  (1 + cos(w0)) / (2*a0);
    }
    else
    {
        bbq.b0 =  (1 - cos(w0)) / (2*a0);
        bbq.b1 =  (1 - cos(w0)) / a0;
        bbq.b2 =  (1 - cos(w0)) / (2*a0);
    }

    bq.set(bbq);
}

void createFirstOrderPass(Biquad& bq, double freq, bool hipass, double sampleRate)
{
    BaseBiquad bbq;
    const double pi = 3.14159265358979323846;
    const double w0 = 2*pi*freq / sampleRate;
    const double c = (std::tan(0.5 * w0) - 1) / (std::tan(0.5 * w0) + 1);

    if (hipass)
    {
        bbq.b0 = 0.5 * (1 - c);
        bbq.b1 = 0.5 * (c - 1);
    }
    else
    {
        bbq.b0 = 0.5 * (1 + c);
        bbq.b1 = 0.5 * (1 + c);
    }

    bbq.a1 = c;
    bbq.a2 = 0;
    bbq.b2 = 0;

    bq.set(bbq);
}


void addButterWorth(BiquadCascade& cascade, double freq, size_t order, bool hipass, double sampleRate)
{
    const size_t numBiquads = (order+1) / 2;

    const size_t startIdx = cascade.biquads.size();
    cascade.biquads.resize(startIdx + numBiquads);
    const double pi = 3.14159265358979323846;

    for (size_t i=0; i<numBiquads; ++i)
    {
        const double Q = -0.5 / std::cos(pi * (2.*i + order + 1.) / (2.*order));
        createSecondOrderPass(cascade.biquads[startIdx + i], freq, Q, hipass, sampleRate);
    }

    if ((order & 1) == 1)
        createFirstOrderPass(cascade.biquads[startIdx + numBiquads - 1], freq, hipass, sampleRate);
}

void addLinkwitzReiley(BiquadCascade& cascade, double freq, size_t order, bool hipass, double sampleRate)
{
    if (order == 0)
    {
        addButterWorth(cascade, freq, 1, hipass, sampleRate);
    }
    else
    {
        addButterWorth(cascade, freq, order, hipass, sampleRate);
        addButterWorth(cascade, freq, order, hipass, sampleRate);
    }
}

}
