cmake_minimum_required(VERSION 3.20)
project(MiniGPU)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(PkgConfig)

find_package(glfw3)
if(NOT glfw3_FOUND)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 glfw3)
    endif()
endif()

include_directories(src)

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

set(GLAD_SOURCES "")
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/glad/src/glad.c")
    set(GLAD_SOURCES third_party/glad/src/glad.c)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/third_party/glad/glad.c")
    set(GLAD_SOURCES third_party/glad/glad.c)
endif()

set(IMGUI_SOURCES "")
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp")
    set(IMGUI_SOURCES
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_demo.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
    )
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp")
        list(APPEND IMGUI_SOURCES third_party/imgui/backends/imgui_impl_glfw.cpp)
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_opengl3.cpp")
        list(APPEND IMGUI_SOURCES third_party/imgui/backends/imgui_impl_opengl3.cpp)
    endif()
endif()

set(IMGUIZMO_SOURCES "")
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/ImGuizmo/ImGuizmo.cpp")
    set(IMGUIZMO_SOURCES third_party/ImGuizmo/ImGuizmo.cpp)
endif()

set(ENGINE_SOURCES src/main.cpp)

file(GLOB CORE_SOURCES "src/core/*.cpp")
list(APPEND ENGINE_SOURCES ${CORE_SOURCES})

file(GLOB MATH_SOURCES "src/math/*.cpp")
list(APPEND ENGINE_SOURCES ${MATH_SOURCES})

file(GLOB RENDERER_SOURCES "src/renderer/*.cpp")
list(APPEND ENGINE_SOURCES ${RENDERER_SOURCES})

file(GLOB SCENE_SOURCES "src/scene/*.cpp")
list(APPEND ENGINE_SOURCES ${SCENE_SOURCES})

file(GLOB EDITOR_SOURCES "src/editor/*.cpp")
list(APPEND ENGINE_SOURCES ${EDITOR_SOURCES})

file(GLOB GUI_SOURCES "src/gui/*.cpp")
list(APPEND ENGINE_SOURCES ${GUI_SOURCES})

file(GLOB UTILS_SOURCES "src/utils/*.cpp")
list(APPEND ENGINE_SOURCES ${UTILS_SOURCES})

add_executable(${PROJECT_NAME}
    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUIZMO_SOURCES}
    ${GLAD_SOURCES}
)

target_link_libraries(${PROJECT_NAME} OpenGL::GL)

if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
elseif(GLFW3_FOUND)
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
else()
    message(WARNING "GLFW not found! You may need to install it or specify the path manually.")
endif()

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

if(EXISTS "${CMAKE_SOURCE_DIR}/shaders")
    file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
endif()

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