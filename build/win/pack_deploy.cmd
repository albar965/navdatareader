@echo off

rem === Deploy built programs. ZIP, check with Windows Defender and copy them to network shares =============================

for /f "delims=" %%# in ('powershell get-date -format "{yyyyMMdd-HHmm}"') do @set FILEDATE=%%#

echo Filedate %FILEDATE%

pushd "%APROJECTS%\deploy"


rem ===========================================================================
rem ==== Pack Navdatareader ===================================================================
del Navdatareader.zip

"C:\Program Files\7-Zip\7z.exe" a Navdatareader.zip "Navdatareader"
IF ERRORLEVEL 1 goto :err

"C:\Program Files\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -DisableRemediation -File "%APROJECTS%\deploy\Navdatareader.zip"
IF ERRORLEVEL 1 goto :err

del \\darkon\public\Navdatareader-%FILEDATE%.zip
copy /Y /Z /B Navdatareader.zip \\darkon\public\Releases\Navdatareader-win-%FILEDATE%.zip
IF ERRORLEVEL 1 goto :err

del \\frida\ssd\alex\Navdatareader-%FILEDATE%.zip
copy /Y /Z /B Navdatareader.zip \\frida\ssd\alex\Releases\Navdatareader-win-%FILEDATE%.zip
IF ERRORLEVEL 1 goto :err

popd

echo ---- Success ----

if not "%1" == "nopause" pause

exit /b 0

:err

echo **** ERROR ****

popd

pause

exit /b 1
