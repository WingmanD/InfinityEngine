file(GLOB_RECURSE _shaders "*.hlsl")

add_custom_target(Shaders SOURCES ${_shaders})

set_source_files_properties(${_shaders} PROPERTIES HEADER_FILE_ONLY TRUE)
