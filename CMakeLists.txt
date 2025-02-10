cmake_minimum_required(VERSION 3.10)

project(GuidGenerator CXX)

include(FetchContent)

# Specify MSVC static runtime configuration
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# Fetch SDL2
FetchContent_Declare(
    SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.8
)
set(SDL_SHARED OFF)  # Disable shared (DLL) build
set(SDL_STATIC ON)   # Enable static build
FetchContent_MakeAvailable(SDL)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Collect all source and header files
file(GLOB_RECURSE ProjectSources "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE ProjectHeaders "${CMAKE_SOURCE_DIR}/src/*.h")

# Add executable
add_executable(3DRenderer     
    ${ProjectSources}
    ${ProjectHeaders} # Helps with IDE navigation
)

# Add precompiled header
target_precompile_headers(3DRenderer PRIVATE
    "${CMAKE_SOURCE_DIR}/src/pch.h"
)

# Set target properties
set_target_properties(3DRenderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(3DRenderer PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
    ${glm_SOURCE_DIR}  # Include GLM headers
    ${CMAKE_SOURCE_DIR}/vendor/stb
)

# Add compiler flags for MSVC
string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")  # Avoid conflict with /W4

target_compile_options(3DRenderer PRIVATE
    /W4                 # Enable high warning level
    /WX                 # Treat warnings as errors
    /permissive-        # Enforce standards conformance
    /MP                 # Enable multi-processor compilation
    /diagnostics:column # Enable detailed diagnostics
)

# Link libraries
target_link_libraries(3DRenderer PRIVATE 
    SDL2::SDL2 
    SDL2::SDL2main
)

# Configure definitions
target_compile_definitions(3DRenderer PUBLIC RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources/")
