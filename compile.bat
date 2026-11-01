@echo off
chcp 65001 >nul

del /q sqlite3.o main.exe output.log 2>nul

"C:\Program Files\JetBrains\CLion2026.2\bin\mingw\bin\gcc.exe" -c sqlite3.c -o sqlite3.o
if errorlevel 1 (
    echo Ошибка компиляции sqlite3.c
    pause
    exit /b 1
)

"C:\Program Files\JetBrains\CLion2026.2\bin\mingw\bin\g++.exe" main.cpp sqlite3.o -o main.exe > output.log 2>&1
if errorlevel 1 (
    echo Ошибка компиляции main.cpp
    pause
    exit /b 1
)

main.exe
pause