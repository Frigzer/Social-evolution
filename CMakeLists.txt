# ------------------ Project Wide Settings ------------------
cmake_minimum_required (VERSION 3.8)
project(Social-evolution LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------ Variables ------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/external)
set(SOURCE_DIR src)

# Enable XAML hot reload for MSVC compilers where supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ------------------ Dependencies ------------------
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)
FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.1
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)
set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml
    GIT_TAG v3.0
    GIT_SHALLOW ON
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
)

if (MSVC)
    target_compile_options(Social-evolution PRIVATE /utf-8)
endif()
