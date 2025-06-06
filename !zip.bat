call !make.bat
call !release.bat
del !game.zip
zip !game.zip game2.exe *.obj *.ttf *.png *.raw -D _BUILD\config.sys -D _BUILD\himemx.exe -D _BUILD\jemm386.exe -x desc.png
