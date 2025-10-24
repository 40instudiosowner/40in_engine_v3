# 40in_engine_v3

This repository contains the source code and resources for the 40in_engine_v3 project.
This project is designed to provide a robust and efficient engine for various applications.

## Build Instructions
To build the 40in_engine_v3 project, follow these steps:
### Windows:
1. Ensure you have [CMake](https://cmake.org/download/) installed on your system.
2. Open a command prompt and navigate to the project directory.
3. Clone the vcpkg repo (the simplest way to work with my project):
```bat
	git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
	C:\vcpkg\bootstrap-vcpkg.bat
```
3.1	Clone 40in_engine_v3 repo using command:
```bat
	git clone https://github.com/40instudiosowner/40in_engine_v3.git
```

4. If you are using Visual Studio 2022 and later versions, just open the project folder, CMake will see vcpkg.json.
   Then CMake will download and build all the dependencies automatically. (Equivalent to 'vcpkg install' command).
	Then if you hit F5 project should build and run (Equivalent to 'cmake --build --preset x64-your_vs_config' command).
5. If you want to build manually, there're 3 ways to do it:
	5.1 Classic:
	```bat
		cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
	```
	5.2 Using CMake presets:
	```bat
		cmake --preset x64-your_vs_config
	```
	After that project files will be generated in the 'build' folder.
	5.3 Total control:
	```bat
		cmake -G "Visual Studio 17 2022" -A x64 -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
	```
	Here you can change generator and architecture as you want.
	Then you can build the project using:
	```bat
		cmake --build --preset x64-your_vs_config
	```
6. If you don't have Visual Studio, you can use other build systems like Ninja or MinGW.
   Just make sure to specify the correct generator in the CMake command.
Ninja example:
1. Install [Ninja](https://ninja-build.org/).
2. Set environment path to include Ninja executable.
3. Check with command:
```bat
	ninja --version
```
4. Then generate build files and build the project:
```bat
	cmake -G "Ninja" -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build build
```
### Linux:
1. Ensure you have [CMake](https://cmake.org/download/) and a C++ compiler installed on your system.

