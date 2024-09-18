@echo off

echo ============================================================================================
echo ======== pack_deploy.cmd ===================================================================
echo ============================================================================================

setlocal enableextensions

if defined APROJECTS ( echo APROJECTS=%APROJECTS% ) else ( echo APROJECTS not set && exit /b 1 )

rem Override by envrionment variable for another target or leave empty for no copying - needs putty tools in path
rem set SSH_DEPLOY_TARGET=user@host:/data/alex/Public/Releases

rem === Deploy built programs. ZIP, check with Windows Defender and copy them to network shares =============================

pushd "%APROJECTS%\deploy"
if errorlevel 1 goto :err

rem Get file version number and remove spaces from variable
rem Navdatareader-2.8.2.beta.zip
set /p FILENAMETEMP=<"%APROJECTS%\deploy\Navdatareader\version.txt"
set FILENAME_NDR=%FILENAMETEMP: =%
set FILENAME_NDR_RELEASE=Navdatareader-win-%FILENAME_NDR%

rem "%APROJECTS%\deploy\Navdatareader" to "%APROJECTS%\deploy\Navdatareader-1.2.0"
rmdir /Q /S "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%"
xcopy /i /s /e /f /y "%APROJECTS%\deploy\Navdatareader" "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%"
if errorlevel 1 goto :err

rem ===========================================================================
rem ==== Pack Navdatareader ===================================================
del "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%.zip"

rem Create, check and copy Zip =======================================
"C:\Program Files\7-Zip\7z.exe" -mx9 -mm=Deflate -mfb=258 -mpass=15 a "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%.zip" "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%" -xr!navdatareader.debug
if errorlevel 1 goto :err

"C:\Program Files\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -DisableRemediation -File "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%.zip"
if errorlevel 1 goto :err

if defined SSH_DEPLOY_TARGET (
pscp -i %HOMEDRIVE%\%HOMEPATH%\.ssh\id_rsa "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%.zip" %SSH_DEPLOY_TARGET%/%FILENAME_NDR_RELEASE%.zip
)

rem Create, check and copy Zip =======================================
"C:\Program Files\7-Zip\7z.exe" a -r "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%_debug.zip" "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%\*.debug"
if errorlevel 1 goto :err

"C:\Program Files\Windows Defender\MpCmdRun.exe" -Scan -ScanType 3 -DisableRemediation -File "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%_debug.zip"
if errorlevel 1 goto :err

if defined SSH_DEPLOY_TARGET (
pscp -i %HOMEDRIVE%\%HOMEPATH%\.ssh\id_rsa "%APROJECTS%\deploy\%FILENAME_NDR_RELEASE%_debug.zip" %SSH_DEPLOY_TARGET%/Debug/%FILENAME_NDR_RELEASE%_debug.zip"
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
