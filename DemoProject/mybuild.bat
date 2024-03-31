@REM Measure-Command {cmake --build build --target Editor --config profile -- /m | Out-Default}
@REM Measure-Command {cmake --build build --target Editor --config debug -- /m | Out-Default}

cmake --build build --target Editor --config profile -- /m

@REM cmake --build build --target AtomSampleViewerStandalone --config debug -- /m
@REM cmake --build build --target Editor --config debug -- /m

@REM cmake --build build --target AutomatedTesting.GameLauncher --config profile -- /m

set BINDIR=.\build\bin\profile
set AP=AssetProcessor.exe
REM Will start AP if it's not running, will wait 3 seconds if the AP needed to be started.
tasklist /nh /fi "imagename eq %AP%" | find /i "%AP%" > nul || (start %BINDIR%\%AP% & timeout /t 3 /nobreak)


@REM cmake --build build --target Editor --config debug -- /m
