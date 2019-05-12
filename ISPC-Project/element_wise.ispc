export void Invert_i8(const uniform unsigned int8 srcp[], uniform unsigned int8 dstp[], uniform int width, uniform int height, uniform int stride) {
    foreach (i = 0 ... height-1, j = 0 ... width-1) {
        (dstp + i * stride)[j] = ~(srcp + i * stride)[j];
    }
}

export void Invert_i16(const uniform unsigned int16 srcp[], uniform unsigned int16 dstp[], uniform int width, uniform int height, uniform int stride) {
    foreach (i = 0 ... height-1, j = 0 ... width-1) {
        (dstp + i * stride)[j] = ~(srcp + i * stride)[j];
    }
}

export void Invert_i16m(const uniform unsigned int16 srcp[], uniform unsigned int16 dstp[], uniform int width, uniform int height, uniform int stride, uniform unsigned int16 peak) {
    foreach (i = 0 ... height-1, j = 0 ... width-1) {
        (dstp + i * stride)[j] = peak - (srcp + i * stride)[j];
    }
}

export void Invert_f32(const uniform float srcp[], uniform float dstp[], uniform int width, uniform int height, uniform int stride, uniform bool uv) {
    if (uv) {
        foreach (i = 0 ... height-1, j = 0 ... width-1) {
            (dstp + i * stride)[j] = -(srcp + i * stride)[j];
        }
    } else {
        foreach (i = 0 ... height-1, j = 0 ... width-1) {
            (dstp + i * stride)[j] = 1.f - (srcp + i * stride)[j];
        }
    }
}
