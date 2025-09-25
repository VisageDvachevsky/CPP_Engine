cmake_minimum_required(VERSION 3.20)
project(MiniGPU)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig)

# Try to find GLFW
find_package(glfw3)
if(NOT glfw3_FOUND)
    # Try pkg-config as fallback
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 glfw3)
    endif()
endif()

# Include directories - adjust these paths according to your structure
include_directories(src)

# Add third_party directories if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/glad/include")
    include_directories(third_party/glad/include)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui")
    include_directories(third_party/imgui)
    include_directories(third_party/imgui/backends)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/ImGuizmo")
    include_directories(third_party/ImGuizmo)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/stb")
    include_directories(third_party/stb)
endif()

# GLAD sources (only if the file exists)
set(GLAD_SOURCES "")
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/glad/src/glad.c")
    set(GLAD_SOURCES third_party/glad/src/glad.c)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/third_party/glad/glad.c")
    set(GLAD_SOURCES third_party/glad/glad.c)
endif()

# ImGui sources (only if they exist)
set(IMGUI_SOURCES "")
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp")
    set(IMGUI_SOURCES
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_demo.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
    )
    
    # Add backends if they exist
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp")
        list(APPEND IMGUI_SOURCES third_party/imgui/backends/imgui_impl_glfw.cpp)
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_opengl3.cpp")
        list(APPEND IMGUI_SOURCES third_party/imgui/backends/imgui_impl_opengl3.cpp)
    endif()
endif()

# ImGuizmo sources (only if it exists)
set(IMGUIZMO_SOURCES "")
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/ImGuizmo/ImGuizmo.cpp")
    set(IMGUIZMO_SOURCES third_party/ImGuizmo/ImGuizmo.cpp)
endif()

# Engine sources - only include files that exist
set(ENGINE_SOURCES src/main.cpp)

# Core sources
file(GLOB CORE_SOURCES "src/core/*.cpp")
list(APPEND ENGINE_SOURCES ${CORE_SOURCES})

# Math sources
file(GLOB MATH_SOURCES "src/math/*.cpp")
list(APPEND ENGINE_SOURCES ${MATH_SOURCES})

# Renderer sources
file(GLOB RENDERER_SOURCES "src/renderer/*.cpp")
list(APPEND ENGINE_SOURCES ${RENDERER_SOURCES})

# Scene sources
file(GLOB SCENE_SOURCES "src/scene/*.cpp")
list(APPEND ENGINE_SOURCES ${SCENE_SOURCES})

# Editor sources
file(GLOB EDITOR_SOURCES "src/editor/*.cpp")
list(APPEND ENGINE_SOURCES ${EDITOR_SOURCES})

# GUI sources
file(GLOB GUI_SOURCES "src/gui/*.cpp")
list(APPEND ENGINE_SOURCES ${GUI_SOURCES})

# Utils sources
file(GLOB UTILS_SOURCES "src/utils/*.cpp")
list(APPEND ENGINE_SOURCES ${UTILS_SOURCES})

# Create executable
add_executable(${PROJECT_NAME}
    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUIZMO_SOURCES}
    ${GLAD_SOURCES}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

# Link GLFW
if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
elseif(GLFW3_FOUND)
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
else()
    message(WARNING "GLFW not found! You may need to install it or specify the path manually.")
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        opengl32
        gdi32
        user32
        kernel32
        shell32
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}
        GL
        X11
        pthread
        Xrandr
        Xi
        dl
    )
endif()

# Copy shaders to build directory
if(EXISTS "${CMAKE_SOURCE_DIR}/shaders")
    file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Print information about what was found
message(STATUS "Engine sources: ${ENGINE_SOURCES}")
if(IMGUI_SOURCES)
    message(STATUS "ImGui found and will be compiled")
else()
    message(STATUS "ImGui not found in third_party/imgui")
endif()

if(GLAD_SOURCES)
    message(STATUS "GLAD found: ${GLAD_SOURCES}")
else()
    message(STATUS "GLAD not found in third_party/glad")
endif()

if(glfw3_FOUND OR GLFW3_FOUND)
    message(STATUS "GLFW found and linked")
else()
    message(STATUS "GLFW not found - you may need to install it")
endif()