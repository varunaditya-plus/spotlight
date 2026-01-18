cmake_minimum_required(VERSION 3.21)
project(spotlight VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Homebrew Qt6
if(EXISTS "/home/linuxbrew/.linuxbrew/Cellar/qtbase")
    set(CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew/Cellar/qtbase/6.10.1" ${CMAKE_PREFIX_PATH})
endif()

# OpenGL for Qt6
set(OPENGL_opengl_LIBRARY "/usr/lib64/libGL.so.1")
set(OPENGL_glx_LIBRARY "/usr/lib64/libGLX.so")
set(OPENGL_INCLUDE_DIR "/usr/include")
if(NOT TARGET OpenGL::GL)
    add_library(OpenGL::GL SHARED IMPORTED)
    set_target_properties(OpenGL::GL PROPERTIES
        IMPORTED_LOCATION "${OPENGL_opengl_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

add_executable(spotlight 
  src/main.cpp 
  src/Spotlight.cpp
  src/actions/actions.cpp
  src/actions/search.cpp
  src/searches/searches.cpp
  src/searches/apps.cpp
  src/searches/settings.cpp
  src/spotlightapps/utils.cpp
  src/spotlightapps/demo/demoapp.cpp
)
target_link_libraries(spotlight PRIVATE Qt6::Widgets)

# Copy Qt platform plugins
if(DEFINED Qt6_DIR)
    get_filename_component(QT6_PREFIX "${Qt6_DIR}/../../.." ABSOLUTE)
    add_custom_command(TARGET spotlight POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:spotlight>/platforms"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${QT6_PREFIX}/share/qt/plugins/platforms" "$<TARGET_FILE_DIR:spotlight>/platforms"
        COMMENT "Copying Qt platform plugins")
endif()
