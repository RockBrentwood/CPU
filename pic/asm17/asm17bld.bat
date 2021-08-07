@echo off
rem *
rem *   A simple batch file to build PIC17c42 executable
rem *   code.
rem *
rem *   This batch file assumes that ASM17 and LINK17
rem *   are in your DOS search path.
rem *
rem *   Enter the file name that you want to assemble
rem *   and link, with no extensions.
rem *
@echo on
asm17 %1
@echo off
IF ERRORLEVEL 1 goto goodbye
@echo on
link17 %1
:goodbye
