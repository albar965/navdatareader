@echo off

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
set PATH=%PATH%;C:\Qt\5.9.1\mingw53_32\bin;C:\Qt\Tools\mingw530_32\bin

rem ===========================================================================
rem ========================== atools 32 bit
pushd "%APROJECTS%\build-atools-release"
del /S /Q /F "%APROJECTS%\build-atools-release"
for /f %%f in ('dir /ad /b "%APROJECTS%\build-atools-release"') do rd /s /q "%APROJECTS%\build-atools-release\%%f"
IF ERRORLEVEL 1 goto :err

qmake.exe "%APROJECTS%\atools\atools.pro" -spec win32-g++ CONFIG+=release
IF ERRORLEVEL 1 goto :err
mingw32-make.exe -j2
IF ERRORLEVEL 1 goto :err
popd

rem ===========================================================================
rem ========================== navdatareader 32 bit
setlocal
set PATH=%PATH%;C:\Qt\5.9.1\mingw53_32\bin;C:\Qt\Tools\mingw530_32\bin
pushd "%APROJECTS%\build-navdatareader-release"
del /S /Q /F "%APROJECTS%\build-navdatareader-release"
for /f %%f in ('dir /ad /b "%APROJECTS%\build-navdatareader-release"') do rd /s /q "%APROJECTS%\build-navdatareader-release\%%f"
IF ERRORLEVEL 1 goto :err

qmake.exe "%APROJECTS%\navdatareader\navdatareader.pro" -spec win32-g++ CONFIG+=release
IF ERRORLEVEL 1 goto :err
mingw32-make.exe -j2
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
