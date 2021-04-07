add_library(glad "include/glad/gl.h" "src/gl.c")
target_include_directories(glad PUBLIC "include/")
set_target_properties(glad PROPERTIES FOLDER "thirdparty/glad")