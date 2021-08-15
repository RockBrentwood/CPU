@echo off
if "%1"=="" goto main

cd %1
type %1.doc | ..\..\addcr
set ASCMD=@asflags
..\..\asl -i ..\..\include -L +t 31 %1.asm
set ASCMD=
..\..\p2bin -l 0 -r $-$ %1
echo n >inp
comp %1.bin %1.ori >nul <inp
if errorlevel 1 goto errcond
echo Test %1 succeeded!
set SUMPASS=%SUMPASS%+
echo %1 : OK >> ..\..\testlog
:goon
del inp >nul
echo +---------------------------------------------------------------+
type %1.lst | find "assembly time" >> ..\..\testlog
type %1.lst | find "Assemblierzeit" >> ..\..\testlog
if exist %1.lst del %1.lst >nul
if exist %1.inc del %1.inc >nul
if exist %1.bin del %1.bin >nul
if exist %1.p del %1.p >nul
cd ..
goto end

:errcond
echo Test %1 failed!
set SUMFAIL=%SUMFAIL%-
echo %1 : failed >> ..\..\testlog
goto goon

:main
if exist ..\addcr.exe goto nocomp
gcc -o ..\addcr.exe ..\addcr.c
:nocomp
echo executing self tests...
echo "=================================================================" >..\testlog
echo Summaric results: >> ..\testlog
set SUMPASS=
set SUMFAIL=
call testall t_166
call testall t_16c5x
call testall t_16c84
call testall t_17c42
call testall t_251
call testall t_296
call testall t_29k
call testall t_32
call testall t_3201x
call testall t_3203x
call testall t_370
call testall t_403
call testall t_4500
call testall t_47c00
call testall t_48
call testall t_56000
call testall t_65
call testall t_6804
call testall t_6805
call testall t_6808
call testall t_6812
call testall t_6816
call testall t_7000
call testall t_7700
call testall t_75k0
call testall t_78c1x
call testall t_78k0
call testall t_85
call testall t_87c800
call testall t_8X30x
call testall t_96
call testall t_97c241
call testall t_9900
call testall t_avr
call testall t_bas52
call testall t_buf32
call testall t_cop8
call testall t_fl90
call testall t_fl900
call testall t_full09
call testall t_h8_3
call testall t_h8_5
call testall t_m16c
call testall t_mic51
call testall t_msp
call testall t_parsys
call testall t_scmp
call testall t_secdrive
call testall t_st6
call testall t_st7
call testall t_st9
call testall t_tms7
call testall t_xa
call testall t_z8
call testall t_z380
echo successes: %SUMPASS% >> ..\testlog
echo failures: %SUMFAIL% >> ..\testlog
type ..\testlog

:end