@echo off
clang example.c -fsanitize=address -o out/example.exe -O0 -gfull -g3 -Wall -Wno-incompatible-pointer-types -Wno-switch -Wno-microsoft-enum-forward-reference -Wno-unused-variable -Wno-unused-function
@echo on