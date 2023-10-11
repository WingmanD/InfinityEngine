SET buildPath=%~dp0
if not exist %buildPath%\Build mkdir %buildPath%\Build
cd %buildPath%\Build
conan install ../Engine/ --build=missing -s cppstd=23 -s build_type=Release
cmake .. -G "Visual Studio 17"