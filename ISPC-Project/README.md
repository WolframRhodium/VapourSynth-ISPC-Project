# Compilation

Requires [Intel SPMD Program Compiler(ISPC)](https://github.com/ispc/ispc).

```
ispc element_wise.ispc -o element_wise.obj -h element_wise_ispc.h --target=avx2-i32x16

gcc -shared -o ispc_project.dll -I "C:\Program Files (x86)\VapourSynth\sdk\include\vapoursynth" ispc_project.c element_wise.c element_wise.obj
```

Available compilation targets of ISPC can be found by running `ispc --help` and looking at the output for the "--target" option.
