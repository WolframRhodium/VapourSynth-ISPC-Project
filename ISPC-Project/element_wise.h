#ifndef ISPC_ELEMENT_WISE_H
#define ISPC_ELEMENT_WISE_H

#include <stdbool.h>

#include "VapourSynth.h"
#include "VSHelper.h"

typedef struct {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    bool process[3];
} InvertData;

extern void VS_CC invertCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi);

typedef struct {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    bool process[3];
    uint16_t maxi[3], mini[3];
    float maxf[3], minf[3];
} LimiterData;

extern void VS_CC limiterCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi);

typedef struct {
    VSNodeRef *node;
    const VSVideoInfo *vi;
    bool process[3];
    uint16_t thresholdi[3], v0i[3], v1i[3];
    float thresholdf[3], v0f[3], v1f[3];
} BinarizeData;

extern void VS_CC binarizeCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi);

#endif // ISPC_ELEMENT_WISE_H
