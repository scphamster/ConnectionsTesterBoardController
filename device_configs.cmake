###################### DEVICE SPECIFIC CONFIGURATIONS ############
if (NOT DEFINED ENV{MCU_TYPE})
    message(FATAL_ERROR "Mcu type is not defined, add cmake environment variable MCU_TYPE with mcu used (e.g. attiny84a)")
endif ()

################### DIRECTORIES CONFIGURATIONS #################
#set(SOURCES_DIRECTORY src)
if (NOT DEFINED ENV{SOURCES_DIRECTORY})
    message(FATAL_ERROR "sources directory is not set in environment variables")
endif ()
message(INFO "Sources directory is: $ENV{SOURCES_DIRECTORY}}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

############## TOOLSET CONFIGURATIONS ####################
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_HOME_DIRECTORY}/$ENV{MCU_TYPE}.cmake) #make this configuration by env variable
message(DEBUG "toolchain file is  ${CMAKE_TOOLCHAIN_FILE}")
set(USE_COMPILER_EXECUTABLE_AS_LINKER TRUE)
set(LINKER_VERBOSE_LIBRARY_INFO FALSE)
set(COMPILER_SHOW_INCLUDED_HEADERS FALSE)

if (LINKER_VERBOSE_LIBRARY_INFO)
    set(LINKER_VERBOSE_CMD "-Wl,--trace")
else ()
    set(LINKER_VERBOSE_CMD "")
endif ()

if (LINKER_VERBOSE_LIBRARY_INFO)
    set(COMPILER_SHOW_HEADERS_CMD "-H")
else ()
    set(COMPILER_SHOW_HEADERS_CMD "")
endif ()

################# COMPILER FLAGS ####################
set(C_CXX_COMPILER_OPTIONS_DEBUG "-Og -g3 -DDEBUG -ffunction-sections -fdata-sections -fpack-struct -fshort-enums ")
set(C_CXX_COMPILER_OPTIONS_RELEASE "-O3 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -DNDEBUG --param max-inline-insns-single=500")
set(CXX_COMPILER_OPTIONS_RELEASE "-fno-rtti")
set(COMPILER_WARNINGS_SETTINGS "-Wall -Werror=return-type -Wreturn-local-addr -Wno-volatile -Wno-unused-function -Wno-unused-variable -Wno-unused-local-typedefs ")
set(BOARD_CONFIGS_COMPILE_FLAGS "-D__AVR_ATtiny84A__ ")
#set(FPU_SETTINGS_COMPILE_FLAGS "-DARM_MATH_CM4=true -mfloat-abi=softfp -mfpu=fpv4-sp-d16")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(C_CXX_CONDITIONAL_COMPILE_FLAGS ${C_CXX_COMPILER_OPTIONS_DEBUG})
    set(CXX_CONDITIONAL_COMPILE_FLAGS ${CXX_COMPILER_OPTIONS_DEBUG})
else ()
    set(C_CXX_CONDITIONAL_COMPILE_FLAGS ${C_CXX_COMPILER_OPTIONS_RELEASE})
endif ()

set(C_CXX_COMPILE_FLAGS "${COMPILER_WARNINGS_SETTINGS} ${BOARD_CONFIGS_COMPILE_FLAGS} ${COMPILER_SHOW_HEADERS_CMD} ${C_CXX_CONDITIONAL_COMPILE_FLAGS} ${FPU_SETTINGS_COMPILE_FLAGS}  -ffunction-sections -c")
set(C_COMPILE_FLAGS "-x c -std=gnu11")
set(CXX_COMPILE_FLAGS "${CXX_CONDITIONAL_COMPILE_FLAGS} -fno-exceptions -std=c++23")

####################### LINKER SCRIPT ##########################
#set(LINKER_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/src/asf_lib/src/ASF/sam/utils/linker_scripts/sam4e/sam4e8/gcc")
#set(LINKER_SCRIPT_FILENAME flash.ld)
#set(LINKER_SCRIPT_COMMAND "-L\"${LINKER_SCRIPT_DIR}\" -T${LINKER_SCRIPT_FILENAME}")
set(LINKER_MAP_GEN_COMMAND "-Wl,-Map=\"${PROJECT_NAME}.map\"")
set(MY_LINK_FLAGS "${LINKER_VERBOSE_CMD} --specs=nosys.specs ${LINKER_MAP_GEN_COMMAND} -Wl,--start-group -lm -Wl,--end-group -Wl,--gc-sections ${LINKER_SCRIPT_COMMAND}")
message(DEBUG "link flags: ${MY_LINK_FLAGS}")
