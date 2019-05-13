// Invert
export void invert_i8(const uniform unsigned int8 srcp[], uniform unsigned int8 dstp[], 
                      uniform int width, uniform int height, uniform int stride) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int8 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = ~src;
    }
}

export void invert_i16(const uniform unsigned int16 srcp[], uniform unsigned int16 dstp[], 
                       uniform int width, uniform int height, uniform int stride) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int16 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = ~src;
    }
}

export void invert_i16m(const uniform unsigned int16 srcp[], uniform unsigned int16 dstp[], 
                        uniform int width, uniform int height, uniform int stride, 
                        uniform unsigned int16 peak) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int16 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = peak - src;
    }
}

export void invert_f32(const uniform float srcp[], uniform float dstp[], 
                       uniform int width, uniform int height, 
                       uniform int stride, uniform bool uv) {
    if (uv) {
        foreach (i = 0 ... height, j = 0 ... width) {
            const float src = (srcp + i * stride)[j];

            (dstp + i * stride)[j] = -src;
        }
    } else {
        foreach (i = 0 ... height, j = 0 ... width) {
            const float src = (srcp + i * stride)[j];

            (dstp + i * stride)[j] = 1.f - src;
        }
    }
}

// Limiter
export void limiter_i8(const uniform unsigned int8 srcp[], uniform unsigned int8 dstp[], 
                       uniform int width, uniform int height, uniform int stride, 
                       uniform unsigned int8 low, uniform unsigned int8 high) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int8 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = clamp(src, low, high);
    }
}

export void limiter_i16(const uniform unsigned int16 srcp[], uniform unsigned int16 dstp[], 
                        uniform int width, uniform int height, uniform int stride, 
                        uniform unsigned int16 low, uniform unsigned int16 high) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int16 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = clamp(src, low, high);
    }
}

export void limiter_f32(const uniform float srcp[], uniform float dstp[], 
                        uniform int width, uniform int height, 
                        uniform int stride, uniform float low, 
                        uniform float high) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const float src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = clamp(src, low, high);
    }
}

// Binarize
export void binarize_i8(const uniform unsigned int8 srcp[], uniform unsigned int8 dstp[], 
                        uniform int width, uniform int height, uniform int stride, 
                        uniform unsigned int8 threshold, uniform unsigned int8 v0, 
                        uniform unsigned int8 v1) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int8 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = (src < threshold) ? v0 : v1;
    }
}

export void binarize_i16(const uniform unsigned int16 srcp[], uniform unsigned int16 dstp[], 
                         uniform int width, uniform int height, uniform int stride, 
                         uniform unsigned int16 threshold, uniform unsigned int16 v0, 
                         uniform unsigned int16 v1) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const unsigned int16 src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = (src < threshold) ? v0 : v1;
    }
}

export void binarize_f32(const uniform float srcp[], uniform float dstp[], 
                         uniform int width, uniform int height, 
                         uniform int stride, uniform float threshold, 
                         uniform float v0, uniform float v1) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const float src = (srcp + i * stride)[j];

        (dstp + i * stride)[j] = (src < threshold) ? v0 : v1;
    }
}

// Merge
export void merge_i8(const uniform unsigned int8 srcp1[], const uniform unsigned int8 srcp2[], 
                     uniform unsigned int8 dstp[], uniform int width, uniform int height, 
                     uniform int stride, uniform int32 weight) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const int32 src1 = (srcp1 + i * stride)[j];
        const int32 src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = src1 + (((src2 - src1) * weight + (1 << 14)) >> 15);
    }
}

export void merge_i16(const uniform unsigned int16 srcp1[], const uniform unsigned int16 srcp2[],
                      uniform unsigned int16 dstp[], uniform int width, uniform int height, 
                      uniform int stride, uniform int32 weight) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const int32 src1 = (srcp1 + i * stride)[j];
        const int32 src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = src1 + (((src2 - src1) * weight + (1 << 14)) >> 15);
    }
}

export void merge_f32(const uniform float srcp1[], const uniform float srcp2[], 
                         uniform float dstp[], uniform int width, uniform int height, 
                         uniform int stride, uniform float weight) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const float src1 = (srcp1 + i * stride)[j];
        const float src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = src1 + ((src2 - src1) * weight);
    }
}

// MakeDiff
export void make_diff_i8(const uniform unsigned int8 srcp1[], const uniform unsigned int8 srcp2[], 
                     uniform unsigned int8 dstp[], uniform int width, uniform int height, 
                     uniform int stride) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const int32 src1 = (srcp1 + i * stride)[j];
        const int32 src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = clamp((src1 - src2) + 128, 0, 255);
    }
}

export void make_diff_i16(const uniform unsigned int16 srcp1[], const uniform unsigned int16 srcp2[],
                      uniform unsigned int16 dstp[], uniform int width, uniform int height, 
                      uniform int stride, uniform int32 halfpoint, uniform int32 maxvalue) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const int32 src1 = (srcp1 + i * stride)[j];
        const int32 src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = clamp(src1 - src2 + halfpoint, 0, maxvalue);
    }
}

export void make_diff_f32(const uniform float srcp1[], const uniform float srcp2[], 
                         uniform float dstp[], uniform int width, uniform int height, 
                         uniform int stride) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const float src1 = (srcp1 + i * stride)[j];
        const float src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = src1 - src2;
    }
}

// MergeDiff
export void merge_diff_i8(const uniform unsigned int8 srcp1[], const uniform unsigned int8 srcp2[], 
                     uniform unsigned int8 dstp[], uniform int width, uniform int height, 
                     uniform int stride) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const int32 src1 = (srcp1 + i * stride)[j];
        const int32 src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = clamp(src1 + src2 - 128, 0, 255);
    }
}

export void merge_diff_i16(const uniform unsigned int16 srcp1[], const uniform unsigned int16 srcp2[],
                      uniform unsigned int16 dstp[], uniform int width, uniform int height, 
                      uniform int stride, uniform int32 halfpoint, uniform int32 maxvalue) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const int32 src1 = (srcp1 + i * stride)[j];
        const int32 src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = clamp(src1 + src2 - halfpoint, 0, maxvalue);
    }
}

export void merge_diff_f32(const uniform float srcp1[], const uniform float srcp2[], 
                         uniform float dstp[], uniform int width, uniform int height, 
                         uniform int stride) {
    foreach (i = 0 ... height, j = 0 ... width) {
        const float src1 = (srcp1 + i * stride)[j];
        const float src2 = (srcp2 + i * stride)[j];

        (dstp + i * stride)[j] = src1 + src2;
    }
}
