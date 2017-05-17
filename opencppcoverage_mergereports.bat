@echo off
setlocal enabledelayedexpansion
set myvar=%1 --verbose --export_type=cobertura:coverage.xml  
for /r %%i in (*.cov) do (

  call :concat %%~nxi
)
echo %myvar%
%myvar%
goto :eof

:concat
set myvar=%myvar% --input_coverage %1
goto :eof
