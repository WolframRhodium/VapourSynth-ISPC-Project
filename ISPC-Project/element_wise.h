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

#endif // ISPC_ELEMENT_WISE_H
