# colorpicker
An eye dropper color picker for Win and Mac in Qt.


## how to build this

### First, Download Qt5

*Don't use perbuild binary files from qt's offical site*

Downlad [qtbase-opensource-src-5.8.0.tar.xz](http://download.qt.io/archive/qt/5.8/5.8.0/submodules/qtbase-opensource-src-5.8.0.tar.xz) .
 if you need build on macOS, download [qtmacextras-opensource-src-5.8.0.tar.xz](http://download.qt.io/archive/qt/5.8/5.8.0/submodules/qtmacextras-opensource-src-5.8.0.tar.xz) .

If you have `wget` and `tar` in the system path, your can also do this

> `wget http://download.qt.io/archive/qt/5.8/5.8.0/submodules/qtbase-opensource-src-5.8.0.tar.xz`

> `wget http://download.qt.io/archive/qt/5.8/5.8.0/submodules/qtmacextras-opensource-src-5.8.0.tar.xz`

> `tar xf qtbase-opensource-src-5.8.0.tar.xz`

> `tar xf qtmacextras-opensource-src-5.8.0.tar.xz`

### Second, Build Qt5 from source

On Windows, *make sure you installed VS2015 SP1 before your try to build this*

> `cd qtbase-opensource-src-5.8.0`   <- change dir to the unzipped qtbase located

> `call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64`

> `cmd /c configure -prefix C:\Qt5\5.8\base\static-mt -release -opensource -confirm-license -nomake tests -nomake examples -no-qml-debug -no-dbus -mp -no-compile-examples -no-opengl -static -static-runtime`

> `nmake && nmake install`

On macOS, *make sure you installed XCode before your try to build this*

> `cd qtbase-opensource-src-5.8.0`   <- change dir to the unzipped qtbase located

> `./configure -prefix /usr/local/3rd/qt5/base/static -release -opensource -confirm-license -nomake tests -nomake examples -no-qml-debug -no-dbus -mp -no-compile-examples -no-opengl -static`

> `make && make install`

> `export PATH=/usr/local/3rd/qt5/base/static/bin:$PATH`

> `cd qtmacextras-opensource-src-5.8.0`   <- change current dir to the unzipped *qtmacextras* located

> `qmake && make install`


### Then, Get cmake and ninja

On Windows

Download [cmake-3.7.2-win64-x64.zip](https://cmake.org/files/v3.7/cmake-3.7.2-win64-x64.zip) , unzip it to `C:\CMake`

Download [ninja-win.zip](https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip) , copy `ninja.exe` from the zip file to `C:\CMake\bin`

On macOS
> `brew install cmake ninja`



### Next, Confirm if everything is fine.


On Windows, *start a new cmd then call these commands*
> `call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64`

> `set PATH=C:\CMake\bin;C:\Qt5\5.8\base\static-mt\bin;%PATH%`

> `cmake --version` <- your should see "cmake version 3.7....."

> `ninja --version` <- your should see "1.7.2"

> `qmake --version` <- your should see "QMake version 3.1...."

*do NOT close this cmd*


On macOS, *start a new terminal then call these commands*

> `export PATH=/usr/local/3rd/qt5/base/static/bin:$PATH`

> `cmake --version` <- your should see "cmake version 3.7....."

> `ninja --version` <- your should see "1.7.2"

> `qmake --version` <- your should see "QMake version 3.1...."


*do NOT close this terminal*



### Last, Download and Build this project

> `git clone git@github.com:mockingbot/colorpicker.git`

> `git submodule init && git submodule update`

On Windows,

> `cd dir_colorpicker_loacated`

> `build.bat`

On macOS,

> `cd dir_colorpicker_loacated`

> `build.sh`

the executable file exist in `tmp\build_win` on Windows and `tmp\build_mac` on macOS
