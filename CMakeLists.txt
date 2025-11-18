cmake_minimum_required(VERSION 3.10)
project(RaylibBoids)

set(CMAKE_CXX_STANDARD 20)
# weird stuff for clang to behave
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER clang++)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Find Raylib (Assuming it's installed on the system)
find_package(raylib REQUIRED)
# Add your source files
file(GLOB SOURCES "src/*.cpp")
add_executable(main ${SOURCES})

# --- Raygui via pkg-config ---
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(RAYGUI REQUIRED raygui)

# target_include_directories(main PRIVATE ${RAYGUI_INCLUDE_DIRS})
# target_link_libraries(main PRIVATE ${RAYGUI_LIBRARIES})

# Link Raylib to your project
target_link_libraries(main raylib)

# If Raylib is not installed system-wide, specify the path
