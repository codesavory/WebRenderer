cmake_minimum_required(VERSION 3.12)
project(WebRenderer)
set(CMAKE_CXX_STANDARD 17)

find_package(fmt CONFIG REQUIRED)
# Build hello_imgui
# =================
find_package(hello-imgui CONFIG REQUIRED)

# Build your app
# ==============
hello_imgui_add_app(webRenderer src/app.cpp)

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
