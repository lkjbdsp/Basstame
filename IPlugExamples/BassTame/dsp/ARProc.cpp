#include "ARProc.h"
#include <cmath>

double ARProc::LogRange::to01(double val) const
{
    return std::log(val / minVal) / std::log(maxVal / minVal);
}

double ARProc::LogRange::clip(double val) const
{
    return std::max(minVal, std::min(maxVal, val));
}

void ARProc::clear()
{
    x1 = 0.;
    x2 = 0.;
    dx1 = 0.;
    dx2 = 0.;

    for (auto& aro : arOut)
        aro = 0.f;
}

void ARProc::setAttack(double newAttack)
{
    attack = attRng.clip(newAttack);
}

void ARProc::setRelease(double newRelease)
{
    release = relRng.clip(newRelease);
}

double ARProc::getAttack() const
{
    return attack;
}

double ARProc::getRelease() const
{
    return release;
}

void ARProc::setBlockSize(size_t blockSize)
{
    if (arOut.size() != blockSize)
        arOut.resize(blockSize);
}

void ARProc::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
}

void ARProc::processBlock(const double* grIn, int numSamples)
{
    const auto a = 820 * std::pow(270e3 / 820., attRng.to01(attack));
    const auto b = std::pow(0.01, 1. - relRng.to01(release));
    const auto c = 0.47e-6;
    const auto d = 6.8e-6;

    const auto e = std::pow(3.2927e2, attRng.to01(attack));

    const auto dt = 1. / sampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        const auto x00 = grIn[i];
        const auto x01 = x1 + x2;
        const auto x02 = x01 - x00;
        const auto x03 = 0.5 + 0.5 * x02 / std::sqrt(x02 * x02 + 0.001);
        const auto x04 = a + std::pow(1e7, x03);        

        const auto x05 = (x00 * 2.4750e+12 - x1 * 750e3 * x04 - x2 * (x04 * 3.3e6 + 750e3 * x04)) / (x04 * 2.4750e+12);

        const auto x06 = (x2 * 91e3 + x05 * 6.8250e+10 - x1 * 750e3) / (6.8250e+10);

        const auto x07 = std::pow(b, x03);
        const auto x08 = c * x07;
        const auto x09 = d * x07;

        const auto x10 = x06 * dt / x08;
        const auto x11 = x05 * dt / x09;
        x1 += 0.5 * (x10 + dx1);
        x2 += 0.5 * (x11 + dx1);

        dx1 = x10;
        dx2 = x11;

        arOut[i] = static_cast<double> (x1 + x2);
    }
}

void ARProc::deDecibel(int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        arOut[i] = std::pow(10., -0.05 * std::max(0., arOut[i]));
}

const std::vector<double>& ARProc::getOutput() const
{
    return arOut;
}