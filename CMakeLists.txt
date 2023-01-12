cmake_minimum_required(VERSION 3.5)

##########################INCLUDES########################
include(device_configs.cmake)
include(attiny84a.cmake)
######################## PROJECT #########################

project(MuxController LANGUAGES CXX C)
set(EXECUTABLE_NAME ${PROJECT_NAME})

#########################SOURCES##########################
set(SRC_DIR $ENV{SOURCES_DIRECTORY})

set(CXX_SOURCES
    ${SRC_DIR}/main.cpp

    ${SRC_DIR}/adc.cpp
    ${SRC_DIR}/adc.hpp

    ${SRC_DIR}/pio_controller.cpp
    ${SRC_DIR}/pio_controller.hpp

    ${SRC_DIR}/command_handler.hpp
    ${SRC_DIR}/command_handler.cpp

    ${SRC_DIR}/shifter.hpp
    ${SRC_DIR}/shifter.cpp

    ${SRC_DIR}/analog_switch.hpp

    ${SRC_DIR}/timer.hpp
    ${SRC_DIR}/timer.cpp

    ${SRC_DIR}/iic.hpp
    ${SRC_DIR}/iic.cpp

    ${SRC_DIR}/my_heap.hpp
    ${SRC_DIR}/my_heap.cpp

    ${SRC_DIR}/ohm_meter.hpp
    ${SRC_DIR}/ohm_meter.cpp

    ${SRC_DIR}/i2c_driver_cpp/i2c_driver.cpp
    ${SRC_DIR}/i2c_driver_cpp/i2c_driver.hpp

    ${SRC_DIR}/EEPROMController.hpp

    )
set(C_SOURCES
    ${SRC_DIR}/debug_tools.h
    ${SRC_DIR}/debug_tools.c
    ${SRC_DIR}/project_configs.h
    ${SRC_DIR}/USI_TWI_Master.h
    ${SRC_DIR}/USI_TWI_Master.c
    ${SRC_DIR}/USI_TWI_Slave.h
    ${SRC_DIR}/USI_TWI_Slave.c
    )

set(INCLUDE_DIRECTORIES
    ${SRC_DIR}
    ${SRC_DIR}/i2c_driver_cpp
    "C:/dev/Libraries/avr-libstdcpp/include" # standard library

    )

set_source_files_properties(${CXX_SOURCES} PROPERTIES COMPILE_FLAGS "${C_CXX_COMPILE_FLAGS} ${CXX_COMPILE_FLAGS}")
set_source_files_properties(${C_SOURCES} PROPERTIES COMPILE_FLAGS "${C_CXX_COMPILE_FLAGS} ${C_COMPILE_FLAGS}")

attiny_add_executable(${EXECUTABLE_NAME} ${CXX_SOURCES} ${C_SOURCES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_DIRECTORIES})
display_size(${EXECUTABLE_NAME})