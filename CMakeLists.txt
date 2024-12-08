cmake_minimum_required(VERSION 3.12)
project(WebRenderer)
set(CMAKE_CXX_STANDARD 17)

#find_package(fmt CONFIG REQUIRED)
# Build hello_imgui
# =================
# 1/  Option 1: if you added hello_imgui as a subfolder, you can add it to your project with:
#if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/hello_imgui)
#    add_subdirectory(external/hello_imgui)
#endif()

# 2/  Option 2: simply fetch hello_imgui during the build
#if (NOT TARGET hello_imgui)
#    message(STATUS "Fetching hello_imgui")
#   include(FetchContent)
#    FetchContent_Declare(hello_imgui GIT_REPOSITORY https://github.com/pthom/hello_imgui.git GIT_TAG master)
#    FetchContent_MakeAvailable(hello_imgui)
#endif()

# 3/  Option 3: via vcpkg
# i/ You can install hello_imgui via vcpkg with:
#     vcpkg install "hello-imgui[opengl3-binding,glfw-binding]"
# ii/ Then you can use it inside CMake with:
#     find_package(hello-imgui CONFIG REQUIRED)

# ============================================================================
# Add dependencies via CPM (cmake/tools.cmake includes cmake/CPM.cmake)
#
# see https://github.com/TheLartians/CPM.cmake for more info
# ============================================================================
include(cmake/CPM.cmake)

# set(HELLOIMGUI_USE_GLFW_METAL ON)
# set(HELLOIMGUI_USE_GLFW_OPENGL3 ON)
# set(HELLOIMGUI_USE_SDL_OPENGL3 ON) - deprecated
set(HELLOIMGUI_USE_SDL2 ON)
set(HELLOIMGUI_HAS_OPENGL3 ON)
CPMAddPackage("gh:pthom/hello_imgui#v1.6.0")
if(hello_imgui_ADDED)
  message(STATUS "hello_imgui library added")
endif()

CPMAddPackage("gh:sgorsten/linalg@2.2")
if(linalg_ADDED)
  message(STATUS "linalg library added")
  add_library(linalg INTERFACE IMPORTED)
  target_include_directories(linalg INTERFACE "${linalg_SOURCE_DIR}")
endif()

CPMAddPackage("gh:fmtlib/fmt#10.1.1")
if(fmt_ADDED)
  message(STATUS "fmt library added")
endif()

# Build your app
# ==============
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

hello_imgui_add_app(
    WebRenderer
    src/app.cpp
    src/renderpass.cpp
    src/shader.cpp
    src/webrenderer.cpp
)

target_link_libraries(WebRenderer PRIVATE linalg fmt::fmt)

# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
# Usage:
# hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
#
# Features: 
# * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
# * It will embed the assets (for desktop, mobile, and emscripten apps)
# * It will perform additional customization (app icon and name on mobile platforms, etc)

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .
