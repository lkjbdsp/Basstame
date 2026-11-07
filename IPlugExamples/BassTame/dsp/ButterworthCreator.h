#pragma once
#include "BaseBiquad.h"

namespace ButterworthCreator
{

void addButterWorth(BiquadCascade& cascade, double freq, size_t order, bool hipass, double sampleRate);
void addLinkwitzReiley(BiquadCascade& cascade, double freq, size_t order, bool hipass, double sampleRate);

}