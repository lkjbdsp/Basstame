#ifndef __BASSTAME__
#define __BASSTAME__

#include "IPlug_include_in_plug_hdr.h"
#include "dsp/BaseBiquad.h"
#include "dsp/ButterAllpassCreator.h"
#include "dsp/ButterworthCreator.h"
#include "dsp/MultiBandSplitter.h"
#include "dsp/GRProc.h"
#include "dsp/ARProc.h"

class Envelope
{
public:

    void setup(double timeMs, double sampleRate)
    {
        release = std::exp(-1. / (0.001 * timeMs * sampleRate));
    }

    void processBlock(double* out, const double* in, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const auto x = std::abs(in[i]);

            value = x > value ? x : value * release;
            out[i] = value;
        }
    }

    void clear()
    {
        value = 0.;
    }

private:
    double value = 0;
    double release = 0;

};

class BassTame : public IPlug
{
public:
    BassTame(IPlugInstanceInfo instanceInfo);
    ~BassTame();

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    void SetBlockSize(int blockSize) override;
    void SetSampleRate(double sampleRate) override;

    void Reset();
    void OnParamChange(int paramIdx, ParamChangeFrom changeFrom);
    void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);



private:

    static const int numSplitFreqs = 33;
    double mAmount = 1.0;
    double curVU = 0;
    int curSplitIndex = -1;
    double sideGain = 1;

    BiquadCascade biquads;
    StateCascade states[2];
    MultiBandSplitter splitter;
    SplitterParams splitParams[numSplitFreqs];
    MultiBandSplitterBasicProcessor splitProc[2];
    std::vector<double> xOvers = std::vector<double> (1);

    Envelope hfEnv;
    GRProc grProc;
    ARProc arProc;

    ITextControl* labelAmount = nullptr;
    ITextControl* labelFreq = nullptr;
    
    std::vector<double*> splitOuts;
    std::vector<double> hfEnvData;
    std::vector<double> splitFreqs;
    int activeBlockSize = -1;
    double activeSampleRate = 0.;
    static const int numBands = 2;
    static const int order = 4;
};

#endif
