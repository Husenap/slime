set(target_name "slime")

set(src_precompiled
    "src/slime/precompiled.hpp")

set(src_slime
    "src/slime/Shader.hpp"
    "src/slime/ShaderProgram.hpp"
    "src/slime/Slime.cpp"
    "src/slime/Slime.hpp"
    "src/main.cpp")

set(src_files
    ${src_precompiled}
    ${src_slime})

# Project
add_executable(${target_name} ${src_files})

target_link_libraries(${target_name}
    compiler_features
    compiler_warnings
    dubu_pack
    dubu_opengl_app)

target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${target_name} PUBLIC ${src_precompiled})

source_group("src" FILES ${src_slime})
source_group("src/precompiled" FILES ${src_precompiled})
source_group("src/precompiled" REGULAR_EXPRESSION "cmake_pch\.[ch]xx")

set_target_properties(${target_name} PROPERTIES
    FOLDER ${${PROJECT_NAME}_FOLDER}
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${target_name}>)

dubu_pack_package("${CMAKE_CURRENT_SOURCE_DIR}/assets")

add_custom_command(
    TARGET ${target_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/assets.dbp
    $<TARGET_FILE_DIR:${target_name}>/assets.dbp)