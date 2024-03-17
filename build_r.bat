@echo off
py generate_header.py
clang example.c -o out/example.exe -O3