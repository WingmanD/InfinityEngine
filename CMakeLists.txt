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

file(DOWNLOAD "https://github.com/microsoft/DirectXTK/releases/latest/download/MakeSpriteFont.exe" "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTK12Fonts/MakeSpriteFont.exe"
     STATUS DownloadStatus
     SHOW_PROGRESS)

list(GET DownloadStatus 0 StatusCode)
list(GET DownloadStatus 1 StatusString)
if(NOT StatusCode EQUAL 0)
    message(FATAL_ERROR "Download error: ${StatusString}")
endif()

add_subdirectory(ReflectionTool)
add_subdirectory(Engine)
