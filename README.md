# VapourSynth-ISPC-Project
Implementation of algorithms for VapourSynth in ISPC.

Available functions:
```
ispc.Binarize(clip clip[, float[] threshold, float[] v0=0, float[] v1, int[] planes=[0, 1, 2]]) # std.Binarize
ispc.Invert(clip clip[, int[] planes=[0, 1, 2]]) # std.Invert
ispc.MakeDiff(clip clipa, clip clipb[, int[] planes=[0, 1, 2]]) # std.MakeDiff
ispc.MergeDiff(clip clipa, clip clipb[, int[] planes=[0, 1, 2]]) # std.MergeDiff
ispc.Merge(clip clipa, clip clipb[, float[] weight = 0.5]) # std.Merge
ispc.Limiter(clip clip[, float[] min, float[] max, int[] planes=[0, 1, 2]]) # std.Limiter
```
