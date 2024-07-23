@echo off

setlocal enableextensions

if defined APROJECTS ( echo %APROJECTS% ) else ( echo APROJECTS not set && exit /b 1 )

rem =============================================================================
rem Set the required environment variable APROJECTS to the base directory for
rem atools and navdatareader.
rem
rem =============================================================================
rem Configuration can be overloaded on the command line by setting the
rem variables below before calling this script.
rem
rem See the *.pro project files for more information.
rem
rem Example:
rem set PATH_SHARED=C:\Qt\5.12.11\mingw73_32\bin;C:\Qt\Tools\mingw730_32\bin

if defined CONF_TYPE ( echo %CONF_TYPE% ) else ( set CONF_TYPE=release)
if defined ATOOLS_INC_PATH ( echo %ATOOLS_INC_PATH% ) else ( set ATOOLS_INC_PATH=%APROJECTS%\atools\src)
if defined ATOOLS_LIB_PATH ( echo %ATOOLS_LIB_PATH% ) else ( set ATOOLS_LIB_PATH=%APROJECTS%\build-atools-%CONF_TYPE%)
if defined DEPLOY_BASE ( echo %DEPLOY_BASE% ) else ( set DEPLOY_BASE=%APROJECTS%\deploy)
if defined ATOOLS_GIT_PATH ( echo %ATOOLS_GIT_PATH% ) else ( set ATOOLS_GIT_PATH=C:\Git\bin\git)

if defined ATOOLS_NO_CRASHHANDLER_64 ( set ATOOLS_NO_CRASHHANDLER=%ATOOLS_NO_CRASHHANDLER_64% ) else ( set ATOOLS_NO_CRASHHANDLER=true)
echo ATOOLS_NO_CRASHHANDLER=%ATOOLS_NO_CRASHHANDLER%

rem Defines the used Qt for all builds
if defined PATH_SHARED ( echo %PATH_SHARED% ) else ( set PATH_SHARED=C:\Qt\5.15.2\mingw81_64\bin;C:\Qt\Tools\mingw810_64\bin)

set ATOOLS_NO_GRIB=true
set ATOOLS_NO_GUI=true
set ATOOLS_NO_ROUTING=true
set ATOOLS_NO_TRACK=true
set ATOOLS_NO_USERDATA=true
set ATOOLS_NO_WEATHER=true
set ATOOLS_NO_WEB=true
set ATOOLS_NO_NAVSERVER=true

rem === Build atools and navdatareader =============================

rem ===========================================================================
rem First delete all deploy directories =======================================
pushd "%APROJECTS%\deploy"
del /S /Q /F "%APROJECTS%\deploy\Navdatareader"
for /f %%f in ('dir /ad /b "%APROJECTS%\deploy\Navdatareader"') do rd /s /q "%APROJECTS%\deploy\Navdatareader\%%f"
rd /S /Q "%APROJECTS%\deploy\Navdatareader"
IF ERRORLEVEL 1 goto :err
popd

setlocal
set PATH=%PATH%;%PATH_SHARED%

rem ===========================================================================
rem ========================== atools 64 bit
pushd "%APROJECTS%\build-atools-release"
del /S /Q /F "%APROJECTS%\build-atools-release"
for /f %%f in ('dir /ad /b "%APROJECTS%\build-atools-release"') do rd /s /q "%APROJECTS%\build-atools-release\%%f"
IF ERRORLEVEL 1 goto :err

qmake.exe "%APROJECTS%\atools\atools.pro" -spec win32-g++ CONFIG+=release
IF ERRORLEVEL 1 goto :err
mingw32-make.exe -j4
IF ERRORLEVEL 1 goto :err
popd

rem ===========================================================================
rem ========================== navdatareader 64 bit
setlocal
set PATH=%PATH%;%PATH_SHARED%
pushd "%APROJECTS%\build-navdatareader-release"
del /S /Q /F "%APROJECTS%\build-navdatareader-release"
for /f %%f in ('dir /ad /b "%APROJECTS%\build-navdatareader-release"') do rd /s /q "%APROJECTS%\build-navdatareader-release\%%f"
IF ERRORLEVEL 1 goto :err

qmake.exe "%APROJECTS%\navdatareader\navdatareader.pro" -spec win32-g++ CONFIG+=release
IF ERRORLEVEL 1 goto :err
mingw32-make.exe -j4
IF ERRORLEVEL 1 goto :err
mingw32-make.exe deploy
IF ERRORLEVEL 1 goto :err
popd
endlocal

echo ---- Success ----

if not "%1" == "nopause" pause

exit /b 0

:err

echo **** ERROR ****

popd

pause

exit /b 1
