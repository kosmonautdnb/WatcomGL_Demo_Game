call !make.bat
call !release.bat
del !game.zip
zip !game.zip game2.exe *.obj *.ttf *.png *.raw -x desc.png
