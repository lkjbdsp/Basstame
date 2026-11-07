#include "MultiBandSplitter.h"
#include <algorithm>
#include <functional>

MultiBandSplitter::MultiBandSplitter()
{
}

MultiBandSplitter::~MultiBandSplitter()
{
}

void MultiBandSplitter::setCrossOvers(SplitterParams& params, const std::vector<double>& newXOvers, int order, double sampleRate)
{
    if (newXOvers == params.xOvers && curSampleRate == sampleRate && curOrder == order)
        return;

    curSampleRate = sampleRate;
    curOrder = order;
    params.xOvers = newXOvers;
    const size_t numXOvers = params.xOvers.size();
    params.xFilters.resize(2*numXOvers);
    params.xAllpass.resize(numXOvers + 1);

    if (numXOvers > 1)
        std::sort(params.xOvers.begin(), params.xOvers.end());

    for (size_t i=0; i<numXOvers; ++i)
    {
        const size_t idx = numXOvers - i - 1;
        addCrossOver(params, idx, params.xOvers[idx], curOrder);
    }

    if ((curOrder & 1) == 1)
    {
        for (size_t i=1; i< params.xFilters.size(); i += 2)
            if (params.xFilters[i].biquads.size() > 0)
                params.xFilters[i].biquads.back().invertPhase();
    }
}

void MultiBandSplitter::getBypassAllpass(BiquadCascade& cascade, const SplitterParams& params) const
{

    for (auto freq : params.xOvers)
        ButterAllpassCreator::addButterAllpass(cascade, freq, curOrder, curSampleRate);
}

void MultiBandSplitter::addCrossOver(SplitterParams& params, size_t index, double freq, int order)
{
    ButterworthCreator::addLinkwitzReiley(params.xFilters[2 * index + 0], freq, order, false, curSampleRate);
    ButterworthCreator::addLinkwitzReiley(params.xFilters[2 * index + 1], freq, order, true, curSampleRate);

    for (size_t i = index+2; i< params.xAllpass.size(); ++i)
        ButterAllpassCreator::addButterAllpass(params.xAllpass[i], freq, order, curSampleRate);
}

MultiBandSplitterBasicProcessor::MultiBandSplitterBasicProcessor()
{
}

MultiBandSplitterBasicProcessor::~MultiBandSplitterBasicProcessor()
{
}


void MultiBandSplitterBasicProcessor::setBlockSize(size_t newBlockSize)
{
    blockSize = newBlockSize;
    updateBuffers();
}

size_t MultiBandSplitterBasicProcessor::getBlockSize() const
{
    return blockSize;
}

void MultiBandSplitterBasicProcessor::matchToSplitter(const SplitterParams& params)
{
    const size_t numXOvers = params.xOvers.size();
    buffers.resize(numXOvers + 1);
    updateBuffers();
    matchStates(xAllState, params.xAllpass);
    matchStates(xFiltState, params.xFilters);
    clear();
}

void MultiBandSplitterBasicProcessor::clear()
{
    for (auto s : xFiltState)
        s.clear();

    for (auto s : xAllState)
        s.clear();
}

void MultiBandSplitterBasicProcessor::processBlock(const SplitterParams& params, const double* in, int numSamples)
{
    const int numXOvers = params.xOvers.size();

    for (int i = numXOvers - 1; i >= 0; --i)
    {
        double* outTop = buffers[i + 1].data();
        double* outBot = buffers[i].data();
        const BiquadCascade& bqTop(params.xFilters[2 * i + 1]);
        const BiquadCascade& bqBot(params.xFilters[2 * i + 0]);
        StateCascade& sTop(xFiltState[2 * i + 1]);
        StateCascade& sBot(xFiltState[2 * i + 0]);

        basicCascadeProcessing(bqBot, sBot, outBot, in, numSamples);
        basicCascadeProcessing(bqTop, sTop, outTop, in, numSamples);
        in = outBot;
    }

    for (int i = 0; i <= numXOvers; ++i)
    {
        const BiquadCascade& bq(params.xAllpass[i]);
        StateCascade& s(xAllState[i]);
        double* data = buffers[i].data();
        basicCascadeProcessing(bq, s, data, data, numSamples);
    }
}

const MultiBandSplitterBasicProcessor::Buffer& MultiBandSplitterBasicProcessor::getOutput(size_t index) const
{
    index = index >= buffers.size() ? buffers.size() - 1 : index;
    return buffers[index];
}

MultiBandSplitterBasicProcessor::Buffer& MultiBandSplitterBasicProcessor::getOutput(size_t index)
{
    index = index >= buffers.size() ? buffers.size() - 1 : index;
    return buffers[index];
}

void MultiBandSplitterBasicProcessor::matchStates(std::vector<StateCascade>& states, const std::vector<BiquadCascade>& biquads) const
{
    states.resize(biquads.size());

    for (size_t i = 0; i < biquads.size(); ++i)
        states[i].matchToBiquadCascade(biquads[i]);
}

void MultiBandSplitterBasicProcessor::updateBuffers()
{
    for (auto& b : buffers)
        b.resize(blockSize);
}



