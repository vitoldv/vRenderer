@echo off
setlocal

REM --- Configuration ---
REM Set the full path to your glslangValidator.exe (PATH might be provided instead)
SET GLSLANG_VALIDATOR_PATH="C:/VulkanSDK/1.4.309.0/Bin/glslangValidator.exe"

REM Check if any shader files were provided
IF "%~1"=="" (
    echo No shader files provided.
    echo Usage: %~nx0 path\to\shader1.vert [path\to\shader2.frag ...]
    goto :eof
)

echo --------------------------------------------------------------------------------------------------
echo Starting shader compilation process...
echo Using compiler: %GLSLANG_VALIDATOR_PATH%

:CompileLoop
REM If there are no more arguments, exit the loop
IF "%~1"=="" goto :EndCompile

SET "SHADER_SOURCE_FILE=%~1"
REM Construct the output file path by appending .spv to the source file path
SET "SHADER_OUTPUT_FILE=%SHADER_SOURCE_FILE%.spv"

echo. 
echo Compiling: "%SHADER_SOURCE_FILE%"
echo Output to  : "%SHADER_OUTPUT_FILE%"

REM Execute the compilation command
%GLSLANG_VALIDATOR_PATH% -o "%SHADER_OUTPUT_FILE%" -V "%SHADER_SOURCE_FILE%"

REM Check for compilation errors
IF ERRORLEVEL 1 (
    echo ERROR: Failed to compile "%SHADER_SOURCE_FILE%". glslangValidator exited with error code %ERRORLEVEL%.
) ELSE (
    echo Successfully compiled "%SHADER_SOURCE_FILE%"
)

REM Move to the next shader file argument
SHIFT
goto :CompileLoop

:EndCompile
endlocal
echo.
echo Shader compilation process finished.
echo --------------------------------------------------------------------------------------------------