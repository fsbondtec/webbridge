@ECHO OFF
SETLOCAL EnableDelayedExpansion

:: Default values
SET CONFIG=Debug
SET REBUILD=0

:: Parse command line arguments
:PARSE_ARGS
IF "%~1"=="" GOTO END_PARSE
IF /I "%~1"=="--debug" (
    SET CONFIG=Debug
    SHIFT
    GOTO PARSE_ARGS
)
IF /I "%~1"=="--release" (
    SET CONFIG=Release
    SHIFT
    GOTO PARSE_ARGS
)
IF /I "%~1"=="--rebuild" (
    SET REBUILD=1
    SHIFT
    GOTO PARSE_ARGS
)
IF /I "%~1"=="--help" (
    GOTO SHOW_HELP
)
IF /I "%~1"=="-h" (
    GOTO SHOW_HELP
)
ECHO Unknown argument: %~1
GOTO SHOW_HELP

:END_PARSE

:: Check if build directory exists
IF NOT EXIST build (
    ECHO Error: build directory not found. Please run configure.bat first.
    EXIT /B 1
)

:: Build command
ECHO.
ECHO Building configuration: %CONFIG%
IF %REBUILD%==1 (
    ECHO Mode: Rebuild ^(clean first^)
) ELSE (
    ECHO Mode: Incremental build
)
ECHO.

IF %REBUILD%==1 (
    cmake --build build --config %CONFIG% --clean-first
) ELSE (
    cmake --build build --config %CONFIG%
)

IF ERRORLEVEL 1 (
    ECHO.
    ECHO Build failed!
    EXIT /B 1
)

ECHO.
ECHO Build completed successfully!
EXIT /B 0

:SHOW_HELP
ECHO.
ECHO Usage: build.bat [OPTIONS]
ECHO.
ECHO Options:
ECHO   --debug          Build Debug configuration ^(default^)
ECHO   --release        Build Release configuration
ECHO   --rebuild        Clean and rebuild
ECHO   --help, -h       Show this help message
ECHO.
ECHO Examples:
ECHO   build.bat                    Build Debug ^(default^)
ECHO   build.bat --release          Build Release
ECHO   build.bat --rebuild          Rebuild Debug
ECHO   build.bat --rebuild --release    Rebuild Release
ECHO   build.bat --debug --rebuild      Rebuild Debug
ECHO.
EXIT /B 0
