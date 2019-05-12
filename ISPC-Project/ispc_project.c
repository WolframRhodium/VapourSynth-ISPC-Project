#include "VapourSynth.h"
#include "VSHelper.h"

#include "element_wise.h"

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
    configFunc("com.wolframrhodium.ispc", "ispc", "ISPC filters", VAPOURSYNTH_API_VERSION, 1, plugin);
    registerFunc("Invert", "clip:clip;planes:int[]:opt;", invertCreate, 0, plugin);
}
