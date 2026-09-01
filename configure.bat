@ECHO OFF

:: remove old build files
if exist build (
	rmdir /s /q build
)
if exist examples\demo\frontend\dist (
	rmdir /s /q examples\demo\frontend\dist
)
if exist examples\demo\frontend\node_modules (
	rmdir /s /q examples\demo\frontend\node_modules
)

cmake -B build -S . -G "Visual Studio 17 2022" -A x64
if ERRORLEVEL 1 (
    GOTO END
)

:END
