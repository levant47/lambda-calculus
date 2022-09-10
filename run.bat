@echo off

REM have to do this because otherwise cl will not be able to compile
mkdir temp

REM compiler options:
REM /Fo specifies directory that's going to be used for temporary build files
REM /Fe specifies output name
REM /Gy collapse identical functions, https://stackoverflow.com/a/629978
REM /GS- disable buffer security checks
REM /Zl ignore CRT when compiling object files
REM /I specified additional directories to search #include-d files in
REM linker options:
REM /NODEFAULTLIB ignore CRT when linking
REM /ENTRY:main override default entry point which is CRT's main

cl ^
    /Fo.\temp\ /Fe.\temp\lci_tests.exe ^
    /Gy /GS- /Zl ^
    /I. ^
    test\test.cpp ^
    /link /NODEFAULTLIB /ENTRY:main /SUBSYSTEM:console ^
    kernel32.lib ^
    || exit /b
temp\lci_tests
