message("-- External Project: dubu-window")
include(FetchContent)

FetchContent_Declare(
    dubu_window
    GIT_REPOSITORY  https://github.com/Husenap/dubu-window.git
    GIT_TAG         v1.2
)

set(dubu_window_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(dubu_window_FOLDER "thirdparty/dubu_window" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(dubu_window)