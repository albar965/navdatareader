@echo off

if defined APROJECTS ( echo %APROJECTS% ) else ( echo APROJECTS not set && exit /b 1 )

rem === Pull from atools and navdatareader repositories =============================

pushd "%APROJECTS%\atools"
"%ATOOLS_GIT_PATH%" pull --verbose  --tags
IF ERRORLEVEL 1 goto :err
popd

pushd "%APROJECTS%\navdatareader"
"%ATOOLS_GIT_PATH%" pull  --verbose  --tags
IF ERRORLEVEL 1 goto :err
popd

echo ---- Success ----

if not "%1" == "nopause" pause

exit /b 0

:err

popd

echo **** ERROR ****

pause

exit /b 1
