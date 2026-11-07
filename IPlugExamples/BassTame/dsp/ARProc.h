#pragma once

#include <vector>

class ARProc
{
public:

    void clear();
    void setAttack(double newAttack);
    void setRelease(double newRelease);
    double getAttack() const;
    double getRelease() const;

    void setBlockSize(size_t newBlockSize);
    void setSampleRate(double newSampleRate);
    void processBlock(const double* grIn, int numSamples);
    void deDecibel(int numSamples);
    const std::vector<double>& getOutput() const;

private:

    struct LogRange
    {
        LogRange(double minval, double maxval) :minVal(minval), maxVal(maxval) {}
        double to01(double val) const;
        double clip(double val) const;
        double minVal;
        double maxVal;
    };

    double attack = 0.1f;
    double release = 1.f;
    double sampleRate = 0;
    LogRange attRng = LogRange(0.1f, 30.f);
    LogRange relRng = LogRange(0.1f, 10.f);
    std::vector<double> arOut;

    double x1 = 0.;
    double x2 = 0.;
    double dx1 = 0.;
    double dx2 = 0.;
};