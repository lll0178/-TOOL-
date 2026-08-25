@echo off
setlocal enabledelayedexpansion
rem ============================================================
rem  AI C++ Fixer - portable installer (VS2022 / VS2026)
rem ============================================================

set "VSIX=%~dp0CppAiFixer.vsix"
if not exist "%VSIX%" set "VSIX=%~dp0CppAiFixer\CppAiFixer\bin\Release\net472\CppAiFixer.vsix"
if not exist "%VSIX%" (
  echo [ERROR] CppAiFixer.vsix not found.
  pause
  exit /b 1
)

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
  echo [ERROR] Visual Studio not found. You can also double-click the .vsix directly.
  pause
  exit /b 1
)

echo Installing AI C++ Fixer ...
echo If a UAC prompt appears, please click Yes.
"%INST%" "%VSIX%"
echo.
echo Installer exit code: %errorlevel%
pause
