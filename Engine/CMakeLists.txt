cmake_minimum_required(VERSION 3.26)

project(Engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_UNITY_BUILD TRUE)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(_path "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE _files "${_path}/Source/*")
list(FILTER _files EXCLUDE REGEX "[A-Za-z0-9]+\.reflection\.h")
list(FILTER _files EXCLUDE REGEX "[A-Za-z0-9]+\.generated\.h")

file(GLOB_RECURSE _content "${_path}/Content/*")

add_subdirectory(Content/Shaders)

add_compile_definitions(NOMINMAX)

add_executable(Engine WIN32 ${_files})

target_include_directories(Engine PUBLIC "${_path}/Source" "${_files}")
target_include_directories(Engine PUBLIC ${directxtk12_SOURCE_DIR}/Inc)
target_include_directories(Engine PUBLIC ${AGILITYSDK_SOURCE_DIR}/build/native/include)
target_include_directories(Engine PUBLIC ${D3DCOMPILER_SOURCE_DIR}/inc)

target_compile_definitions(Engine PRIVATE "UNICODE" "_UNICODE")
set_target_properties(Engine PROPERTIES UNITY_BUILD_BATCH_SIZE 8)

if(MSVC)
    target_compile_options(Engine PRIVATE "/MP")
endif()

target_link_libraries(Engine PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib" "dxguid.lib" DirectXTK12 ${CONAN_LIBS})

# run ReflectionTool pre-build
add_custom_command(
    TARGET Engine
    PRE_BUILD
    COMMAND $<TARGET_FILE:ReflectionTool> ${_path}/Logs/ReflectionLog.txt ${_path}/Source
    COMMENT "Running ReflectionTool..."
)

set(SOURCE_DIR "${AGILITYSDK_SOURCE_DIR}/build/native/bin/x64")
set(DEST_DIR "${CMAKE_BINARY_DIR}/Engine/bin/D3D12")

add_custom_command(TARGET Engine POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${SOURCE_DIR}" "${DEST_DIR}")
                   
set(SOURCE_DIR "${D3DCOMPILER_SOURCE_DIR}/bin/x64")
set(DEST_DIR "${CMAKE_BINARY_DIR}/Engine/bin/D3D12")

add_custom_command(TARGET Engine POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${SOURCE_DIR}" "${DEST_DIR}")

set(SOURCE_DIR "${D3DCOMPILERPDB_SOURCE_DIR}/symbols/x64")
set(DEST_DIR "${CMAKE_BINARY_DIR}/Engine/bin/D3D12")

add_custom_command(TARGET Engine POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${SOURCE_DIR}" "${DEST_DIR}")

