@echo off
setlocal

REM Diretorios das bibliotecas
set LIBS_DIR=libs
set SDL3_DIR=%LIBS_DIR%\SDL3-3.2.8\x86_64-w64-mingw32
set SDL3_IMAGE_DIR=%LIBS_DIR%\SDL3_image-3.2.4\x86_64-w64-mingw32
set SDL3_TTF_DIR=%LIBS_DIR%\SDL3_ttf-3.2.2\x86_64-w64-mingw32

REM Flags de compilacao
set CXXFLAGS=-std=c++17 -Wall -Wextra -O2 -static-libgcc -static-libstdc++
set INCLUDES=-I"%SDL3_DIR%\include" -I"%SDL3_IMAGE_DIR%\include" -I"%SDL3_TTF_DIR%\include"
set LDFLAGS=-L"%SDL3_DIR%\lib" -L"%SDL3_IMAGE_DIR%\lib" -L"%SDL3_TTF_DIR%\lib"
set LDLIBS=-lSDL3 -lSDL3_image -lSDL3_ttf

echo === Compilando Projeto 1 - Computacao Visual ===
if not exist build mkdir build

echo [1/4] Compilando main.cpp...
g++ %CXXFLAGS% %INCLUDES% -c src\main.cpp -o build\main.o
if %ERRORLEVEL% neq 0 goto error

echo [2/4] Compilando image_processor.cpp...
g++ %CXXFLAGS% %INCLUDES% -c src\image_processor.cpp -o build\image_processor.o
if %ERRORLEVEL% neq 0 goto error

echo [3/4] Compilando histogram.cpp...
g++ %CXXFLAGS% %INCLUDES% -c src\histogram.cpp -o build\histogram.o
if %ERRORLEVEL% neq 0 goto error

echo [4/4] Compilando gui.cpp...
g++ %CXXFLAGS% %INCLUDES% -c src\gui.cpp -o build\gui.o
if %ERRORLEVEL% neq 0 goto error

echo === Linkando executavel ===
g++ %CXXFLAGS% build\main.o build\image_processor.o build\histogram.o build\gui.o -o imgproc.exe %LDFLAGS% %LDLIBS%
if %ERRORLEVEL% neq 0 goto error

echo === Copiando DLLs ===
copy /Y "%SDL3_DIR%\bin\SDL3.dll" .
copy /Y "%SDL3_IMAGE_DIR%\bin\SDL3_image.dll" .
copy /Y "%SDL3_TTF_DIR%\bin\SDL3_ttf.dll" .

echo === Sucesso! ===
echo Para rodar: imgproc.exe caminho_da_imagem.png
goto end

:error
echo === ERRO NA COMPILACAO ===
exit /b 1

:end
pause
