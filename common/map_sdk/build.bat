@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

set VCPKG=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

@echo off
set "folder=hadmap"
if exist "%build_dir%" (
    echo hadmap dir exists. Calling clean.bat...
    call clean.bat
) else (
    echo hadmap dir does not exist.
)

cd datamodel
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

cd mapdb
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

cd map_import
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -DPROJ_NEW_API=1 -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

cd transmission
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

cd map_engine
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

cd route_plan
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

cd routingmap
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
cd ..\..

md hadmap
cd hadmap
md lib
md bin
md include
cd ..

xcopy datamodel\include .\hadmap\include /e /i /h
xcopy map_engine\include .\hadmap\include /e /i /h
xcopy map_import\include .\hadmap\include /e /i /h
xcopy mapdb\include .\hadmap\include /e /i /h
xcopy route_plan\include .\hadmap\include /e /i /h
xcopy routingmap\include .\hadmap\include /e /i /h
xcopy transmission\include .\hadmap\include /e /i /h

copy datamodel\*.lib .\hadmap\lib\
copy map_engine\*.lib .\hadmap\lib\
copy map_import\*.lib .\hadmap\lib\
copy mapdb\*.lib .\hadmap\lib\
copy route_plan\*.lib .\hadmap\lib\
copy routingmap\*.lib .\hadmap\lib\
copy transmission\*.lib .\hadmap\lib\

copy datamodel\*.dll .\hadmap\bin\
copy map_engine\*.dll .\hadmap\bin\
copy map_import\*.dll .\hadmap\bin\
copy mapdb\*.dll .\hadmap\bin\
copy route_plan\*.dll .\hadmap\bin\
copy routingmap\*.dll .\hadmap\bin\
copy transmission\*.dll .\hadmap\bin\

tar zcvf hadmap.tar.gz hadmap
