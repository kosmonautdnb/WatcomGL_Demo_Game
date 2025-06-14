call !make.bat
call !release.bat
deltree src
mkdir src
mkdir src\_build
copy *.cpp src
copy *.hpp src
copy *.h src
copy *.bat src
copy *. src
copy _build\*.* src\_build\
del !demo1.zip
zip !demo1.zip game2.exe *.obj *.ttf *.png *.raw src\*.* src\_build\*.* -x desc.png
