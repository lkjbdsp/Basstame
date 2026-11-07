#include "BassTame.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

namespace AudioBufferDsp
{
void clear(double* buf, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        buf[i] = 0;
}

void copy(double* out, const double* in, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        out[i] = in[i];
}

void mul(double* out, const double* in1, const double* in2, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        out[i] = in1[i] * in2[i];
}

void mul(double* out, const double* in, double mul, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        out[i] = in[i] * mul;
}

void mul(double* out, double mul, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        out[i] *= mul;
}

void add(double* out, const double* in, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
        out[i] += in[i];
}

void sideGain(double* chL, double* chR, double gain, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        const auto m = 0.5 * (chL[i] + chR[i]);
        auto s = 0.5 * (chL[i] - chR[i]);

        s *= gain;

        chL[i] = m + s;
        chR[i] = m - s;
    }
}

} // AudioBufferDsp

class VUMeter : public IBitmapControl
{
public:
    VUMeter(IPlugBase* pPlug, double& val, int x, int y, IBitmap* pBitmap, IChannelBlend::EBlendMethod blendMethod = IChannelBlend::kBlendNone)
        : IBitmapControl(pPlug, x, y, pBitmap, blendMethod), value(val)
    {
        IRECT pR(0, 0, 0, 0);
        SetTargetRECT(pR);
    }

    void OnGUIIdle() override 
    {
        if (value != GetValue())
            SetValueFromUserInput(value);
    }

private:
    double& value;
};


enum ELayout
{
    kWidth = GUI_WIDTH,
    kHeight = GUI_HEIGHT,

    kAmountX = 100,
    kAmountY = 110,
    kFreqX = 275,
    kFreqY = 110,

    kLabelY = 235,
    kLabelWidth = 125,
    kLabelHeight = 30,

    kKnobFrames = 99
};

BassTame::BassTame(IPlugInstanceInfo instanceInfo)
    :	IPLUG_CTOR(kNumPublicParams, kNumPrivateParams, kNumPrograms, instanceInfo)
{
    TRACE;

    GetParam(kAmount)->InitDouble("Amount", 50., 0., 100.0, 0.01, "%");
    GetParam(kFreq)->InitDouble("Crossover", 50., 30, 200, 1, "Hz");
    GetParam(kFreq)->SetShape(2.);

    IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

    pGraphics->AttachBackground(BG_ID, BG_FN);
    
    IBitmap* knob = pGraphics->LoadPointerToBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
    IBitmap* vu = pGraphics->LoadPointerToBitmap(VU_ID, VU_FN, kKnobFrames);

    pGraphics->AttachControl(new IKnobMultiControl(this, kAmountX, kAmountY, kAmount, knob));
    pGraphics->AttachControl(new VUMeter(this, curVU, kAmountX, kAmountY, vu));

    pGraphics->AttachControl(new IKnobMultiControl(this, kFreqX, kFreqY, kFreq, knob));

    IColor textColor = IColor(255, 100, 100, 100);
    IText labelText(24, &textColor, "Arial", IText::kStyleNormal, IText::kAlignCenter, 0, IText::kQualityDefault);
    labelAmount = new ITextControl(this, IRECT(kAmountX, kLabelY, kAmountX + kLabelWidth, kLabelY + kLabelHeight), &labelText, "50");
    labelFreq = new ITextControl(this, IRECT(kFreqX, kLabelY, kFreqX + kLabelWidth, kLabelY + kLabelHeight), &labelText, "100");
    
    pGraphics->AttachControl(labelAmount);
    pGraphics->AttachControl(labelFreq);
    
    AttachGraphics(pGraphics);

    MakeDefaultPreset((char *) "-", kNumPrograms);
}

void BassTame::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const auto splitFreq = 120.f;

    splitOuts.resize(numBands);

    {
        const auto minFreq = GetParam(kFreq)->GetMin();
        const auto maxFreq = GetParam(kFreq)->GetMax();
        splitFreqs.clear();

        for (int i = 0; i < numSplitFreqs; ++i)
        {
            xOvers[0] = minFreq * std::pow(maxFreq / minFreq, i / (numSplitFreqs - 1.));
            splitFreqs.push_back(xOvers[0]);
            splitter.setCrossOvers(splitParams[i], xOvers, order, sampleRate);
        }
    }

    splitProc[0].setBlockSize(samplesPerBlock);
    splitProc[1].setBlockSize(samplesPerBlock);
    splitProc[0].matchToSplitter(splitParams[0]);
    splitProc[1].matchToSplitter(splitParams[0]);
    splitter.getBypassAllpass(biquads, splitParams[0]);
    states[0].matchToBiquadCascade(biquads);
    states[1].matchToBiquadCascade(biquads);

    hfEnvData.resize(samplesPerBlock);
    hfEnv.setup(300, sampleRate);

    grProc.setBlockSize(samplesPerBlock);
    grProc.setRatio(4);   
    grProc.setMaxGr(12.);

    arProc.setSampleRate(sampleRate);
    arProc.setBlockSize(samplesPerBlock);
    arProc.setAttack(1.0);
    arProc.setRelease(0.5);
}

void BassTame::SetBlockSize(int blockSize)
{
    WDL_MutexLock lock(&mMutex);
    IPlug::SetBlockSize(blockSize);
    activeBlockSize = blockSize;

    if (activeBlockSize > 0 && activeSampleRate > 0)
        prepareToPlay(activeSampleRate, activeBlockSize);
}

void BassTame::SetSampleRate(double sampleRate)
{
    IPlug::SetSampleRate(sampleRate);
    activeSampleRate = sampleRate;

    if (activeBlockSize > 0 && activeSampleRate > 0)
        prepareToPlay(activeSampleRate, activeBlockSize);
}

BassTame::~BassTame() {}

void BassTame::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    grProc.setThresh((1. - mAmount) * 40 - 30);
    grProc.setRatio(1. + std::sqrt(mAmount) * 5);

    double* in1 = inputs[0];
    double* in2 = inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];
    const auto isStereo = in2 != nullptr && out2 != nullptr && in1 != in2;
    const auto splitFreqVal = static_cast<int> (GetParam(kFreq)->GetNormalized() * (numSplitFreqs-1) + 0.5);
    const auto splitIdx = std::max(0, std::min(numSplitFreqs - 1, splitFreqVal));

    basicCascadeProcessing(biquads, states[0], out1, in1, nFrames);
    splitProc[0].processBlock(splitParams[splitIdx], out1, nFrames);

    if (isStereo)
    {
        basicCascadeProcessing(biquads, states[1], out2, in2, nFrames);
        splitProc[1].processBlock(splitParams[splitIdx], out2, nFrames);
        auto chL = splitProc[0].getOutput(0).data();
        auto chR = splitProc[1].getOutput(0).data();
        
        AudioBufferDsp::sideGain(chL, chR, sideGain, nFrames);
    }

    if (isStereo)
    {
        const auto inL = splitProc[0].getOutput(1).data();
        const auto inR = splitProc[1].getOutput(1).data();
        auto hed = hfEnvData.data();
        AudioBufferDsp::copy(hed, inL, nFrames);
        AudioBufferDsp::add(hed, inR, nFrames);
        AudioBufferDsp::mul(hed, 0.5, nFrames);
        hfEnv.processBlock(hed, hed, nFrames);
    }
    else
    {
        const auto in = splitProc[0].getOutput(1).data();
        hfEnv.processBlock(hfEnvData.data(), in, nFrames);
    }

    grProc.processBlock(splitProc[0].getOutput(0).data(), hfEnvData.data(), nFrames);
    const auto grData = grProc.getOutput();
    arProc.processBlock(grData.data(), nFrames);

    curVU = arProc.getOutput().data()[nFrames - 1] / grProc.getMaxGr();
    arProc.deDecibel(nFrames);
    const auto arData = arProc.getOutput();

    for (int i = 0; i < numBands; ++i)
        splitOuts[i] = splitProc[0].getOutput(i).data();

    AudioBufferDsp::mul(out1, splitOuts[0], arData.data(), nFrames);
    AudioBufferDsp::add(out1, splitOuts[1], nFrames);

    if (isStereo)
    {
        for (int i = 0; i < numBands; ++i)
            splitOuts[i] = splitProc[1].getOutput(i).data();

        AudioBufferDsp::mul(out2, splitOuts[0], arData.data(), nFrames);
        AudioBufferDsp::add(out2, splitOuts[1], nFrames);
    }    
}

void BassTame::Reset()
{
    TRACE;
    IMutexLock lock(this);

    curVU = 0;
    hfEnv.clear();
}

void BassTame::OnParamChange(int paramIdx, ParamChangeFrom changeFrom)
{
    IMutexLock lock(this);

    switch (paramIdx)
    {
        case kAmount:
        {
            const auto text = std::to_string(static_cast<int> (GetParam(kAmount)->Value() + 0.5)) + "%";
            mAmount = GetParam(kAmount)->Value() / 100.;
            labelAmount->SetTextFromPlug(const_cast<char*> (text.c_str()));
        }
            break;
        case kFreq:
        {
            const auto splitFreqVal = static_cast<int> (GetParam(kFreq)->GetNormalized() * (numSplitFreqs - 1.) + 0.5);

            if (splitFreqs.size() == numSplitFreqs)
            {
                const auto splitFreq = splitFreqs[splitFreqVal];
                const auto text = std::to_string(static_cast<int> (splitFreq + 0.5)) + " Hz";
                labelFreq->SetTextFromPlug(const_cast<char*> (text.c_str()));
            }

            break;
        }
        default:
            break;
    }
}
