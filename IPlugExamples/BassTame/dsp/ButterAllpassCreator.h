#pragma once

#include "BaseBiquad.h"

namespace ButterAllpassCreator
{

void addButterAllpass(BiquadCascade& cascade, double freq, size_t order, double sampleRate);

}
