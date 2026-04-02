cmake_minimum_required(VERSION 3.16)
project(HairSimu LANGUAGES CXX)

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HAIRSIMU_BUILD_VIEWER "Build OpenGL interactive viewer" ON)

add_executable(HairSimu
    src/main.cpp
    src/HairSimulation.cpp
    src/HairModelLoader.cpp
    src/ShadingModels.cpp
    src/Experiment.cpp
    src/Viewer.cpp
)

target_include_directories(HairSimu PRIVATE include)

if(HAIRSIMU_BUILD_VIEWER)
    find_package(OpenGL QUIET)
    find_package(glfw3 QUIET)

    if(OpenGL_FOUND AND (TARGET glfw OR TARGET glfw3::glfw))
        target_compile_definitions(HairSimu PRIVATE HAIRSIMU_ENABLE_VIEWER=1)
        if(TARGET glfw)
            target_link_libraries(HairSimu PRIVATE OpenGL::GL glfw)
        else()
            target_link_libraries(HairSimu PRIVATE OpenGL::GL glfw3::glfw)
        endif()
        message(STATUS "HairSimu viewer enabled")
    else()
        message(WARNING "OpenGL/GLFW not found; viewer will be built as stub mode")
    endif()
endif()

if(MSVC)
    target_compile_options(HairSimu PRIVATE /W4)
else()
    target_compile_options(HairSimu PRIVATE -Wall -Wextra -Wpedantic)
endif()
