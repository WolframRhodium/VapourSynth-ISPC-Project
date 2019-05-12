#include "VapourSynth.h"
#include "VSHelper.h"

#include "element_wise.h"

VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
    configFunc("com.wolframrhodium.ispc", "ispc", "ISPC filters", VAPOURSYNTH_API_VERSION, 1, plugin);
    registerFunc("Binarize", "clip:clip;threshold:float[]:opt;v0:float[]:opt;v1:float[]:opt;planes:int[]:opt;", binarizeCreate, 0, plugin);
    registerFunc("Invert", "clip:clip;planes:int[]:opt;", invertCreate, 0, plugin);
    registerFunc("Merge", "clipa:clip;clipb:clip;weight:float[]:opt;", mergeCreate, 0, plugin);
    registerFunc("Limiter", "clip:clip;min:float[]:opt;max:float[]:opt;planes:int[]:opt;", limiterCreate, 0, plugin);
}
