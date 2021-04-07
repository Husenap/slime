message("-- External Project: dubu-pack")
include(FetchContent)

FetchContent_Declare(
    dubu_pack
    GIT_REPOSITORY  https://github.com/Husenap/dubu-pack.git
    GIT_TAG         v1.1
)

set(dubu_pack_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(dubu_pack_FOLDER "thirdparty/dubu_pack" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(dubu_pack)