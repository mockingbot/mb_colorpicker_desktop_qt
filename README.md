# colorpicker
An eye dropper color picker for Win and Mac in Qt.


## how to build this

0. Get lastest `qt5 source`, `cmake` and `ninja` for building this,
   then build qt5 with `-static -static-runtime` flags
1. run `git submodule init && git submodule update`,
   this will fetch a cmake helper file for automatic handle qt runtime
2. call `build.bat` on Windows,
   call `build.sh` on macOS. the output executable file exist in tmp\build_win on Windows and tmp\build_mac on macOS
