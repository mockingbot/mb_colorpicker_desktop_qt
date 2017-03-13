@echo OFF
setlocal


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: check needed tool set ok

where /q cmake
if ERRORLEVEL 1 (
    echo command cmake does not find.
    echo make sure cmake is installed and placed in path.
    exit /B
)


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
setlocal enabledelayedexpansion
:: trace build time
set build_start_time=%time%


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
echo generate cmake files

:: select msvc for cmake
set CC=cl
set CXX=cl
set ld=link

set build_dir=tmp\build_win

if not exist %build_dir% mkdir %build_dir%
pushd %build_dir% 1> nul
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../..
popd 1> nul

echo ninja build

pushd %build_dir% 1> nul
ninja
popd 1> nul



::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:END
set build_end_time=%time%
echo use time %build_start_time% -- %build_end_time%
endlocal

