cmake_minimum_required(VERSION 3.13.0)

project(QualificationMatrix)

include_directories(${QualificationMatrix_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

find_package(Qt5 COMPONENTS LinguistTools Widgets Sql PrintSupport REQUIRED)

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

# Create executable and link libraries.

add_executable(QualificationMatrix WIN32
    icons.qrc
    src/model/qmdatamanager.cpp
    src/framework/qmdatedelegate.cpp
    src/main.cpp
    src/framework/qmdialog.cpp
    src/general/qmmainwindow.cpp
    src/framework/qmproxysqlrelationaldelegate.cpp
    src/qualimatrix/qmqualimatrixdelegate.cpp
    src/qualimatrix/qmqualimatrixheaderview.cpp
    src/qualimatrix/qmqualimatrixmodel.cpp
    src/qualimatrix/qmtableview.cpp
    src/qualiresult/qmqualiresultmodel.cpp
    src/qualiresult/qmqualiresultrecord.cpp
    src/settings/qmsettingsdialog.cpp
    src/settings/qmfuncsettingswidget.cpp
    src/settings/qmemployeesettingswidget.cpp
    src/settings/qmsettingswidget.cpp
    src/settings/qmsettings.cpp
    src/settings/qmapplicationsettings.cpp
    src/framework/qmcolorchooserdelegate.cpp
    src/settings/qmtrainsettingswidget.cpp
    src/settings/qmdatabasesettings.cpp
    src/settings/qmgeneralsettingswidget.cpp
    src/settings/qmqualimatrixsettingswidget.cpp
    src/settings/qmqualiresultsettingswidget.cpp
    src/qualimatrix/qmqualimatrixwidget.cpp
    src/qualiresult/qmqualiresultwidget.cpp
    src/traindata/qmtraindatawidget.cpp
    src/traindata/qmimportcsvdialog.cpp
    src/framework/qmsqlrelationaltablemodel.cpp
    src/framework/qminfolabel.cpp
    src/framework/qmextendedselectiondialog.cpp
    src/framework/qmplaintexteditdelegate.cpp
    src/database/qmdatabasedialog.cpp
    src/database/qmdatabaseupdatedialog.cpp
    src/database/qmdatabaseupdater.cpp
    src/framework/qmbooleandelegate.cpp
    src/settings/qmemployeedetailsdialog.cpp
    src/model/qmfunctionmodel.cpp
    src/model/qmtrainingmodel.cpp
    src/model/qmtrainingdatamodel.cpp
    src/model/qmfunctiongroupmodel.cpp
    src/model/qmtraininggroupmodel.cpp
    src/model/qmtrainingdatastatemodel.cpp
    src/model/qmemployeemodel.cpp
    src/model/qmemployeeviewmodel.cpp
    src/model/qmemployeefunctionmodel.cpp
    src/model/qmqualificationmatrixmodel.cpp
    src/model/qmtrainingexceptionmodel.cpp
    src/model/qminfomodel.cpp
    src/model/qmcertificatemodel.cpp
    src/model/qmtraindatacertificatemodel.cpp
    src/model/qmtraindatacertificateviewmodel.cpp
    src/general/qmprogresslogdialog.cpp
    src/certificate/qmcertificatedialog.cpp
    src/certificate/qmcertificateintegritycheckdialog.cpp
    src/certificate/qmnewcertificatedialog.cpp
    src/signinglist/qmsigninglistdialog.cpp
    src/signinglist/qmsigninglistdocument.cpp
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
    src/config.h)

# copy database directory to binary
add_custom_command(
    TARGET QualificationMatrix PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/database $<TARGET_FILE_DIR:QualificationMatrix>/database)

target_sources(QualificationMatrix PRIVATE ${QM_FILES})

target_link_libraries(QualificationMatrix Qt5::Widgets Qt5::Sql Qt5::PrintSupport)
