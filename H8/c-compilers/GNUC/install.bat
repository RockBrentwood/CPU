@echo off
if exist disk1 goto disk1
echo. Please insert Disk 1 into the floppy drive
echo. and try the installation again.
goto syntax2

:disk1
if not "%1"=="" goto cont

:syntax
echo. Incorrect syntax.
:syntax2
echo. To install the H8/300 GNU Tool Set, use the following
echo. command line:
echo.               install drive:path
echo.                       where the complete drive and path
echo.                       are specified.
goto end

:cont
if exist %1\gnu-rel.txt goto reinst

echo Creating directory %1 ....
md %1

:reinst
echo Unpacking file 1 .....
pkunzip -d h83g-asm %1 > nul
echo Unpacking file 2 .....
pkunzip -d h83g-c %1 > nul
echo Unpacking file 3 .....
pkunzip -d h83g-dbg %1 > nul
echo Unpacking file 4 .....
pkunzip -d h83g-dem %1 > nul

:disk2
echo Please insert disk 2 and
pause

if not exist disk2 goto disk2

echo Unpacking file 5 .....
pkunzip -d h83g-inf %1 > nul
echo Unpacking file 6 .....
pkunzip -d h83g-lib %1 > nul
echo Unpacking file 7 .....
pkunzip -d h83g-lnk %1 > nul
echo Unpacking file 8 .....
pkunzip -d h83g-msc %1 > nul

echo Please be sure to read the release notes
echo found in the %1 directory.

goto end

:end
