cd build 
rd /s /q bin
rd /s /q lib
cd ..
rd /s /q build

cd opendrive_io
rd /s /q build
cd ..

cd hadmap_server/map_parser
rd /s /q build
rd /s /q bin

cd protobuf
rd /s /q build
cd ../../..

cd service
DEL *.exe
