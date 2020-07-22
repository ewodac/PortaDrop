cmake_minimum_required(VERSION 3.8)
project(ewodInterface)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(PkgConfig) 
pkg_check_modules(GTKMM gtkmm-3.0) 

set(src "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(inc "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(sources
    ${src}/main.cpp
    ${src}/Camera_cv.cpp
    ${src}/CameraWindow.cpp
    ${src}/DataP.cpp
    ${src}/DelayTask.cpp
    ${src}/DialogExtVolt.cpp
    ${src}/DummyImpAnalyser.cpp
    ${src}/EmStatPico.cpp
    ${src}/ExperimentData.cpp
    ${src}/Freq_Test.cpp
    ${src}/FSHelper.cpp
    ${src}/GpibConnection.cpp
    ${src}/GUI.cpp
    ${src}/HP4294A.cpp
    ${src}/I2CFreqTask.cpp
    ${src}/I2CTask.cpp
    ${src}/I2CTempTask.cpp
    ${src}/I2CVoltageTask.cpp
    ${src}/ImpAnalyser.cpp
    ${src}/ImpAnalyserTask.cpp
    ${src}/ListView_Experiment.cpp
    ${src}/ListView_Freq.cpp
    ${src}/ListView_Project.cpp
    ${src}/ListView_SavedData.cpp
    ${src}/Logbook.cpp
    ${src}/Log_Event.cpp
    ${src}/MeasurementError.cpp
    ${src}/MeasurementPackage.cpp
    ${src}/MeasurementValue.cpp
    ${src}/Novocontrol.cpp
    ${src}/PadTask.cpp
    ${src}/PlotWindow.cpp
    ${src}/Preferences.cpp
    ${src}/Recipe.cpp
    ${src}/Relais.cpp
    ${src}/Spectrometer.cpp
    ${src}/SpectrometerTask.cpp
    ${src}/StatusLed.cpp
    ${src}/Task.cpp
    ${src}/TempData.cpp
    ${src}/TestTask.cpp
    ${src}/Timer.cpp
    ${src}/tinyxml2.cpp
    ${src}/TransientGUIHandler.cpp
    ${src}/TransImpTask.cpp
    ${src}/TransSpect.cpp
    ${src}/TreeView_Recipe.cpp
    ${src}/Uc_Connection.cpp
    )
    
add_executable(ewodInterface ${sources})

link_directories(${GTKMM_LIBRARY_DIRS}) 

target_include_directories(ewodInterface PRIVATE ${inc})
include_directories(${WIRINGPI_INCLUDE_DIRS})
include_directories(include ${GTKMM_INCLUDE_DIRS})

target_link_libraries(ewodInterface -lwiringPi)
target_link_libraries(ewodInterface -lpthread)
target_link_libraries(ewodInterface -lseabreeze)
target_link_libraries(ewodInterface -lusb)
target_link_libraries(ewodInterface -lgpib)
target_link_libraries(ewodInterface -lraspicam)
target_link_libraries(ewodInterface -lraspicam_cv)
target_link_libraries(ewodInterface -lopencv_core)
target_link_libraries(ewodInterface -lopencv_highgui)
target_link_libraries(ewodInterface -lopencv_imgcodecs)
target_link_libraries(ewodInterface -lopencv_videoio)
target_link_libraries(ewodInterface -lopencv_imgproc)
target_link_libraries(ewodInterface ${GTKMM_LIBRARIES}) 

install(TARGETS ewodInterface DESTINATION bin)
install(FILES style/ewod_gui.glade DESTINATION glade)
install(FILES style/styles.css DESTINATION glade)
