@echo off
set BuildDir=build
echo start to generate visual studio solution

if exist %BuildDir% (
    del /s /q %BuildDir%\*.*
    rd /s /q %BuildDir%
)

mkdir build

cd build

cmake -G "Visual Studio 15 2017 Win64" -T v140 ..
