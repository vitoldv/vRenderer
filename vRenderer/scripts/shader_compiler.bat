
@echo off
setlocal enabledelayedexpansion

REM --- Default values ---
SET "OUTPUT_DIR="
SET "SHADER_INCLUDE_DIR="
SET "SHADER_FILES="
SET "HAS_ERRORS=0"

REM --- Find glslangValidator ---
SET "GLSLANG_VALIDATOR_PATH="
REM Check if glslangValidator is in the system PATH
where glslangValidator >nul 2>nul
IF %ERRORLEVEL% EQU 0 (
    SET "GLSLANG_VALIDATOR_PATH=glslangValidator"
    echo Found glslangValidator in system PATH.
) ELSE (
    REM Check for Vulkan SDK path environment variable
    IF DEFINED VK_SDK_PATH (
        SET "SDK_VALIDATOR_PATH=%VK_SDK_PATH%\Bin\glslangValidator.exe"
        IF EXIST "!SDK_VALIDATOR_PATH!" (
            SET "GLSLANG_VALIDATOR_PATH=!SDK_VALIDATOR_PATH!"
            echo Found glslangValidator via VK_SDK_PATH.
        )
    )
)

IF NOT DEFINED GLSLANG_VALIDATOR_PATH (
    echo ERROR: Could not find glslangValidator.exe.
    echo Please ensure it is in your system PATH or that the VK_SDK_PATH environment variable is set correctly.
    goto :ErrorExit
)

REM --- Argument Parsing ---
:ParseArgs
IF "%~1"=="" goto :ValidateArgs

IF /I "%~1"=="-o" (
    IF "%~2"=="" (
        echo ERROR: Missing value for the -o argument.
        goto :Usage
    )
    SET "OUTPUT_DIR=%~2"
    SHIFT
    SHIFT
    goto :ParseArgs
)

IF /I "%~1"=="-i" (
    IF "%~2"=="" (
        echo ERROR: Missing value for the -i argument.
        goto :Usage
    )
    SET "SHADER_INCLUDE_DIR=%~2"
    SHIFT
    SHIFT
    goto :ParseArgs
)

REM Collect remaining arguments as shader files
SET "SHADER_FILES=!SHADER_FILES! "%~1""
SHIFT
goto :ParseArgs

:ValidateArgs
REM --- Argument Validation ---
IF "%OUTPUT_DIR%"=="" (
    echo ERROR: No output directory provided. Use the -o flag.
    goto :Usage
)
IF "%SHADER_INCLUDE_DIR%"=="" (
    echo ERROR: No include directory provided. Use the -i flag.
    goto :Usage
)

REM Remove leading space from SHADER_FILES
IF DEFINED SHADER_FILES SET "SHADER_FILES=!SHADER_FILES:~1!"

IF NOT DEFINED SHADER_FILES (
    echo ERROR: No shader files provided.
    goto :Usage
)
IF "%SHADER_FILES%"=="" (
    echo ERROR: No shader files provided.
    goto :Usage
)


REM --- Directory Creation ---
IF NOT EXIST "%OUTPUT_DIR%" (
    echo Creating output directory: "%OUTPUT_DIR%"
    mkdir "%OUTPUT_DIR%"
)

echo --------------------------------------------------------------------------------------------------
echo Starting shader compilation process...
echo Using compiler: %GLSLANG_VALIDATOR_PATH%
echo Output Directory: "%OUTPUT_DIR%"
echo Include Directory: "%SHADER_INCLUDE_DIR%"
echo --------------------------------------------------------------------------------------------------

REM --- Compilation Loop ---
FOR %%F IN (%SHADER_FILES%) DO (
    SET "SHADER_SOURCE_FILE=%%~F"
    SET "SHADER_OUTPUT_FILE=%OUTPUT_DIR%\%%~nxF.spv"

    echo.
    echo Compiling: "!SHADER_SOURCE_FILE!"
    echo Output to  : "!SHADER_OUTPUT_FILE!"

    REM Execute the compilation command
    "%GLSLANG_VALIDATOR_PATH%" -o "!SHADER_OUTPUT_FILE!" -V "!SHADER_SOURCE_FILE!" -l -I"%SHADER_INCLUDE_DIR%"

    REM Check for compilation errors
    IF !ERRORLEVEL! NEQ 0 (
        echo ERROR: Failed to compile "!SHADER_SOURCE_FILE!". glslangValidator exited with error code !ERRORLEVEL!.
        SET "HAS_ERRORS=1"
    ) ELSE (
        echo Successfully compiled "!SHADER_SOURCE_FILE!"
    )
)

goto :EndCompile

:Usage
echo.
echo Usage: %~nx0 -o [output_dir] -i [include_dir] [shader1] [shader2] ...
echo.
echo   -o "output_dir"    (Required) The directory where compiled .spv files will be saved.
echo   -i "include_dir"   (Required) The directory for shader include files.
echo   [shader1]...       (Required) One or more shader source files to compile.
echo.
echo   Example: %~nx0 -o "build/shaders" -i "src/shaders" "shader.vert" "shader.frag"
echo.
goto :ErrorExit

:EndCompile
echo.
IF %HAS_ERRORS% EQU 1 (
    echo Shader compilation process finished with one or more errors.
    goto :ErrorExit
) ELSE (
    echo All shaders compiled successfully.
    echo --------------------------------------------------------------------------------------------------
    endlocal
    exit /b 0
)

:ErrorExit
endlocal
exit /b 1

