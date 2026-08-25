@echo off
setlocal enabledelayedexpansion
rem ============================================================
rem  AI C++ Fixer - portable installer (VS2022 / VS2026)
rem  Put CppAiFixer.vsix in the same folder as this script.
rem ============================================================

rem ---- 1. locate the .vsix (next to this script first) ----
set "VSIX=%~dp0CppAiFixer.vsix"
if not exist "%VSIX%" (
  echo [ERROR] CppAiFixer.vsix not found next to this script.
  echo Please put the .vsix file in the same folder and run again.
  pause
  exit /b 1
)

rem ---- 2. locate VSIXInstaller.exe ----
set "INST="
for %%E in (
  "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VSIXInstaller.exe"
  "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\VSIXInstaller.exe"
  "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\VSIXInstaller.exe"
  "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\VSIXInstaller.exe"
  "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\VSIXInstaller.exe"
  "C:\Program Files\Microsoft Visual Studio\18\Professional\Common7\IDE\VSIXInstaller.exe"
) do if exist %%E set "INST=%%~E"

if "%INST%"=="" (
  set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
  if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -products * -latest -requires Microsoft.VisualStudio.Component.CoreEditor -property installationPath`) do (
      if exist "%%i\Common7\IDE\VSIXInstaller.exe" set "INST=%%i\Common7\IDE\VSIXInstaller.exe"
    )
  )
)

if "%INST%"=="" (
  echo [ERROR] Visual Studio 2022/2026 not found on this machine.
  echo Alternative: double-click CppAiFixer.vsix directly.
  pause
  exit /b 1
)

rem ---- 3. install ----
echo Installing AI C++ Fixer ...
echo If a UAC prompt appears, please click Yes.
echo.
"%INST%" "%VSIX%"
echo.
echo Installer exit code: %errorlevel%
echo After install: close all VS windows, reopen Visual Studio,
echo then set your API key at Tools ^> Options ^> AI C++ Fixer.
pause
