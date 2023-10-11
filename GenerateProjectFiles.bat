SET buildPath=%~dp0
if not exist %buildPath%\Build mkdir %buildPath%\Build
cd %buildPath%\Build
conan install ../Engine/ --build=missing -s cppstd=23 -s build_type=Debug
cmake .. -G "Visual Studio 17"
@pause