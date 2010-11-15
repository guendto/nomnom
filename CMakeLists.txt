
cmake_minimum_required (VERSION 2.8.0)

project (nomnom)

set (CMAKE_VERBOSE_MAKEFILE false)

add_definitions (-Wall)
#add_definitions (-Werror)

# Version.

set (VERSION "0.1.0")
set (VERSION_LONG "${VERSION}")
set (BRANCH "master")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    find_program (GIT_PROG "git")
    if (GIT_PROG)
        execute_process (
            COMMAND ${GIT_PROG} "describe" "${BRANCH}"
            OUTPUT_VARIABLE DESCR
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (DESCR)
            set (VERSION_LONG "${DESCR}")
        endif ()
    endif ()
endif ()

find_program (DATE_PROG "date")
if (DATE_PROG)
    execute_process (
        COMMAND ${DATE_PROG} "+%Y-%m-%d"
        OUTPUT_VARIABLE DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (DATE)
        set (VERSION_LONG "${VERSION_LONG} built on ${DATE}")
    endif ()
endif ()

set (HOST_SETUP "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
set (VERSION_LONG "${VERSION_LONG} for ${HOST_SETUP}")

# Set default build type.

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "debug")
endif ()

# config.h

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

# nomnom.desktop

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/nomnom.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nomnom.desktop"
)

# Sources.

set (nomnom_src
    # UI.
    src/i/Preferences.cpp
    src/i/MainWindow.cpp
    src/i/Reminder.cpp
    src/i/LogView.cpp
    src/i/About.cpp
    # Other.
    src/Download.cpp
    src/History.cpp
    src/Video.cpp
    src/Log.cpp
    src/main.cpp
    src/util.cpp
    src/tips.cpp
)

set (nomnom_hdr
    src/History.h
    src/Log.h
    src/util.h
    src/tips.h
)

set (nomnom_moc_hdr
    src/i/Preferences.h
    src/i/MainWindow.h
    src/i/Reminder.h
    src/i/LogView.h
    src/Download.h
    src/i/About.h
    src/Video.h
)

set (nomnom_ui
    i/Preferences.ui
    i/MainWindow.ui
    i/Reminder.ui
    i/LogView.ui
    i/About.ui
)

set (nomnom_rc rc/nomnom.qrc)
file (GLOB nomnom_ts tr/*.ts)

# Prerequisites.

find_package (Qt4 4.5.0 COMPONENTS QtCore QtGui QtScript REQUIRED)
include (${QT_USE_FILE})

qt4_add_resources   (nomnom_rc_src  ${nomnom_rc})
qt4_add_translation (nomnom_qm      ${nomnom_ts})
qt4_wrap_ui         (nomnom_ui_hdr  ${nomnom_ui})
qt4_wrap_cpp        (nomnom_moc_src ${nomnom_moc_hdr})

# http://www.vtk.org/Wiki/CMake_2.8.0_Docs : qt4_create_translation

# The executable.

include_directories (
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i
)

add_executable (nomnom
    ${nomnom_src}
    ${nomnom_hdr}
    ${nomnom_moc_src}
    ${nomnom_rc_src}
    ${nomnom_ui_hdr}
    ${nomnom_qm}
)

# Status.

message (STATUS)
message (STATUS "nomnom version     : ${VERSION}")
message (STATUS "host setup         : ${HOST_SETUP}")
message (STATUS "install prefix     : ${CMAKE_INSTALL_PREFIX}")
message (STATUS "build type         : ${CMAKE_BUILD_TYPE}")
message (STATUS "  (debug, release, relwithdebinfo, minsizerel)")
message (STATUS "compiler           : ${CMAKE_CXX_COMPILER}")
message (STATUS "linker             : ${CMAKE_LINKER}")
message (STATUS "make program       : ${CMAKE_MAKE_PROGRAM}")
if (DATE_PROG)
    message (STATUS "date program       : ${DATE_PROG}")
else ()
    message (STATUS "date program       : (not found)")
endif ()
message (STATUS "verbose make       : ${CMAKE_VERBOSE_MAKEFILE}")
message (STATUS)

# Force these variables to be written to cache.

set (CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "Install destination for ${PROJECT_NAME}" FORCE)

set (CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Type of build: none, debug, release, relwithdebinfo minsizerel" FORCE)

set (CMAKE_VERBOSE_MAKEFILE "${CMAKE_VERBOSE_MAKEFILE}" CACHE BOOL
    "Build with verbose makefiles: true, false" FORCE)

# Link.

target_link_libraries (nomnom ${QT_LIBRARIES})

# CPack.

include (InstallRequiredSystemLibraries)

set (CPACK_SOURCE_PACKAGE_FILE_NAME
    "nomnom-${VERSION}"
    CACHE INTERNAL
    "tarball basename"
)

set (CPACK_PACKAGE_FILE_NAME ${CPACK_SOURCE_PACKAGE_FILE_NAME})
set (CPACK_GENERATOR "TGZ")
set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)
set (CPACK_RESOURCE_FILE_README  ${CMAKE_CURRENT_SOURCE_DIR}/README)

set (CPACK_SOURCE_IGNORE_FILES
    "/.git"
    "/.cmake/"
    "/CMakeFiles/"
    "/CMakeCache/"
    "/Makefile/"
    "/tmp/"
    "/OLD/"
    "/old/"
    "TODO"
    "/patches/"
)

# Notice the inclusion here, after setting the cpack variables.
include (CPack)

# Install.

install (PROGRAMS
    "${CMAKE_CURRENT_BINARY_DIR}/nomnom" DESTINATION bin/)

install (FILES ${nomnom_qm} DESTINATION share/nomnom/tr/)

install (FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nomnom.desktop" DESTINATION share/applications/)

install (FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/rc/img/nomnom.png" DESTINATION share/pixmaps/)

# Uninstall.

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target (
    uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
)


