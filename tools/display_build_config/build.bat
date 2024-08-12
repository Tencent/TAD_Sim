md mini
curl https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -o ./mini/ini.h
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
