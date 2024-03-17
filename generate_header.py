c_src = "";
h_src = "";
result = "";

with open("src/_jsoncanvas.c") as f:
    c_src = f.read();

with open("src/_jsoncanvas.h") as f:
    h_src = f.read();

result += (h_src);
result += c_src.replace('#include "_jsoncanvas.h"', "\n\n#ifdef JSONCANVAS_IMPLEMENTATION\n")
result += "\n#endif\n";

with open("jsoncanvas.h", "w") as f:
    f.write(result);
