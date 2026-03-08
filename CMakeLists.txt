# ------------------ Project Wide Settings ------------------
cmake_minimum_required (VERSION 3.10)
project(Social-evolution LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------ Variables ------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(SOURCE_DIR src)

find_package(OpenMP REQUIRED)

# ------------------ Dependencies ------------------
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2

)
FetchContent_MakeAvailable(SFML)
FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.1
    
)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)
set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml
    GIT_TAG v3.0
    
)
FetchContent_MakeAvailable(ImGui-SFML)

project ("Social-evolution")

# ------------------ Executable ------------------
add_executable (Social-evolution)

target_sources(Social-evolution
    PRIVATE
        ${SOURCE_DIR}/main.cpp
        ${SOURCE_DIR}/Agent.cpp
        ${SOURCE_DIR}/Grid.cpp
        ${SOURCE_DIR}/GuiPanel.cpp
        ${SOURCE_DIR}/LeftPanel.cpp
        ${SOURCE_DIR}/Simulation.cpp
        ${SOURCE_DIR}/SimulationApp.cpp
        ${SOURCE_DIR}/SimulationRenderer.cpp
)

target_include_directories(Social-evolution
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Social-evolution
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        ImGui-SFML
        OpenMP::OpenMP_CXX
)
