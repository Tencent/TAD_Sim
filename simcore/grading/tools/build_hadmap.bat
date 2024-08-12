if defined VCPKG_ROOT (
    :: If the environment variable VCPKG_ROOT is defined, this command line will output its value.
    echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    :: If VCPKG_ROOT is not defined, this command line will prompt the user to set this variable
    echo Please set system variables: VCPKG_ROOT
    pause
)

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=off -S . -B _build_vs2022x64 -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build _build_vs2022x64 --config Release -j 16 --verbose

md hadmap
cd hadmap
md lib\x64\Release
md deps
md include
cd ..

xcopy datamodel\include .\hadmap\include /e /i /h
xcopy map_engine\include .\hadmap\include /e /i /h
xcopy map_import\include .\hadmap\include /e /i /h
xcopy mapdb\include .\hadmap\include /e /i /h
xcopy route_plan\include .\hadmap\include /e /i /h
xcopy routingmap\include .\hadmap\include /e /i /h
xcopy transmission\include .\hadmap\include /e /i /h

copy _build_vs2022x64\Release\*.lib .\hadmap\lib\x64\Release\
