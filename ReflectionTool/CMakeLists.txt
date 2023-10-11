cmake_minimum_required(VERSION 3.26)

project(ReflectionTool)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE _files "${CMAKE_CURRENT_SOURCE_DIR}/Source/*")

add_executable(ReflectionTool ${_files})
target_include_directories(ReflectionTool PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${_files}")
export(TARGETS ReflectionTool FILE ReflectionToolTargets.cmake)
