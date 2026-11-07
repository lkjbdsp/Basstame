#pragma once

#include "BaseBiquad.h"
#include "ButterworthCreator.h"
#include "ButterAllpassCreator.h"
#include <vector>
#include "PreAllocVector.h"

struct SplitterParams
{
    std::vector<double> xOvers;
    std::vector<BiquadCascade> xFilters;
    std::vector<BiquadCascade> xAllpass;
};

class MultiBandSplitter
{
public:

    MultiBandSplitter();
    ~MultiBandSplitter();

    void setCrossOvers(SplitterParams& params, const std::vector<double>& newXOvers, int order, double sampleRate);
    void getBypassAllpass(BiquadCascade& cascade, const SplitterParams& params) const;

private:

    void addCrossOver(SplitterParams& params, size_t index, double freq, int order);

    double curSampleRate = 0;
    int curOrder = -1;
    SplitterParams splitParams;
};

class MultiBandSplitterBasicProcessor
{
public:
    typedef std::vector<double> Buffer;

    MultiBandSplitterBasicProcessor();
    ~MultiBandSplitterBasicProcessor();

    void setBlockSize(size_t newBlockSize);
    size_t getBlockSize() const;

    void matchToSplitter(const SplitterParams& params);
    void clear();
    void processBlock(const SplitterParams& params, const double* in, int numSamples);
    const Buffer& getOutput(size_t index) const;
    Buffer& getOutput(size_t index);

private:
    
    void matchStates(std::vector<StateCascade>& states, const std::vector<BiquadCascade>& biquads) const;
    void updateBuffers();

    size_t blockSize = 0;
    std::vector<StateCascade> xFiltState;
    std::vector<StateCascade> xAllState;
    std::vector<Buffer> buffers;
};
