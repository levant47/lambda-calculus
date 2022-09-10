@echo off
cl /Os /GS- main_windows.cpp /link /SUBSYSTEM:console /NODEFAULTLIB /ENTRY:main kernel32.lib user32.lib
del main_windows.obj
del main_windows.exe
