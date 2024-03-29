#include <stdbool.h>

#include "element_wise.h"
#include "element_wise_ispc.h" // generated by ispc

// Invert
static void VS_CC invertInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    InvertData *d = (InvertData *) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC invertGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    InvertData *d = (InvertData *) * instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);

        const int pl[] = { 0, 1, 2 };
        const VSFrameRef *fr[] = {d->process[0] ? NULL : src, d->process[1] ? NULL : src, d->process[2] ? NULL : src};
        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, d->vi->width, d->vi->height, fr, pl, src, core);

        for (int plane = 0; plane < d->vi->format->numPlanes; plane++) {
            if (d->process[plane]) {
                int stride = vsapi->getStride(src, plane) / d->vi->format->bytesPerSample;
                int height = vsapi->getFrameHeight(src, plane);
                int width = vsapi->getFrameWidth(src, plane);
                const uint8_t * VS_RESTRICT srcp = vsapi->getReadPtr(src, plane);
                uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);

                if (d->vi->format->sampleType == stInteger) {
                    if (d->vi->format->bytesPerSample == 1) {
                        invert_i8(srcp, dstp, width, height, stride);

                    } else if (d->vi->format->bytesPerSample == 2) {
                        if (d->vi->format->bitsPerSample == 16) {
                            invert_i16((const uint16_t *)srcp, (uint16_t *)dstp, width, height, stride);
                        } else {
                            const uint16_t peak = (1 << d->vi->format->bitsPerSample) - 1;

                            invert_i16m((const uint16_t *)srcp, (uint16_t *)dstp, width, height, stride, peak);
                        }
                    }
                } else if (d->vi->format->sampleType == stFloat) {
                    if (d->vi->format->bytesPerSample == 4) {
                        const bool uv = (plane > 0) && ((d->vi->format->colorFamily == cmYUV) || (d->vi->format->colorFamily == cmYCoCg));

                        invert_f32((const float *)srcp, (float *)dstp, width, height, stride, uv);
                    }
                }
            }
        }

        vsapi->freeFrame(src);
        return dst;
    }

    return 0;
}

static void VS_CC invertFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    InvertData *d = (InvertData *)instanceData;
    vsapi->freeNode(d->node);
    free(d);
}

void VS_CC invertCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    InvertData d;

    d.node = vsapi->propGetNode(in, "clip", 0, NULL);
    d.vi = vsapi->getVideoInfo(d.node);

    int num_planes = d.vi->format->numPlanes;
    const int m = vsapi->propNumElements(in, "planes");

    for (int i = 0; i < 3; i++)
        d.process[i] = (m <= 0);

    for (int i = 0; i < vsapi->propNumElements(in, "planes"); i++) {
        int plane = int64ToIntS(vsapi->propGetInt(in, "planes", i, 0));

        if (plane < 0 || plane >= num_planes) {
            vsapi->freeNode(d.node);
            vsapi->setError(out, "ispc.Invert: plane index out of range");
            return;
        }

        if (d.process[plane]) {
            vsapi->freeNode(d.node);
            vsapi->setError(out, "ispc.Invert: plane specified twice");
            return;
        }

        d.process[plane] = true;
    }

    InvertData * const data = malloc(sizeof(d));
    *data = d;

    vsapi->createFilter(in, out, "Invert", invertInit, invertGetFrame, invertFree, fmParallel, 0, data, core);
}

// Limiter
static void VS_CC limiterInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    LimiterData *d = (LimiterData *) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC limiterGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    LimiterData *d = (LimiterData *) * instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);

        const int pl[] = { 0, 1, 2 };
        const VSFrameRef *fr[] = {d->process[0] ? NULL : src, d->process[1] ? NULL : src, d->process[2] ? NULL : src};
        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, d->vi->width, d->vi->height, fr, pl, src, core);

        for (int plane = 0; plane < d->vi->format->numPlanes; plane++) {
            if (d->process[plane]) {
                int stride = vsapi->getStride(src, plane) / d->vi->format->bytesPerSample;
                int height = vsapi->getFrameHeight(src, plane);
                int width = vsapi->getFrameWidth(src, plane);
                const uint8_t * VS_RESTRICT srcp = vsapi->getReadPtr(src, plane);
                uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);

                if (d->vi->format->sampleType == stInteger) {
                    if (d->vi->format->bytesPerSample == 1) {
                        limiter_i8(srcp, dstp, width, height, stride, (uint8_t)d->mini[plane], (uint8_t)d->maxi[plane]);

                    } else if (d->vi->format->bytesPerSample == 2) {
                        limiter_i16((const uint16_t *)srcp, (uint16_t *)dstp, width, height, stride, d->mini[plane], d->maxi[plane]);
                    }
                } else if (d->vi->format->sampleType == stFloat) {
                    if (d->vi->format->bytesPerSample == 4) {
                        limiter_f32((const float *)srcp, (float *)dstp, width, height, stride, d->minf[plane], d->maxf[plane]);
                    }
                }
            }
        }

        vsapi->freeFrame(src);
        return dst;
    }

    return 0;
}

static void VS_CC limiterFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    LimiterData *d = (LimiterData *)instanceData;
    vsapi->freeNode(d->node);
    free(d);
}

void VS_CC limiterCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    LimiterData d;

    d.node = vsapi->propGetNode(in, "clip", 0, NULL);
    d.vi = vsapi->getVideoInfo(d.node);

    int num_planes = d.vi->format->numPlanes;

    bool prevValid;

    if (vsapi->propNumElements(in, "min") > num_planes) {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "ispc.Limiter: \"min\" has more values specified than there are planes");
        return;
    }

    prevValid = false;
    for (int i = 0; i < num_planes; i++) {
        int err;

        if (d.vi->format->sampleType == stInteger) {
            uint16_t temp = (uint16_t)(vsapi->propGetFloat(in, "min", i, &err) + .5);
            if (err) {
                temp = (i == 0) ? 0U : d.mini[i-1];
            } else {
                prevValid = true;
            }

            d.mini[i] = temp;

        } else if (d.vi->format->sampleType == stFloat) {
            float temp = (float)vsapi->propGetFloat(in, "min", i, &err);
            if (err) {
                temp = prevValid ? d.minf[i-1] : (((i > 0) && ((d.vi->format->colorFamily == cmYUV) || (d.vi->format->colorFamily == cmYCoCg))) ? -0.5f : 0.f);
            } else {
                prevValid = true;
            }

            d.minf[i] = temp;
        }
    }

    if (vsapi->propNumElements(in, "max") > num_planes) {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "ispc.Limiter: \"max\" has more values specified than there are planes");
        return;
    }

    prevValid = false;
    for (int i = 0; i < num_planes; i++) {
        int err;

        if (d.vi->format->sampleType == stInteger) {
            uint16_t temp = (uint16_t)(vsapi->propGetFloat(in, "max", i, &err) + .5);
            if (err) {
                temp = (i == 0) ? ((1 << d.vi->format->bitsPerSample) - 1) : d.maxi[i-1];
            } else {
                prevValid = true;
            }

            d.maxi[i] = temp;

            if (d.mini[i] > d.maxi[i]) {
                vsapi->freeNode(d.node);
                vsapi->setError(out, "ispc.Limiter: min bigger than max");
                return;
            }

        } else if (d.vi->format->sampleType == stFloat) {
            float temp = (float)vsapi->propGetFloat(in, "max", i, &err);
            if (err) {
                temp = prevValid ? d.maxf[i-1] : (((i > 0) && ((d.vi->format->colorFamily == cmYUV) || (d.vi->format->colorFamily == cmYCoCg))) ? 0.5f : 1.f);
            } else {
                prevValid = true;
            }

            d.maxf[i] = temp;

            if (d.minf[i] > d.maxf[i]) {
                vsapi->freeNode(d.node);
                vsapi->setError(out, "ispc.Limiter: min bigger than max");
                return;
            }
        }
    }

    const int m = vsapi->propNumElements(in, "planes");

    for (int i = 0; i < 3; i++)
        d.process[i] = (m <= 0);

    for (int i = 0; i < vsapi->propNumElements(in, "planes"); i++) {
        int plane = int64ToIntS(vsapi->propGetInt(in, "planes", i, NULL));

        if (plane < 0 || plane >= num_planes) {
            vsapi->freeNode(d.node);
            vsapi->setError(out, "ispc.Limiter: plane index out of range");
            return;
        }

        if (d.process[plane]) {
            vsapi->freeNode(d.node);
            vsapi->setError(out, "ispc.Limiter: plane specified twice");
            return;
        }

        d.process[plane] = true;
    }

    LimiterData * const data = malloc(sizeof(d));
    *data = d;

    vsapi->createFilter(in, out, "Limiter", limiterInit, limiterGetFrame, limiterFree, fmParallel, 0, data, core);
}

// Binarize
static void VS_CC binarizeInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    BinarizeData *d = (BinarizeData *) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC binarizeGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    BinarizeData *d = (BinarizeData *) * instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src = vsapi->getFrameFilter(n, d->node, frameCtx);

        const int pl[] = { 0, 1, 2 };
        const VSFrameRef *fr[] = {d->process[0] ? NULL : src, d->process[1] ? NULL : src, d->process[2] ? NULL : src};
        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, d->vi->width, d->vi->height, fr, pl, src, core);

        for (int plane = 0; plane < d->vi->format->numPlanes; plane++) {
            if (d->process[plane]) {
                int stride = vsapi->getStride(src, plane) / d->vi->format->bytesPerSample;
                int height = vsapi->getFrameHeight(src, plane);
                int width = vsapi->getFrameWidth(src, plane);
                const uint8_t * VS_RESTRICT srcp = vsapi->getReadPtr(src, plane);
                uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);

                if (d->vi->format->sampleType == stInteger) {
                    if (d->vi->format->bytesPerSample == 1) {
                        binarize_i8(srcp, dstp, width, height, stride, (uint8_t)d->thresholdi[plane], (uint8_t)d->v0i[plane], (uint8_t)d->v1i[plane]);

                    } else if (d->vi->format->bytesPerSample == 2) {
                        binarize_i16((const uint16_t *)srcp, (uint16_t *)dstp, width, height, stride, d->thresholdi[plane], d->v0i[plane], d->v1i[plane]);
                    }
                } else if (d->vi->format->sampleType == stFloat) {
                    if (d->vi->format->bytesPerSample == 4) {
                        binarize_f32((const float *)srcp, (float *)dstp, width, height, stride, d->thresholdf[plane], d->v0f[plane], d->v1f[plane]);
                    }
                }
            }
        }

        vsapi->freeFrame(src);
        return dst;
    }

    return 0;
}

static void VS_CC binarizeFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    BinarizeData *d = (BinarizeData *)instanceData;
    vsapi->freeNode(d->node);
    free(d);
}

void VS_CC binarizeCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    BinarizeData d;

    d.node = vsapi->propGetNode(in, "clip", 0, NULL);
    d.vi = vsapi->getVideoInfo(d.node);

    int num_planes = d.vi->format->numPlanes;

    bool prevValid;

    if (vsapi->propNumElements(in, "threshold") > num_planes) {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "ispc.Binarize: \"threshold\" has more values specified than there are planes");
        return;
    }

    prevValid = false;
    for (int i = 0; i < num_planes; i++) {
        int err;

        if (d.vi->format->sampleType == stInteger) {
            uint16_t temp = (uint16_t)(vsapi->propGetFloat(in, "threshold", i, &err) + .5);
            if (err) {
                temp = (i == 0) ? (1 << (d.vi->format->bitsPerSample - 1)) : d.thresholdi[i-1];
            } else {
                prevValid = true;
            }

            d.thresholdi[i] = temp;

        } else if (d.vi->format->sampleType == stFloat) {
            float temp = (float)vsapi->propGetFloat(in, "threshold", i, &err);
            if (err) {
                temp = prevValid ? d.thresholdf[i-1] : (((i > 0) && ((d.vi->format->colorFamily == cmYUV) || (d.vi->format->colorFamily == cmYCoCg))) ? 0.f : 0.5f);
            } else {
                prevValid = true;
            }

            d.thresholdf[i] = temp;
        }
    }

    if (vsapi->propNumElements(in, "v0") > num_planes) {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "ispc.Binarize: \"v0\" has more values specified than there are planes");
        return;
    }

    prevValid = false;
    for (int i = 0; i < num_planes; i++) {
        int err;

        if (d.vi->format->sampleType == stInteger) {
            uint16_t temp = (uint16_t)(vsapi->propGetFloat(in, "v0", i, &err) + .5);
            if (err) {
                temp = (i == 0) ? 0U : d.v0i[i-1];
            } else {
                prevValid = true;
            }

            d.v0i[i] = temp;

        } else if (d.vi->format->sampleType == stFloat) {
            float temp = (float)vsapi->propGetFloat(in, "v0", i, &err);
            if (err) {
                temp = prevValid ? d.v0f[i-1] : (((i > 0) && ((d.vi->format->colorFamily == cmYUV) || (d.vi->format->colorFamily == cmYCoCg))) ? -0.5f : 0.f);
            } else {
                prevValid = true;
            }

            d.v0f[i] = temp;
        }
    }

    if (vsapi->propNumElements(in, "v1") > num_planes) {
        vsapi->freeNode(d.node);
        vsapi->setError(out, "ispc.Binarize: \"v1\" has more values specified than there are planes");
        return;
    }

    prevValid = false;
    for (int i = 0; i < num_planes; i++) {
        int err;

        if (d.vi->format->sampleType == stInteger) {
            uint16_t temp = (uint16_t)(vsapi->propGetFloat(in, "v1", i, &err) + .5);
            if (err) {
                temp = (i == 0) ? ((1 << d.vi->format->bitsPerSample) - 1) : d.v1i[i-1];
            } else {
                prevValid = true;
            }

            d.v1i[i] = temp;

        } else if (d.vi->format->sampleType == stFloat) {
            float temp = (float)vsapi->propGetFloat(in, "v1", i, &err);
            if (err) {
                temp = prevValid ? d.v1f[i-1] : (((i > 0) && ((d.vi->format->colorFamily == cmYUV) || (d.vi->format->colorFamily == cmYCoCg))) ? 0.5f : 1.f);
            } else {
                prevValid = true;
            }

            d.v1f[i] = temp;
        }
    }

    const int m = vsapi->propNumElements(in, "planes");

    for (int i = 0; i < 3; i++)
        d.process[i] = (m <= 0);

    for (int i = 0; i < vsapi->propNumElements(in, "planes"); i++) {
        int plane = int64ToIntS(vsapi->propGetInt(in, "planes", i, NULL));

        if (plane < 0 || plane >= num_planes) {
            vsapi->freeNode(d.node);
            vsapi->setError(out, "ispc.Binarize: plane index out of range");
            return;
        }

        if (d.process[plane]) {
            vsapi->freeNode(d.node);
            vsapi->setError(out, "ispc.Binarize: plane specified twice");
            return;
        }

        d.process[plane] = true;
    }

    BinarizeData * const data = malloc(sizeof(d));
    *data = d;

    vsapi->createFilter(in, out, "Binarize", binarizeInit, binarizeGetFrame, binarizeFree, fmParallel, 0, data, core);
}

// Merge
static void VS_CC mergeInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    MergeData *d = (MergeData *) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mergeGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    MergeData *d = (MergeData *) * instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node1, frameCtx);
        vsapi->requestFrameFilter(n, d->node2, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src1 = vsapi->getFrameFilter(n, d->node1, frameCtx);
        const VSFrameRef *src2 = vsapi->getFrameFilter(n, d->node2, frameCtx);

        const int pl[] = { 0, 1, 2 };
        const VSFrameRef *fs[] = { NULL, src1, src2 }; // kMerge, kCopyFirst, kCopySecond
        const VSFrameRef *fr[] = {fs[d->process[0]], fs[d->process[1]], fs[d->process[2]]};
        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, d->vi->width, d->vi->height, fr, pl, src1, core);

        for (int plane = 0; plane < d->vi->format->numPlanes; plane++) {
            if (d->process[plane] == kMerge) {
                int stride = vsapi->getStride(src1, plane) / d->vi->format->bytesPerSample;
                int height = vsapi->getFrameHeight(src1, plane);
                int width = vsapi->getFrameWidth(src1, plane);
                const uint8_t * VS_RESTRICT srcp1 = vsapi->getReadPtr(src1, plane);
                const uint8_t * VS_RESTRICT srcp2 = vsapi->getReadPtr(src2, plane);
                uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);

                if (d->vi->format->sampleType == stInteger) {
                    if (d->vi->format->bytesPerSample == 1) {
                        merge_i8(srcp1, srcp2, dstp, width, height, stride, d->weighti[plane]);

                    } else if (d->vi->format->bytesPerSample == 2) {
                        merge_i16((const uint16_t *)srcp1, (const uint16_t *)srcp2, (uint16_t *)dstp, width, height, stride, d->weighti[plane]);
                    }
                } else if (d->vi->format->sampleType == stFloat) {
                    if (d->vi->format->bytesPerSample == 4) {
                        merge_f32((const float *)srcp1, (const float *)srcp2, (float *)dstp, width, height, stride, d->weightf[plane]);
                    }
                }
            }
        }

        vsapi->freeFrame(src1);
        vsapi->freeFrame(src2);
        return dst;
    }

    return 0;
}

static void VS_CC mergeFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    MergeData *d = (MergeData *)instanceData;
    vsapi->freeNode(d->node1);
    vsapi->freeNode(d->node2);
    free(d);
}

void VS_CC mergeCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    MergeData d;

    d.node1 = vsapi->propGetNode(in, "clipa", 0, NULL);
    d.node2 = vsapi->propGetNode(in, "clipb", 0, NULL);
    d.vi = vsapi->getVideoInfo(d.node1);

    if (!isConstantFormat(d.vi) || !isSameFormat(d.vi, vsapi->getVideoInfo(d.node2))) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.Merge: both clips must have constant format and dimensions, and the same format and dimensions");
        return;
    }

    if ((d.vi->format->colorFamily == cmCompat) || (vsapi->getVideoInfo(d.node2)->format->colorFamily == cmCompat)) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.Merge: compat formats are not supported");
        return;
    }

    if ((d.vi->format->sampleType == stInteger && d.vi->format->bytesPerSample != 1 && d.vi->format->bytesPerSample != 2)
        || (d.vi->format->sampleType == stFloat && d.vi->format->bytesPerSample != 4)) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.Merge: only 8-16 bit integer and 32 bit float input supported");
        return;
    }

    int num_planes = d.vi->format->numPlanes;

    if (vsapi->propNumElements(in, "weight") > num_planes) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.Merge: \"weight\" has more values specified than there are planes");
        return;
    }

    bool prevValid = false;
    for (int i = 0; i < 3; i++) {
        int err;

        float temp = (float)vsapi->propGetFloat(in, "weight", i, &err);
        if (err) {
            temp = prevValid ? d.weightf[i-1] : 0.5f;
        } else {
            if ((temp < 0.f) || (temp > 1.f)) {
                vsapi->freeNode(d.node1);
                vsapi->freeNode(d.node2);
                vsapi->setError(out, "ispc.Merge: \"weight\" must be between 0 and 1");
                return;
            }
            prevValid = true;
        }

        d.weightf[i] = temp;
        d.weighti[i] = (uint32_t)(d.weightf[i] * (1 << 15) + 0.5f);

        if (d.vi->format->sampleType == stInteger) {
            if (d.weighti[i] == 0U) {
                d.process[i] = kCopyFirst;
            } else if (d.weighti[i] == (1 << 15)) {
                d.process[i] = kCopySecond;
            } else {
                d.process[i] = kMerge;
            }
        } else if (d.vi->format->sampleType == stFloat) {
            if (d.weightf[i] == 0.f) {
                d.process[i] = kCopyFirst;
            } else if (d.weightf[i] == 1.f) {
                d.process[i] = kCopySecond;
            } else {
                d.process[i] = kMerge;
            }
        }
    }

    MergeData * const data = malloc(sizeof(d));
    *data = d;

    vsapi->createFilter(in, out, "Merge", mergeInit, mergeGetFrame, mergeFree, fmParallel, 0, data, core);
}

// MakeDiff
static void VS_CC makeDiffInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    MakeDiffData *d = (MakeDiffData *) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC makeDiffGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    MakeDiffData *d = (MakeDiffData *) * instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node1, frameCtx);
        vsapi->requestFrameFilter(n, d->node2, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src1 = vsapi->getFrameFilter(n, d->node1, frameCtx);
        const VSFrameRef *src2 = vsapi->getFrameFilter(n, d->node2, frameCtx);

        const int pl[] = { 0, 1, 2 };
        const VSFrameRef *fr[] = { d->process[0] ? NULL : src1, d->process[1] ? NULL : src1, d->process[2] ? NULL : src1 };
        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, d->vi->width, d->vi->height, fr, pl, src1, core);

        for (int plane = 0; plane < d->vi->format->numPlanes; plane++) {
            if (d->process[plane]) {
                int stride = vsapi->getStride(src1, plane) / d->vi->format->bytesPerSample;
                int height = vsapi->getFrameHeight(src1, plane);
                int width = vsapi->getFrameWidth(src1, plane);
                const uint8_t * VS_RESTRICT srcp1 = vsapi->getReadPtr(src1, plane);
                const uint8_t * VS_RESTRICT srcp2 = vsapi->getReadPtr(src2, plane);
                uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);

                if (d->vi->format->sampleType == stInteger) {
                    if (d->vi->format->bytesPerSample == 1) {
                        make_diff_i8(srcp1, srcp2, dstp, width, height, stride);

                    } else if (d->vi->format->bytesPerSample == 2) {
                        const int halfpoint = 1 << (d->vi->format->bitsPerSample - 1);
                        const int maxvalue = (1 << d->vi->format->bitsPerSample) - 1;

                        make_diff_i16((const uint16_t *)srcp1, (const uint16_t *)srcp2, (uint16_t *)dstp, width, height, stride, halfpoint, maxvalue);
                    }
                } else if (d->vi->format->sampleType == stFloat) {
                    if (d->vi->format->bytesPerSample == 4) {
                        make_diff_f32((const float *)srcp1, (const float *)srcp2, (float *)dstp, width, height, stride);
                    }
                }
            }
        }

        vsapi->freeFrame(src1);
        vsapi->freeFrame(src2);
        return dst;
    }

    return 0;
}

static void VS_CC makeDiffFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    MakeDiffData *d = (MakeDiffData *)instanceData;
    vsapi->freeNode(d->node1);
    vsapi->freeNode(d->node2);
    free(d);
}

void VS_CC makeDiffCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    MakeDiffData d;

    d.node1 = vsapi->propGetNode(in, "clipa", 0, NULL);
    d.node2 = vsapi->propGetNode(in, "clipb", 0, NULL);
    d.vi = vsapi->getVideoInfo(d.node1);

    if (!isConstantFormat(d.vi) || !isSameFormat(d.vi, vsapi->getVideoInfo(d.node2))) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.MakeDiff: both clips must have constant format and dimensions, and the same format and dimensions");
        return;
    }

    if ((d.vi->format->colorFamily == cmCompat) || (vsapi->getVideoInfo(d.node2)->format->colorFamily == cmCompat)) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.MakeDiff: compat formats are not supported");
        return;
    }

    if ((d.vi->format->sampleType == stInteger && d.vi->format->bytesPerSample != 1 && d.vi->format->bytesPerSample != 2)
        || (d.vi->format->sampleType == stFloat && d.vi->format->bytesPerSample != 4)) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.MakeDiff: only 8-16 bit integer and 32 bit float input supported");
        return;
    }

    int num_planes = d.vi->format->numPlanes;
    const int m = vsapi->propNumElements(in, "planes");

    for (int i = 0; i < 3; i++)
        d.process[i] = (m <= 0);

    for (int i = 0; i < vsapi->propNumElements(in, "planes"); i++) {
        int plane = int64ToIntS(vsapi->propGetInt(in, "planes", i, 0));

        if (plane < 0 || plane >= num_planes) {
            vsapi->freeNode(d.node1);
            vsapi->freeNode(d.node2);
            vsapi->setError(out, "ispc.MakeDiff: plane index out of range");
            return;
        }

        if (d.process[plane]) {
            vsapi->freeNode(d.node1);
            vsapi->freeNode(d.node2);
            vsapi->setError(out, "ispc.MakeDiff: plane specified twice");
            return;
        }

        d.process[plane] = true;
    }

    MakeDiffData * const data = malloc(sizeof(d));
    *data = d;

    vsapi->createFilter(in, out, "MakeDiff", makeDiffInit, makeDiffGetFrame, makeDiffFree, fmParallel, 0, data, core);
}

// MergeDiff
static void VS_CC mergeDiffInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    MergeDiffData *d = (MergeDiffData *) * instanceData;
    vsapi->setVideoInfo(d->vi, 1, node);
}

static const VSFrameRef *VS_CC mergeDiffGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    MergeDiffData *d = (MergeDiffData *) * instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node1, frameCtx);
        vsapi->requestFrameFilter(n, d->node2, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src1 = vsapi->getFrameFilter(n, d->node1, frameCtx);
        const VSFrameRef *src2 = vsapi->getFrameFilter(n, d->node2, frameCtx);

        const int pl[] = { 0, 1, 2 };
        const VSFrameRef *fr[] = { d->process[0] ? NULL : src1, d->process[1] ? NULL : src1, d->process[2] ? NULL : src1 };
        VSFrameRef *dst = vsapi->newVideoFrame2(d->vi->format, d->vi->width, d->vi->height, fr, pl, src1, core);

        for (int plane = 0; plane < d->vi->format->numPlanes; plane++) {
            if (d->process[plane]) {
                int stride = vsapi->getStride(src1, plane) / d->vi->format->bytesPerSample;
                int height = vsapi->getFrameHeight(src1, plane);
                int width = vsapi->getFrameWidth(src1, plane);
                const uint8_t * VS_RESTRICT srcp1 = vsapi->getReadPtr(src1, plane);
                const uint8_t * VS_RESTRICT srcp2 = vsapi->getReadPtr(src2, plane);
                uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);

                if (d->vi->format->sampleType == stInteger) {
                    if (d->vi->format->bytesPerSample == 1) {
                        merge_diff_i8(srcp1, srcp2, dstp, width, height, stride);

                    } else if (d->vi->format->bytesPerSample == 2) {
                        const int halfpoint = 1 << (d->vi->format->bitsPerSample - 1);
                        const int maxvalue = (1 << d->vi->format->bitsPerSample) - 1;

                        merge_diff_i16((const uint16_t *)srcp1, (const uint16_t *)srcp2, (uint16_t *)dstp, width, height, stride, halfpoint, maxvalue);
                    }
                } else if (d->vi->format->sampleType == stFloat) {
                    if (d->vi->format->bytesPerSample == 4) {
                        merge_diff_f32((const float *)srcp1, (const float *)srcp2, (float *)dstp, width, height, stride);
                    }
                }
            }
        }

        vsapi->freeFrame(src1);
        vsapi->freeFrame(src2);
        return dst;
    }

    return 0;
}

static void VS_CC mergeDiffFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    MergeDiffData *d = (MergeDiffData *)instanceData;
    vsapi->freeNode(d->node1);
    vsapi->freeNode(d->node2);
    free(d);
}

void VS_CC mergeDiffCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    MergeDiffData d;

    d.node1 = vsapi->propGetNode(in, "clipa", 0, NULL);
    d.node2 = vsapi->propGetNode(in, "clipb", 0, NULL);
    d.vi = vsapi->getVideoInfo(d.node1);

    if (!isConstantFormat(d.vi) || !isSameFormat(d.vi, vsapi->getVideoInfo(d.node2))) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.MergeDiff: both clips must have constant format and dimensions, and the same format and dimensions");
        return;
    }

    if ((d.vi->format->colorFamily == cmCompat) || (vsapi->getVideoInfo(d.node2)->format->colorFamily == cmCompat)) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.MergeDiff: compat formats are not supported");
        return;
    }

    if ((d.vi->format->sampleType == stInteger && d.vi->format->bytesPerSample != 1 && d.vi->format->bytesPerSample != 2)
        || (d.vi->format->sampleType == stFloat && d.vi->format->bytesPerSample != 4)) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "ispc.MergeDiff: only 8-16 bit integer and 32 bit float input supported");
        return;
    }

    int num_planes = d.vi->format->numPlanes;
    const int m = vsapi->propNumElements(in, "planes");

    for (int i = 0; i < 3; i++)
        d.process[i] = (m <= 0);

    for (int i = 0; i < vsapi->propNumElements(in, "planes"); i++) {
        int plane = int64ToIntS(vsapi->propGetInt(in, "planes", i, 0));

        if (plane < 0 || plane >= num_planes) {
            vsapi->freeNode(d.node1);
            vsapi->freeNode(d.node2);
            vsapi->setError(out, "ispc.MergeDiff: plane index out of range");
            return;
        }

        if (d.process[plane]) {
            vsapi->freeNode(d.node1);
            vsapi->freeNode(d.node2);
            vsapi->setError(out, "ispc.MergeDiff: plane specified twice");
            return;
        }

        d.process[plane] = true;
    }

    MergeDiffData * const data = malloc(sizeof(d));
    *data = d;

    vsapi->createFilter(in, out, "MergeDiff", mergeDiffInit, mergeDiffGetFrame, mergeDiffFree, fmParallel, 0, data, core);
}
