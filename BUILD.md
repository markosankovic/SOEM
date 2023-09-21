# Build

## Windows

Download and install Microsoft Visual Studio Community 2022 (64-bit).

Start the Developer Command Prompt for VS 2022.

```sh
git clone https://github.com/markosankovic/SOEM.git
cd SOEM
git checkout -t origin/somanet
mkdir build
cd build
```

Change the CMakeLists.txt file to include only the test projects for Windows:

```cmake
add_subdirectory(test/simple_ng)
add_subdirectory(test/meas)
add_subdirectory(test/win32/simple_test)
add_subdirectory(test/win32/slaveinfo)
```

If you wish to use **nmake** for building:

```sh
cmake .. -G "NMake Makefiles"
nmake
```

For building the apps without debugging symbols, run the following instead:

```sh
cmake -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" ..
nmake
```

If you wish to use **Visual Studio** for building:

```sh
cmake -G "Visual Studio 17 2022" ..
```

The last command will generate a Visual Studio solution.

## SOMANET

Gets built with all other tests.

Run:

```sh
C:\SOEM\build\test\somanet\somanet.exe
```

APIs:

- http://localhost:8000/api/getAdapters

## Links

- [SOEM](https://ms-iot.github.io/ROSOnWindows/tutorials/ethercat/soem.html)
- [Mongoose](http://mongoose.ws/)
