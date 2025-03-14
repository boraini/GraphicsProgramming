# This CMake file is based on Polytonic/Glitter

cmake_minimum_required(VERSION 3.0)
project(GraphicsProgramming)

# "Pure" C++17 and no extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default debug and optimize flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(WEBSITE_ROOT ${CMAKE_BINARY_DIR}/website)
# Output directories
if(EMSCRIPTEN)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/emscripten/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/emscripten/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${WEBSITE_ROOT})
else()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/native/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/native/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# For library compilation, add C source version in case the compiler doesn't like the stdlib headers
if(EMSCRIPTEN)
add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()

# Load library "GLFW" used for window creation
if(NOT EMSCRIPTEN)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_X11 OFF)
option(GLFW_BUILD_WAYLAND OFF)
add_subdirectory(vendor/glfw)
endif()

# Load library "Assimp" literally "asset importer"
# It needs to be prebuilt if using Emscripten. Check the README file for instructions
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(vendor/assimp)

# Load library "Bullet" used for physics simulation
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(vendor/bullet)

# Load library "spdlog" used for logging
add_subdirectory(vendor/spdlog)

# Enable a lot of warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17 -fcolor-diagnostics")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

# Emscripten requires that we use their ports for some libraries
if(EMSCRIPTEN)
    set(EMSCRIPTEN_COMPILER_FLAGS "-sUSE_ZLIB=1 -sUSE_WEBGL2=1 -sUSE_GLFW=3 -sUSE_BULLET=1 -sFETCH=1 -sINITIAL_MEMORY=134217728 -sALLOW_MEMORY_GROWTH=1 -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_COMPILER_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_COMPILER_FLAGS}")

    # the linker flag -sMAX_WEBGL_VERSION=2 is used because due to Emscripten internals,
    # the GLFW_CONTEXT_VERSION_MAJOR is ignored. When using the port, the flag is
    # automatically set so there is no need to set it.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -sNO_DISABLE_EXCEPTION_CATCHING -s ASSERTIONS=1 -s WASM=1 -s SAFE_HEAP=1")
endif()

# Include headers
include_directories(common/include/
                    vendor/assimp/include/
                    vendor/bullet/src/
                    vendor/glm/
                    vendor/stb/
                    vendor/imgui/
                    vendor/spdlog/include/)

# There are some libraries which we have to compile every time. For Emscripten we use the JS-native versions.
if(EMSCRIPTEN)
# include_directories($ENV{EMSDK}/upstream/emscripten/cache/sysroot/include)
file(GLOB GLAD_SOURCES)
else()
include_directories(vendor/glad/include/)
include_directories(vendor/glfw/include/)
file(GLOB GLAD_SOURCES vendor/glad/src/glad.c)
endif()

file(GLOB IMGUI_SOURCES vendor/imgui/*.cpp
                        vendor/imgui/backends/imgui_impl_opengl3.cpp
                        vendor/imgui/backends/imgui_impl_glfw.cpp)

set(VENDORS_SOURCES ${GLAD_SOURCES} ${IMGUI_SOURCES})

file(GLOB COMMON_HEADERS  common/include/*.hpp)
file(GLOB COMMON_SOURCES  common/src/*.cpp)
file(GLOB COMMON_SHADERS  common/shaders/*.comp
                          common/shaders/*.frag
                          common/shaders/*.geom
                          common/shaders/*.vert)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                         .gitattributes
                         .gitignore
                         .gitmodules)
file(GLOB COMMON_ASSETS common/assets/**/*)

source_group("Common Headers" FILES ${COMMON_HEADERS})
source_group("Common Shaders" FILES ${COMMON_SHADERS})
source_group("Common Sources" FILES ${COMMON_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

if(EMSCRIPTEN)
# fetch.hpp will load from the website root
add_definitions(-DGLFW_INCLUDE_NONE -DCOMMON_ASSETS_DIR=\"/assets/\")
else()
# fetch.hpp will load from the assets directory
add_definitions(-DGLFW_INCLUDE_NONE -DCOMMON_ASSETS_DIR=\"${CMAKE_CURRENT_LIST_DIR}/common/assets/\")
endif()

if(EMSCRIPTEN)
add_custom_target(website_assets)
add_custom_command(
          TARGET website_assets POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/common/assets ${WEBSITE_ROOT}/assets
          DEPENDS ${COMMON_ASSETS})
endif()

# Add each app as an executable (probably not good practice)
message("Adding apps")
file(GLOB children LIST_DIRECTORIES true apps/*)
FOREACH(child ${children})
  IF(IS_DIRECTORY ${child})
      cmake_path(GET child FILENAME project)
      message("Adding executable for ${project} at ${child}")
      set(outchild ${CMAKE_BINARY_DIR}/${project})

      file(GLOB PROJECT_HEADERS ${child}/include/*.hpp)
      file(GLOB PROJECT_SOURCES ${child}/src/*.cpp)
      file(GLOB PROJECT_SHADERS
                          ${child}/shaders/*.comp
                          ${child}/shaders/*.frag
                          ${child}/shaders/*.geom
                          ${child}/shaders/*.vert)

      source_group("${project} Headers" FILES ${PROJECT_HEADERS})
      source_group("${project} Sources" FILES ${PROJECT_SOURCES})
      source_group("${project} Shaders" FILES ${PROJECT_SHADERS})

      add_executable(${project} ${COMMON_SOURCES} ${COMMON_HEADERS} ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_SHADERS} ${VENDORS_SOURCES})

      target_compile_definitions(${project} PUBLIC -DPROJECT_SOURCE_DIR=\"${child}/\")
      if(EMSCRIPTEN)
      target_compile_options(${project} PUBLIC --preload-file ${child}/shaders --preload-file ${CMAKE_CURRENT_LIST_DIR}/common/assets)
      set_target_properties(${project} PROPERTIES OUTPUT_NAME "emscripten-generated")
      endif()

      target_include_directories(${project} PUBLIC ${child}/include/ ${outchild}/include/)

      target_link_libraries(${project} assimp spdlog)
      if(NOT EMSCRIPTEN)
      message("Linking glfw, glad, and bullet")
      target_link_libraries(${project} glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} BulletDynamics BulletCollision LinearMath)
      endif()
      
      set_target_properties(${project} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${project})

      # Copy shaders to the output
      add_custom_command(
          TARGET ${project} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory ${child}/shaders $<TARGET_FILE_DIR:${project}>
          DEPENDS ${COMMON_SHADERS} ${PROJECT_SHADERS})
      
      # Copy assets to the website root
      if(EMSCRIPTEN)
      
      endif()

      # Produce the shaders.hpp file
      add_custom_target(${project}-prebuild DEPENDS ${PROJECT_SHADERS})
      add_custom_command(
          TARGET ${project}-prebuild PRE_BUILD
          COMMAND python scripts/project-pre-build.py ${child} ${outchild}
          WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
      add_dependencies(${project} ${project}-prebuild)
      
      if(EMSCRIPTEN)
          # Copy web files
          add_dependencies(${project} website_assets)

          add_custom_command(
              TARGET ${project} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/common/web $<TARGET_FILE_DIR:${project}>
              DEPENDS ${SHELL_SRC}
          )
      endif()
  ENDIF()
ENDFOREACH()
