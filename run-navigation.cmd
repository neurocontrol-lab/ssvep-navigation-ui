@echo off
setlocal
cd /d "%~dp0"
set "NAVIGATION_PYTHON=%SSVEP_NAVIGATION_PYTHON%"
if not defined NAVIGATION_PYTHON if exist ".venv\Scripts\python.exe" set "NAVIGATION_PYTHON=%~dp0.venv\Scripts\python.exe"
if not defined NAVIGATION_PYTHON if exist "%USERPROFILE%\.conda\envs\venv\python.exe" set "NAVIGATION_PYTHON=%USERPROFILE%\.conda\envs\venv\python.exe"
if not defined NAVIGATION_PYTHON set "NAVIGATION_PYTHON=python.exe"
"%NAVIGATION_PYTHON%" "launcher\navigation_launcher.py"
if errorlevel 1 pause
