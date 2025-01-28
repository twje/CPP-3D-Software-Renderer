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
FetchContent_MakeAvailable(SDL)

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

# Enable clang-tidy static analysis
set(CLANG_TIDY_COMMAND "clang-tidy")

# Apply clang-tidy to all targets
set_target_properties(3DRenderer PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND};-checks=*,-clang-analyzer-alpha.*"
)