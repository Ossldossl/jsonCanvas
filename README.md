# jsonCanvas
json*C*anvas is an implementation of the [JSONCanvas Spec](https://jsoncanvas.org/spec/1.0) in C.
> This library does ONLY provide the ability to create JSONCanvas files, and not to render them!

# Usage
This library is in the style of the great [stb-headers](https://github.com/nothings/stb) which means you just have to include the header file in your project and you're done.
> NOTE: By default, jsoncanvas.h acts as a normal header file. When defining the JSONCANVAS_IMPLEMENTATION macro in _ONLY ONE_ source file, it pastes the implementation (what in this case would be "jsoncanvas.c") into the C-File it is included in. 
*INCLUDING jsoncanvas.h WITH THE MACRO IN MORE THAN ONE FILE LEADS TO ODR-VIOLATIONS*
```c
#define JSONCANVAS_IMPLEMENTATION // only define in *one* file
#include "jsoncanvas.h" // include everywhere you want
...
```
See _example.c_ for an example.

# Making changes
For comfort purposes, the actual development happens in src/_jsoncanvas.c and src/_jsoncanvas.h. Change those files for changes and generate the single-header file by running the _generate_header.py_ script.
<br>The default allocator can be changed by defining the _ALLOCATE_ and _FREE_ macros.