#include "GRProc.h"

#include <cmath>

void GRProc::clear()
{
    for (auto& gro : grOut)
        gro = 0.f;
}

void GRProc::setThresh(double newThresh)
{
    thresh = newThresh;
}

void GRProc::setRatio(double newRatio)
{
    ratio = newRatio;
}

void GRProc::setMaxGr(double newMaxGr)
{
    maxGr = newMaxGr;
}

double GRProc::getThresh() const
{
    return thresh;
}

double GRProc::getRatio() const
{
    return ratio;
}

double GRProc::getMaxGr() const
{
    return maxGr;
}

void GRProc::setBlockSize(size_t blockSize)
{
    if (blockSize != grOut.size())
        grOut.resize(blockSize);
}

void GRProc::processBlock(const double* scIn, const double* cmpIn, int numSamples)
{
    if (scIn == nullptr || numSamples > static_cast<int> (grOut.size()))
        return;
    
    const auto a = 1.f - 1.f / ratio;
    const auto b = 1.f;

    for (int i = 0; i < numSamples; ++i)
    {
        const auto x00 = 20.f * std::log10(std::max(1e-8, std::abs(scIn[i])));
        const auto x01 = 20.f * std::log10(std::max(1e-8, std::abs(cmpIn[i])));
        const auto x02 = x01 > -100 ? x00 - x01 - thresh : -100;
        const auto x03 = 0.5f + 0.5f * x02 / std::sqrt(x02 * x02 + b);
        const auto x04 = x02 * x03 * a;
        const auto x05 = x04 - maxGr;
        const auto x06 = 0.5 + 0.5 * x05 / std::sqrt(x05 * x05 + 0.1 * b);
        const auto x07 = x04 * (1. - x06) + maxGr * x06;
        grOut[i] = x07;
    }
}

const std::vector<double>& GRProc::getOutput() const
{
    return grOut;
}