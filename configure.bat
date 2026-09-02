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

cmake --preset windows-vs2022
if ERRORLEVEL 1 (
    GOTO END
)

:END
