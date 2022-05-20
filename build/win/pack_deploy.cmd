@echo off

if defined APROJECTS ( echo %APROJECTS% ) else ( echo APROJECTS not set && exit /b 1 )

rem === Deploy built programs. ZIP, check with Windows Defender and copy them to network shares =============================

rem Override by envrionment variable for another target or leave empty for no copying - needs putty tools in path
rem set SSH_DEPLOY_TARGET=user@host:/data/alex/Public/Releases

pushd "%APROJECTS%\deploy"

rem ===========================================================================
rem ==== Pack Navdatareader ===================================================================
del Navdatareader.zip

"C:\Program Files\7-Zip\7z.exe" a Navdatareader.zip "Navdatareader"
IF ERRORLEVEL 1 goto :err

"C:\Program Files\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -DisableRemediation -File "%APROJECTS%\deploy\Navdatareader.zip"
IF ERRORLEVEL 1 goto :err

rem ===========================================================================
rem ==== Copy all =============================================================

set /p FILENAMETEMP=<"%APROJECTS%\deploy\Navdatareader\version.txt"
set FILENAME=%FILENAMETEMP: =%

if defined SSH_DEPLOY_TARGET (
pscp -i "%HOMEDRIVE%\%HOMEPATH%\.ssh\id_rsa" "Navdatareader.zip" "%SSH_DEPLOY_TARGET%/Navdatareader-win-%FILENAME%.zip"
)

popd

echo ---- Success ----

if not "%1" == "nopause" pause

exit /b 0

:err

echo **** ERROR ****

popd

pause

exit /b 1
