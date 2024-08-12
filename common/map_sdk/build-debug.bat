SET VCPKG=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

@echo off
set "folder=hadmap-debug"
if exist "%build_dir%" (
    echo hadmap dir exists. Calling clean.bat...
    call clean.bat
) else (
    echo hadmap dir does not exist.
)

cd datamodel
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

cd mapdb
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

cd map_import
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -DPROJ_NEW_API=1 -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

cd transmission
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

cd map_engine
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

cd route_plan
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

cd routingmap
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Debug
copy ..\Debug\*.lib ..
copy ..\Debug\*.dll ..
cd ..\..

md hadmap-debug
cd hadmap-debug
md lib
md bin
md include
cd ..

xcopy datamodel\include .\hadmap-debug\include /e /i /h
xcopy map_engine\include .\hadmap-debug\include /e /i /h
xcopy map_import\include .\hadmap-debug\include /e /i /h
xcopy mapdb\include .\hadmap-debug\include /e /i /h
xcopy route_plan\include .\hadmap-debug\include /e /i /h
xcopy routingmap\include .\hadmap-debug\include /e /i /h
xcopy transmission\include .\hadmap-debug\include /e /i /h

copy datamodel\*.lib .\hadmap-debug\lib\
copy map_engine\*.lib .\hadmap-debug\lib\
copy map_import\*.lib .\hadmap-debug\lib\
copy mapdb\*.lib .\hadmap-debug\lib\
copy route_plan\*.lib .\hadmap-debug\lib\
copy routingmap\*.lib .\hadmap-debug\lib\
copy transmission\*.lib .\hadmap-debug\lib\

copy datamodel\*.dll .\hadmap-debug\bin\
copy map_engine\*.dll .\hadmap-debug\bin\
copy map_import\*.dll .\hadmap-debug\bin\
copy mapdb\*.dll .\hadmap-debug\bin\
copy route_plan\*.dll .\hadmap-debug\bin\
copy routingmap\*.dll .\hadmap-debug\bin\
copy transmission\*.dll .\hadmap-debug\bin\
