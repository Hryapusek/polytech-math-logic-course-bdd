# Description
Polytech math logic course work. Code supports both vertical and horizontal skleika.  
You can set your own neighbours in Conditions.cpp file. Code contains description  
and examples of all six condition types.

main.cpp file contains short description of course work in other words. Everything
goes with practical examples. If you don't understand something in task document,
then you probably will better understand it from commentaries in the code.

# Build
First build libbuddy.a file and put it into lib directory. File that already 
in lib directory is build for AMD64 and there is no guarantee that it is
compatible with your processor.

Also compiler required that supports C++23 features. For example [g++](http://www.equation.com/servlet/equation.cmd?fa=fortran). Download ***latest*** version!  
Install [CMake](https://cmake.org/download/) after this.

## Windows
First install [Ninja](https://github.com/ninja-build/ninja/releases) and add it's executable to PATH variable.  
Then go to project directory and execute in powershell the next script.
```powershell
cmake -G Ninja
mkdir build
cd build
cmake -S ../ -B .
cmake --build . --target bdd_main
```
After this you should see **bdd_main** executable in build directory.

## Linux
Install cmake on linux and just go with
```powershell
mkdir build
cd build
cmake -S ../ -B .
cmake --build . --target bdd_main
```
After this you should see **bdd_main** executable in build directory.
