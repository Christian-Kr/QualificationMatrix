# ------------------------------------------------------------------------------
# General

# Minimum version needd to work
cmake_minimum_required(VERSION 3.13.0)

# Name of the project
project(QualificationMatrix)

# Add module path
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

# Include the subdirectory
include_directories(${QualificationMatrix_SOURCE_DIR}/src)

# If version of cmake is too old, current dir must be explicitly included.
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

# General CPP macros
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

# ------------------------------------------------------------------------------
# Standard macros for Qt

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

find_package(Qt6 6.3
        COMPONENTS LinguistTools Widgets Sql PrintSupport
        REQUIRED)
qt_standard_project_setup()

# Add application icon for windows.
set(APP_ICON_RESOURCE_WINDOWS "rcs/qm_res.rc")

# ------------------------------------------------------------------------------
# Botan

find_package(Botan REQUIRED)
include_directories(${BOTAN_INCLUDE_DIRS} ${BOTAN_INCLUDE_DIRS}/botan-2)

# ------------------------------------------------------------------------------
# Translation

set(TS_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/trans/qm_de.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/trans/qm_en.ts)
qt6_add_translation(QM_FILES
        ${TS_FILES})

# ------------------------------------------------------------------------------
# Create executable and link libraries

# Get all source, header and ui files in src and ui.
file(GLOB_RECURSE SRCS src/*.h src/*.cpp)
file(GLOB_RECURSE UIS ui/*.ui)

if (APPLE)
    add_executable(QualificationMatrix
            MACOSX_BUNDLE rcs/icons.qrc ${SRCS} ${UIS} ${APP_ICON_RESOURCE_WINDOWS})
else ()
    add_executable(QualificationMatrix
            WIN32 rcs/icons.qrc ${SRCS} ${UIS} ${APP_ICON_RESOURCE_WINDOWS})
endif ()
target_link_libraries(QualificationMatrix
        Qt6::Widgets Qt6::Sql Qt6::PrintSupport
        ${BOTAN_LIBRARIES})

# ------------------------------------------------------------------------------
# Deploy

# Copy the database directory.
add_custom_command(TARGET QualificationMatrix
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/database
        $<TARGET_FILE_DIR:QualificationMatrix>/database)

# Copy the styles directory.
add_custom_command(TARGET QualificationMatrix
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/styles
        $<TARGET_FILE_DIR:QualificationMatrix>/styles)

# Copy translation files.
target_sources(QualificationMatrix
        PRIVATE ${QM_FILES})

install(TARGETS QualificationMatrix
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

qt_generate_deploy_app_script(
        TARGET QualificationMatrix
        OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
