cmake_minimum_required(VERSION 3.12)
project(InfinityEngine)

include(FetchContent)

FetchContent_Declare(
  DirectXTK12
  GIT_REPOSITORY https://github.com/microsoft/DirectXTK12.git
  GIT_TAG        main
  SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTK12"
)

FetchContent_MakeAvailable(DirectXTK12)

add_subdirectory(ReflectionTool)
add_subdirectory(Engine)
