cmake_minimum_required(VERSION 3.21)
project(AGP_Deferred_PBR_SSAO LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------
# Dependencies (via vcpkg)
# --------------------------------------------------------------------
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)

# --------------------------------------------------------------------
# Source files (your own code)
# --------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        Engine/Code/*.cpp
        Engine/Code/*.h
)

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

# --------------------------------------------------------------------
# Include directories (for your engine code)
# --------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Code
        ${Stb_INCLUDE_DIR}
)

# --------------------------------------------------------------------
# Link libraries
# --------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glad::glad
        glm::glm
        imgui::imgui
        assimp::assimp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

# Optional: specify output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
