@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set VCPKG=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
set "MAP_SDK_ROOT=%~dp0"
set "MAP_SDK_ROOT=%MAP_SDK_ROOT:~0,-1%"
set "MAP_SDK_BUILD=%MAP_SDK_ROOT%\hadmap"
@REM Setting build parmameters DATAMODEL
set "DATAMODEL_ROOT=%MAP_SDK_ROOT%\datamodel"
set "DATAMODEL_BUILD=%DATAMODEL_ROOT%\build"
@REM Setting build parmameters MAPDB
set "MAPDB_ROOT=%MAP_SDK_ROOT%\mapdb"
set "MAPDB_BUILD=%MAPDB_ROOT%\build"
@REM Setting build parmameters MAP_IMPORT
set "MAP_IMPORT_ROOT=%MAP_SDK_ROOT%\map_import"
set "MAP_IMPORT_BUILD=%MAP_IMPORT_ROOT%\build"
@REM Setting build parmameters TRANSMISSION
set "TRANSMISSION_ROOT=%MAP_SDK_ROOT%\transmission"
set "TRANSMISSION_BUILD=%TRANSMISSION_ROOT%\build"
@REM Setting build parmameters MAP_ENGINE
set "MAP_ENGINE_ROOT=%MAP_SDK_ROOT%\map_engine"
set "MAP_ENGINE_BUILD=%MAP_ENGINE_ROOT%\build"
@REM Setting build parmameters ROUTE_PLAN
set "ROUTE_PLAN_ROOT=%MAP_SDK_ROOT%\route_plan"
set "ROUTE_PLAN_BUILD=%ROUTE_PLAN_ROOT%\build"
@REM Setting build parmameters ROUTINGMAP
set "ROUTINGMAP_ROOT=%MAP_SDK_ROOT%\routingmap"
set "ROUTINGMAP_BUILD=%ROUTINGMAP_ROOT%\build"
@REM
set "SDK_NAME=hadmap.tar.gz"

@REM Clean & mkdir
IF EXIST "%MAP_SDK_ROOT%\%SDK_NAME%" del "%MAP_SDK_ROOT%\%SDK_NAME%"
IF EXIST "%MAP_SDK_BUILD%" rmdir /s /q "%MAP_SDK_BUILD%"
mkdir "%MAP_SDK_BUILD%\bin"
mkdir "%MAP_SDK_BUILD%\lib"
mkdir "%MAP_SDK_BUILD%\include"
@REM Clean & mkdir DATAMODEL
IF EXIST "%DATAMODEL_ROOT%\Release" rmdir /s /q "%DATAMODEL_ROOT%\Release"
IF EXIST "%DATAMODEL_ROOT%\Debug" rmdir /s /q "%DATAMODEL_ROOT%\Debug"
IF EXIST "%DATAMODEL_ROOT%\*.dll" del "%DATAMODEL_ROOT%\*.dll"
IF EXIST "%DATAMODEL_ROOT%\*.lib" del "%DATAMODEL_ROOT%\*.lib"
IF EXIST "%DATAMODEL_ROOT%\*.so" del "%DATAMODEL_ROOT%\*.so"
IF EXIST "%DATAMODEL_BUILD%" rmdir /s /q "%DATAMODEL_BUILD%"
mkdir "%DATAMODEL_BUILD%"
@REM Clean & mkdir MAPDB
IF EXIST "%MAPDB_ROOT%\Release" rmdir /s /q "%MAPDB_ROOT%\Release"
IF EXIST "%MAPDB_ROOT%\Debug" rmdir /s /q "%MAPDB_ROOT%\Debug"
IF EXIST "%MAPDB_ROOT%\*.dll" del "%MAPDB_ROOT%\*.dll"
IF EXIST "%MAPDB_ROOT%\*.lib" del "%MAPDB_ROOT%\*.lib"
IF EXIST "%MAPDB_ROOT%\*.so" del "%MAPDB_ROOT%\*.so"
IF EXIST "%MAPDB_BUILD%" rmdir /s /q "%MAPDB_BUILD%"
mkdir "%MAPDB_BUILD%"
@REM Clean & mkdir MAP_IMPORT
IF EXIST "%MAP_IMPORT_ROOT%\Release" rmdir /s /q "%MAP_IMPORT_ROOT%\Release"
IF EXIST "%MAP_IMPORT_ROOT%\Debug" rmdir /s /q "%MAP_IMPORT_ROOT%\Debug"
IF EXIST "%MAP_IMPORT_ROOT%\*.dll" del "%MAP_IMPORT_ROOT%\*.dll"
IF EXIST "%MAP_IMPORT_ROOT%\*.lib" del "%MAP_IMPORT_ROOT%\*.lib"
IF EXIST "%MAP_IMPORT_ROOT%\*.so" del "%MAP_IMPORT_ROOT%\*.so"
IF EXIST "%MAP_IMPORT_BUILD%" rmdir /s /q "%MAP_IMPORT_BUILD%"
mkdir "%MAP_IMPORT_BUILD%"
@REM Clean & mkdir TRANSMISSION
IF EXIST "%TRANSMISSION_ROOT%\Release" rmdir /s /q "%TRANSMISSION_ROOT%\Release"
IF EXIST "%TRANSMISSION_ROOT%\Debug" rmdir /s /q "%TRANSMISSION_ROOT%\Debug"
IF EXIST "%TRANSMISSION_ROOT%\*.dll" del "%TRANSMISSION_ROOT%\*.dll"
IF EXIST "%TRANSMISSION_ROOT%\*.lib" del "%TRANSMISSION_ROOT%\*.lib"
IF EXIST "%TRANSMISSION_ROOT%\*.so" del "%TRANSMISSION_ROOT%\*.so"
IF EXIST "%TRANSMISSION_BUILD%" rmdir /s /q "%TRANSMISSION_BUILD%"
mkdir "%TRANSMISSION_BUILD%"
@REM Clean & mkdir MAP_ENGINE
IF EXIST "%MAP_ENGINE_ROOT%\Release" rmdir /s /q "%MAP_ENGINE_ROOT%\Release"
IF EXIST "%MAP_ENGINE_ROOT%\Debug" rmdir /s /q "%MAP_ENGINE_ROOT%\Debug"
IF EXIST "%MAP_ENGINE_ROOT%\*.dll" del "%MAP_ENGINE_ROOT%\*.dll"
IF EXIST "%MAP_ENGINE_ROOT%\*.lib" del "%MAP_ENGINE_ROOT%\*.lib"
IF EXIST "%MAP_ENGINE_ROOT%\*.so" del "%MAP_ENGINE_ROOT%\*.so"
IF EXIST "%MAP_ENGINE_BUILD%" rmdir /s /q "%MAP_ENGINE_BUILD%"
mkdir "%MAP_ENGINE_BUILD%"
@REM Clean & mkdir ROUTE_PLAN
IF EXIST "%ROUTE_PLAN_ROOT%\Release" rmdir /s /q "%ROUTE_PLAN_ROOT%\Release"
IF EXIST "%ROUTE_PLAN_ROOT%\Debug" rmdir /s /q "%ROUTE_PLAN_ROOT%\Debug"
IF EXIST "%ROUTE_PLAN_ROOT%\*.dll" del "%ROUTE_PLAN_ROOT%\*.dll"
IF EXIST "%ROUTE_PLAN_ROOT%\*.lib" del "%ROUTE_PLAN_ROOT%\*.lib"
IF EXIST "%ROUTE_PLAN_ROOT%\*.so" del "%ROUTE_PLAN_ROOT%\*.so"
IF EXIST "%ROUTE_PLAN_BUILD%" rmdir /s /q "%ROUTE_PLAN_BUILD%"
mkdir "%ROUTE_PLAN_BUILD%"
@REM Clean & mkdir ROUTINGMAP
IF EXIST "%ROUTINGMAP_ROOT%\Release" rmdir /s /q "%ROUTINGMAP_ROOT%\Release"
IF EXIST "%ROUTINGMAP_ROOT%\Debug" rmdir /s /q "%ROUTINGMAP_ROOT%\Debug"
IF EXIST "%ROUTINGMAP_ROOT%\*.dll" del "%ROUTINGMAP_ROOT%\*.dll"
IF EXIST "%ROUTINGMAP_ROOT%\*.lib" del "%ROUTINGMAP_ROOT%\*.lib"
IF EXIST "%ROUTINGMAP_ROOT%\*.so" del "%ROUTINGMAP_ROOT%\*.so"
IF EXIST "%ROUTINGMAP_BUILD%" rmdir /s /q "%ROUTINGMAP_BUILD%"
mkdir "%ROUTINGMAP_BUILD%"

@REM build
@REM build datamodel
cd "%DATAMODEL_BUILD%"
echo "datamodel build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "datamodel build successfully."
echo.

@REM build mapdb
cd "%MAPDB_BUILD%"
echo "mapdb build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "mapdb build successfully."
echo.

@REM build map_import
cd "%MAP_IMPORT_BUILD%"
echo "map_import build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -DPROJ_NEW_API=1 -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "map_import build successfully."
echo.

@REM build transmission
cd "%TRANSMISSION_BUILD%"
echo "transmission build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "transmission build successfully."
echo.

@REM build map_engine
cd "%MAP_ENGINE_BUILD%"
echo "map_engine build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "map_engine build successfully."
echo.

@REM build route_plan
cd "%ROUTE_PLAN_BUILD%"
echo "route_plan build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "route_plan build successfully."
echo.

@REM build routingmap
cd "%ROUTINGMAP_BUILD%"
echo "routingmap build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG% -A x64
cmake --build . --config Release
copy ..\Release\*.lib ..
copy ..\Release\*.dll ..
echo "routingmap build successfully."
echo.

@REM deploy
xcopy "%DATAMODEL_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h
xcopy "%MAPDB_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h
xcopy "%MAP_IMPORT_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h
xcopy "%TRANSMISSION_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h
xcopy "%MAP_ENGINE_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h
xcopy "%ROUTE_PLAN_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h
xcopy "%ROUTINGMAP_ROOT%\include" "%MAP_SDK_BUILD%\include" /e /i /h

copy "%DATAMODEL_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"
copy "%MAPDB_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"
copy "%MAP_IMPORT_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"
copy "%TRANSMISSION_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"
copy "%MAP_ENGINE_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"
copy "%ROUTE_PLAN_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"
copy "%ROUTINGMAP_ROOT%\*.lib" "%MAP_SDK_BUILD%\lib\"

copy "%DATAMODEL_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"
copy "%MAPDB_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"
copy "%MAP_IMPORT_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"
copy "%TRANSMISSION_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"
copy "%MAP_ENGINE_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"
copy "%ROUTE_PLAN_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"
copy "%ROUTINGMAP_ROOT%\*.dll" "%MAP_SDK_BUILD%\bin\"

cd "%MAP_SDK_BUILD%"
tar zcvf "%MAP_SDK_ROOT%\%SDK_NAME%" ./*

@REM Change the working directory back to the original directory where the script was run
cd "%MAP_SDK_ROOT%"
