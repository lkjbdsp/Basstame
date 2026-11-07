#pragma once

#define PLUG_MFR "lkjb"
#define PLUG_NAME "BassTame"

#define PLUG_CLASS_NAME BassTame

#define BUNDLE_MFR "lkjb"
#define BUNDLE_NAME "BassTame"

#define PLUG_ENTRY BassTame_Entry
#define PLUG_VIEW_ENTRY BassTame_ViewEntry

#define PLUG_ENTRY_STR "BassTame_Entry"
#define PLUG_VIEW_ENTRY_STR "BassTame_ViewEntry"

#define VIEW_CLASS BassTame_View
#define VIEW_CLASS_STR "BassTame_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00020000
#define VST3_VER_STR "2.0.0"

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes. At least one capital letter
#define PLUG_UNIQUE_ID 'Ipef'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'Acme'

// ProTools stuff

#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[2] = {'EFN1', 'EFN2'};
  const int PLUG_TYPE_IDS_AS[2] = {'EFA1', 'EFA2'}; // AudioSuite
#endif

#define PLUG_MFR_PT "lkjb\nlkjb\nAcme"
#define PLUG_NAME_PT "BassTame\nIPEF"
#define PLUG_TYPE_PT "Effect"
#define PLUG_DOES_AUDIOSUITE 1

/* PLUG_TYPE_PT can be "None", "EQ", "Dynamics", "PitchShift", "Reverb", "Delay", "Modulation", 
"Harmonic" "NoiseReduction" "Dither" "SoundField" "Effect" 
instrument determined by PLUG _IS _INST
*/

#define PLUG_CHANNEL_IO "1-1 2-2"

#define PLUG_LATENCY 0
#define PLUG_IS_INST 0

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 0
#define AU_MIDI_FX 0

#define PLUG_DOES_STATE_CHUNKS 0

// Unique IDs for each image resource.
#define KNOB_ID 101
#define VU_ID 102
#define BG_ID 103

// Image resource locations for this plug.
#define KNOB_FN "resources/img/knob.png"
#define VU_FN "resources/img/vu.png"
#define BG_FN "resources/img/background.png"

// GUI default dimensions
#define GUI_WIDTH 500
#define GUI_HEIGHT 300

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API)
#include "app_wrapper/app_resource.h"
#endif

// vst3 stuff
#define MFR_URL "lkjbdsp.wordpress.com"
#define MFR_EMAIL "lkjbdsp@gmail.com"
#define EFFECT_TYPE_VST3 "Fx"


// Parameters and presets stuff ------------------------------------------------------------------------
#define kNumPrograms 1
#define kNumPublicParams 2
#define kNumPrivateParams 0
  
enum EParam
{
  kAmount,
  kFreq
};
