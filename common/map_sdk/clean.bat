@echo off

cd datamodel
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd mapdb
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd map_import
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd transmission
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd map_engine
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd route_plan
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd routingmap
rd /s /q build
rd /s /q Release
rd /s /q Debug
DEL *.dll
DEL *.lib
DEL *.so
cd ..

cd hadmap

rd /s /q bin
rd /s /q include
rd /s /q lib
DEL *.dll
DEL *.lib
DEL *.so

cd ..
rd /s /q hadmap
rd /s /q hadmap-debug
DEL hadmap.tar.gz
