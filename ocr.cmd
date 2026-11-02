@echo off
"%~dp0artifacts\win32-x64\bin\ocr.exe" --model-dir "%~dp0models" %*
exit /b %errorlevel%
