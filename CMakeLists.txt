# This CMake file is based on Polytonic/Glitter

cmake_minimum_required(VERSION 3.0)
project(GraphicsProgramming)

# Load library "GLFW" used for window creation
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(vendor/glfw)

# Load library "Assimp" literally "asset importer"
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(common/include/
                    vendor/assimp/include/
                    vendor/bullet/src/
                    vendor/glad/include/
                    vendor/glfw/include/
                    vendor/glm/
                    vendor/stb/
                    vendor/imgui/
                    vendor/spdlog/include/)

# There are some libraries which we have to compile every time
file(GLOB IMGUI_SOURCES vendor/imgui/*.cpp
                        vendor/imgui/backends/imgui_impl_opengl3.cpp
                        vendor/imgui/backends/imgui_impl_glfw.cpp)
file(GLOB GLAD_SOURCES vendor/glad/src/glad.c)
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

source_group("Common Headers" FILES ${COMMON_HEADERS})
source_group("Common Shaders" FILES ${COMMON_SHADERS})
source_group("Common Sources" FILES ${COMMON_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE -DCOMMON_ASSETS_DIR=\"${CMAKE_CURRENT_LIST_DIR}/common/assets/\")

# Add each app as an executable (properly not good practice)
message("Adding apps")
file(GLOB children LIST_DIRECTORIES true apps/*)
FOREACH(child ${children})
  IF(IS_DIRECTORY ${child})
      cmake_path(GET child FILENAME project)
      message("Adding executable for ${project} at ${child}")

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

      message("Child: ${child}")
      target_compile_definitions(${project} PUBLIC -DPROJECT_SOURCE_DIR=\"${child}/\")
      target_include_directories(${project} PUBLIC ${child}/include/)
      target_link_libraries(${project} assimp glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES}
                      BulletDynamics BulletCollision LinearMath spdlog)
      set_target_properties(${project} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${project})

      add_custom_command(
          TARGET ${project} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory ${child}/shaders $<TARGET_FILE_DIR:${project}>
          DEPENDS ${COMMON_SHADERS} ${PROJECT_SHADERS})
  ENDIF()
ENDFOREACH()
