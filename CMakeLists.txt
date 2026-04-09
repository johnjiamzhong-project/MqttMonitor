cmake_minimum_required(VERSION 3.16)

project(MqttMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 PahoMqttCpp REQUIRED COMPONENTS Widgets)

add_executable(MqttMonitor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

set_target_properties(MqttMonitor PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(MqttMonitor PRIVATE 
    Qt5::Widgets
    PahoMqttCpp::paho-mqttpp3
    )

include(GNUInstallDirs)
install(TARGETS MqttMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)