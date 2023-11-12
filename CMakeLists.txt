cmake_minimum_required(VERSION 3.12)
project(InfinityEngine)

include(FetchContent)

FetchContent_Declare(
  DirectXTK
  GIT_REPOSITORY https://github.com/microsoft/DirectXTK.git
  GIT_TAG        main
  SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTK"
)

FetchContent_MakeAvailable(DirectXTK)

add_subdirectory(ReflectionTool)
add_subdirectory(Engine)
