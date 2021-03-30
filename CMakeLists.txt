# -------------------------------------------------------------------------------------------------
# General

# Minimum version needd to work
cmake_minimum_required(VERSION 3.13.0)

# Name of the project
project(QualificationMatrix)

# Include the subdirectory
include_directories(${QualificationMatrix_SOURCE_DIR}/src)

# If version of cmake is too old, current dir must be explicitly included.
if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

# General CPP macros
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------------------------------------
# Standard macros for Qt

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

find_package(Qt5 COMPONENTS LinguistTools Widgets Sql PrintSupport REQUIRED)

# Add application icon for windows.
set(APP_ICON_RESOURCE_WINDOWS "qm_res.rc")

# -------------------------------------------------------------------------------------------------
# Translation

set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/qm_de.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/qm_en.ts)
file(GLOB_RECURSE TS_SOURCES "*.cpp" "*.h" "*.ui")

# Add a custom translation target. ts files will only be updated if explicitly wanted.
add_custom_target(translations)

foreach (_ts_file ${TS_FILES})
    get_filename_component(_ts_name ${_ts_file} NAME_WE)
    add_custom_command(
        OUTPUT "${_ts_file}"
        DEPENDS ${TS_SOURCES}
        COMMAND ${Qt5_LUPDATE_EXECUTABLE} -recursive
        ${CMAKE_CURRENT_SOURCE_DIR} -ts ${_ts_file})

    # create a target that depends on the generated files
    add_custom_target(translate_${_ts_name} DEPENDS ${_ts_file})

    # attach the custom target for this ts certificate to the parent target
    add_dependencies(translations translate_${_ts_name})
endforeach ()

qt5_add_translation(QM_FILES ${TS_FILES})

# -------------------------------------------------------------------------------------------------
# Create executable and link libraries

add_executable(QualificationMatrix WIN32
    icons.qrc
    src/model/qmdatamanager.cpp
    src/model/qmdatamanager.h
    src/framework/qmdatedelegate.cpp
    src/framework/qmdatedelegate.h
    src/main.cpp
    src/framework/qmdialog.cpp
    src/framework/qmdialog.h
    src/general/qmmainwindow.cpp
    src/general/qmmainwindow.h
    src/framework/qmproxysqlrelationaldelegate.cpp
    src/framework/qmproxysqlrelationaldelegate.h
    src/qualimatrix/qmqualimatrixdelegate.cpp
    src/qualimatrix/qmqualimatrixdelegate.h
    src/qualimatrix/qmqualimatrixheaderview.cpp
    src/qualimatrix/qmqualimatrixheaderview.h
    src/qualimatrix/qmqualimatrixmodel.cpp
    src/qualimatrix/qmqualimatrixmodel.h
    src/qualimatrix/qmtableview.cpp
    src/qualimatrix/qmtableview.h
    src/qualiresult/qmqualiresultmodel.cpp
    src/qualiresult/qmqualiresultmodel.h
    src/qualiresult/qmqualiresultrecord.cpp
    src/qualiresult/qmqualiresultrecord.h
    src/settings/qmsettingsdialog.cpp
    src/settings/qmsettingsdialog.h
    src/settings/qmfuncsettingswidget.cpp
    src/settings/qmfuncsettingswidget.h
    src/settings/qmemployeesettingswidget.cpp
    src/settings/qmemployeesettingswidget.h
    src/settings/qmsettingswidget.cpp
    src/settings/qmsettingswidget.h
    src/settings/qmsettings.cpp
    src/settings/qmsettings.h
    src/settings/qmapplicationsettings.cpp
    src/settings/qmapplicationsettings.h
    src/framework/qmcolorchooserdelegate.cpp
    src/framework/qmcolorchooserdelegate.h
    src/settings/qmtrainsettingswidget.cpp
    src/settings/qmtrainsettingswidget.h
    src/settings/qmdatabasesettings.cpp
    src/settings/qmdatabasesettings.h
    src/settings/qmgeneralsettingswidget.cpp
    src/settings/qmgeneralsettingswidget.h
    src/settings/qmqualimatrixsettingswidget.cpp
    src/settings/qmqualimatrixsettingswidget.h
    src/settings/qmqualiresultsettingswidget.cpp
    src/settings/qmqualiresultsettingswidget.h
    src/qualimatrix/qmqualimatrixwidget.cpp
    src/qualimatrix/qmqualimatrixwidget.h
    src/qualiresult/qmqualiresultwidget.cpp
    src/qualiresult/qmqualiresultwidget.h
    src/traindata/qmtraindatawidget.cpp
    src/traindata/qmtraindatawidget.h
    src/traindata/qmimportcsvdialog.cpp
    src/traindata/qmimportcsvdialog.h
    src/framework/qmsqlrelationaltablemodel.cpp
    src/framework/qmsqlrelationaltablemodel.h
    src/framework/qminfolabel.cpp
    src/framework/qminfolabel.h
    src/framework/qmextendedselectiondialog.cpp
    src/framework/qmextendedselectiondialog.h
    src/framework/qmplaintexteditdelegate.cpp
    src/framework/qmplaintexteditdelegate.h
    src/framework/qmtreesettingsdelegate.cpp
    src/framework/qmtreesettingsdelegate.h
    src/database/qmdatabasedialog.cpp
    src/database/qmdatabasedialog.h
    src/database/qmdatabaseupdatedialog.cpp
    src/database/qmdatabaseupdatedialog.h
    src/database/qmdatabaseupdater.cpp
    src/database/qmdatabaseupdater.h
    src/framework/qmbooleandelegate.cpp
    src/framework/qmbooleandelegate.h
    src/settings/qmemployeedetailsdialog.cpp
    src/settings/qmemployeedetailsdialog.h
    src/model/qmfunctionmodel.cpp
    src/model/qmfunctionmodel.h
    src/model/qmtrainingmodel.cpp
    src/model/qmtrainingmodel.h
    src/model/qmtrainingdatamodel.cpp
    src/model/qmtrainingdatamodel.h
    src/model/qmfunctiongroupmodel.cpp
    src/model/qmfunctiongroupmodel.h
    src/model/qmtraininggroupmodel.cpp
    src/model/qmtraininggroupmodel.h
    src/model/qmtrainingdatastatemodel.cpp
    src/model/qmtrainingdatastatemodel.h
    src/model/qmemployeemodel.cpp
    src/model/qmemployeemodel.h
    src/model/qmemployeeviewmodel.cpp
    src/model/qmemployeeviewmodel.h
    src/model/qmemployeefunctionmodel.cpp
    src/model/qmemployeefunctionmodel.h
    src/model/qmqualificationmatrixmodel.cpp
    src/model/qmqualificationmatrixmodel.h
    src/model/qmtrainingexceptionmodel.cpp
    src/model/qmtrainingexceptionmodel.h
    src/model/qminfomodel.cpp
    src/model/qminfomodel.h
    src/model/qmcertificatemodel.cpp
    src/model/qmcertificatemodel.h
    src/model/qmtraindatacertificatemodel.cpp
    src/model/qmtraindatacertificatemodel.h
    src/model/qmtraindatacertificateviewmodel.cpp
    src/model/qmtraindatacertificateviewmodel.h
    src/general/qmprogresslogdialog.cpp
    src/general/qmprogresslogdialog.h
    src/certificate/qmcertificatedialog.cpp
    src/certificate/qmcertificatedialog.h
    src/certificate/qmcertificateintegritycheckdialog.cpp
    src/certificate/qmcertificateintegritycheckdialog.h
    src/certificate/qmnewcertificatedialog.cpp
    src/certificate/qmnewcertificatedialog.h
    src/signinglist/qmsigninglistdialog.cpp
    src/signinglist/qmsigninglistdialog.h
    src/signinglist/qmsigninglistdocument.cpp
    src/signinglist/qmsigninglistdocument.h
    ui/qmemployeesettingswidget.ui
    ui/qmfuncsettingswidget.ui
    ui/qmmainwindow.ui
    ui/qmqualimatrixwidget.ui
    ui/qmqualiresultwidget.ui
    ui/qmsettingsdialog.ui
    ui/qmtraindatawidget.ui
    ui/qmtrainsettingswidget.ui
    ui/qmimportcsvdialog.ui
    ui/qmdatabasedialog.ui
    ui/qmemployeedetailsdialog.ui
    ui/qmprogresslogdialog.ui
    ui/qmgeneralsettingswidget.ui
    ui/qmqualimatrixsettingswidget.ui
    ui/qmqualiresultsettingswidget.ui
    ui/qmdatabaseupdatedialog.ui
    ui/qmcertificatedialog.ui
    ui/qmcertificateintegritycheckdialog.ui
    ui/qmsigninglistdialog.ui
    ui/qmnewcertificatedialog.ui
    ui/qmextendedselectiondialog.ui
    src/config.h
    ${APP_ICON_RESOURCE_WINDOWS})


target_link_libraries(QualificationMatrix Qt5::Widgets Qt5::Sql Qt5::PrintSupport)

# -------------------------------------------------------------------------------------------------
# Deploy

# Copy the database directory
add_custom_command(
    TARGET QualificationMatrix PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/database $<TARGET_FILE_DIR:QualificationMatrix>/database)

# Copy translation files
target_sources(QualificationMatrix PRIVATE ${QM_FILES})
