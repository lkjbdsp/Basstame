#pragma once

#include <vector>

class GRProc
{
public:

    void clear();
    void setThresh(double newThresh);
    void setRatio(double newRatio);
    void setMaxGr(double newMaxGr);
    double getThresh() const;
    double getRatio() const;
    double getMaxGr() const;

    void setBlockSize(size_t blockSize);
    void processBlock(const double* scIn, const double* cmpIn, int numSamples);
    const std::vector<double>& getOutput() const;

private:

    double thresh = 0.f;
    double ratio = 4.f;
    double maxGr = 12.;
    std::vector<double> grOut;
};