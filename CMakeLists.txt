cmake_minimum_required(VERSION 3.12)
project(InfinityEngine)

if (WIN32)
    set(DirectXTK12_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTK12")
    include(FetchContent)
    
    FetchContent_Declare(
        DirectXTK12
        GIT_REPOSITORY https://github.com/microsoft/DirectXTK12.git
        GIT_TAG        main
        SOURCE_DIR     DirectXTK12_DIR
    )
    
    FetchContent_MakeAvailable(DirectXTK12)

    file(GLOB DirectXTK12_CONTENT "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTK12Fonts")
    if(NOT DirectXTK12_CONTENT)
        
        file(DOWNLOAD "https://github.com/microsoft/DirectXTK/releases/latest/download/MakeSpriteFont.exe" "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTK12Fonts/MakeSpriteFont.exe"
            STATUS DownloadStatus
            SHOW_PROGRESS)
        
        list(GET DownloadStatus 0 StatusCode)
        list(GET DownloadStatus 1 StatusString)
        if(NOT StatusCode EQUAL 0)
            message(FATAL_ERROR "Failed to download package DirectXTK12 : ${StatusString}")
        endif()
    endif()
    
    set(AGILITYSDK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/AgilitySDK")
    
    file(GLOB AGILITYSDK_CONTENT "${AGILITYSDK_SOURCE_DIR}/*")
    if(NOT AGILITYSDK_CONTENT)
        set(AGILITY_URI "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.613.2")
        set(AGILITY_ZIP "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Agility.zip")

        execute_process(
            COMMAND powershell -Command "Invoke-WebRequest -Uri ${AGILITY_URI} -OutFile ${AGILITY_ZIP}"
            RESULT_VARIABLE DownloadStatus
        )
        if(NOT DownloadStatus STREQUAL "0")
        message(FATAL_ERROR "Failed to download package AgilitySDK")
        endif()
        
        execute_process(
            COMMAND powershell -Command "Expand-Archive -Path ${AGILITY_ZIP} -DestinationPath ${AGILITYSDK_SOURCE_DIR} -Force"
            RESULT_VARIABLE ExtractStatus
        )
        if(NOT ExtractStatus STREQUAL "0")
            message(FATAL_ERROR "Failed to extract package AgilitySDK")
        endif()
        
        file(REMOVE "${AGILITY_ZIP}")
    endif()
  
    set(D3DCOMPILER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/D3DCompiler")
    
    file(GLOB D3DCOMPILER_CONTENT "${D3DCOMPILER_SOURCE_DIR}/*")
    if(NOT D3DCOMPILER_CONTENT)
        set(D3DCOMPILER_URI "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2403.1/dxc_2024_03_22.zip")
        set(D3DCOMPILER_ZIP "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/D3DCompiler.zip")

        execute_process(
            COMMAND powershell -Command "Invoke-WebRequest -Uri ${D3DCOMPILER_URI} -OutFile ${D3DCOMPILER_ZIP}"
            RESULT_VARIABLE DownloadStatus
        )
        if(NOT DownloadStatus STREQUAL "0")
            message(FATAL_ERROR "Failed to download package D3DCompiler")
        endif()
        
        execute_process(
            COMMAND powershell -Command "Expand-Archive -Path ${D3DCOMPILER_ZIP} -DestinationPath ${D3DCOMPILER_SOURCE_DIR} -Force"
            RESULT_VARIABLE ExtractStatus
        )
        if(NOT ExtractStatus STREQUAL "0")
            message(FATAL_ERROR "Failed to extract package D3DCompiler")
        endif()
        
        file(REMOVE "${D3DCOMPILER_ZIP}")
    endif()
    
    set(D3DCOMPILERPDB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/D3DCompiler/PDB")

    file(GLOB D3DCOMPILERPDB_CONTENT "${D3DCOMPILERPDB_SOURCE_DIR}/*")
    if(NOT D3DCOMPILERPDB_CONTENT)
        set(D3DCOMPILERPDB_URI "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2403.1/pdb_2024_03_22.zip")
        set(D3DCOMPILERPDB_ZIP "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/D3DCompilerPDB.zip")

        execute_process(
            COMMAND powershell -Command "Invoke-WebRequest -Uri ${D3DCOMPILERPDB_URI} -OutFile ${D3DCOMPILERPDB_ZIP}"
            RESULT_VARIABLE DownloadStatus
        )
        if(NOT DownloadStatus STREQUAL "0")
            message(FATAL_ERROR "Failed to download package D3DCompilerPDB")
        endif()
        
        execute_process(
            COMMAND powershell -Command "Expand-Archive -Path ${D3DCOMPILERPDB_ZIP} -DestinationPath ${D3DCOMPILERPDB_SOURCE_DIR} -Force"
            RESULT_VARIABLE ExtractStatus
        )
        if(NOT ExtractStatus STREQUAL "0")
            message(FATAL_ERROR "Failed to extract package D3DCompilerPDB")
        endif()
        
        file(REMOVE "${D3DCOMPILERPDB_ZIP}")
    endif()
endif()

add_subdirectory(ReflectionTool)
add_subdirectory(Engine)
