##################### CMAKE CONFIGS ####################
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_EXECUTABLE_SUFFIX_C .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)

#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#################### LIBRARIES AND TOOLCHAIN #################
set(PACKS_REPO "C:/Program Files (x86)/Atmel/Studio/7.0/packs")

if (NOT DEFINED ENV{AVR_TOOLCHAIN_FOLDER})
    message(FATAL_ERROR "AVR_TOOLCHAIN_FOLDER Environment variable is not set! (example: C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\avr\include)")
endif ()

message(DEBUG " use external tools? ${USE_EXTERNAL_TOOLSET} :: toolsdir: ${TOOLSDIR}")
if ($ENV{USE_EXTERNAL_TOOLSET})
    set(TOOLSDIR "C:/dev/tools/arm_gcc_toolchain/10 2021.10/bin")
else ()
#    set(TOOLSDIR "C:/Program Files (x86)/Atmel/Studio/7.0/toolchain/avr8/avr8-gnu-toolchain/bin")
#    set(TOOLSDIR "C:/dev/tools/avr8-gnu-toolchain-win32_x86_64/bin")
    set(TOOLSDIR "C:/dev/tools/avr-gcc-12.1.0-x64-windows/avr-gcc-12.1.0-x64-windows/bin")
endif ()

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

find_program(CMAKE_C_COMPILER NAMES avr-gcc HINTS ${TOOLSDIR})
find_program(CMAKE_CXX_COMPILER NAMES avr-g++ HINTS ${TOOLSDIR})
find_program(CMAKE_OBJCOPY NAMES avr-objcopy HINTS ${TOOLSDIR})
message(DEBUG "Use compiler executable as linker? ${USE_COMPILER_EXECUTABLE_AS_LINKER}")
if (USE_COMPILER_EXECUTABLE_AS_LINKER)
    find_program(CMAKE_LINKER NAMES avr-g++ HINTS ${TOOLSDIR})
else ()
    find_program(CMAKE_LINKER NAMES avr-ld HINTS ${TOOLSDIR})
endif ()
find_program(CMAKE_SIZE NAMES avr-size HINTS ${TOOLSDIR})

function(display_size TARGET)
    add_custom_target(${TARGET}_display_size ALL
                      COMMAND ${CMAKE_SIZE} ${TARGET}${CMAKE_EXECUTABLE_SUFFIX_C}
                      COMMENT "Target Size:"
                      DEPENDS ${TARGET}${CMAKE_EXECUTABLE_SUFFIX_C}
                      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                      )
endfunction(display_size)

function(generate_hex_file FORTARGET)
    add_custom_command(TARGET ${FORTARGET} POST_BUILD
                       COMMAND ${CMAKE_OBJCOPY} -O ihex ${FORTARGET}${CMAKE_EXECUTABLE_SUFFIX_C} ${FORTARGET}.hex
                       COMMENT "Generating hex file ${FORTARGET}.hex"
                       DEPENDS ${TARGET}${CMAKE_EXECUTABLE_SUFFIX_C}
                       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                       )
endfunction(generate_hex_file)

function(generate_binary_file MYTARGET)
    add_custom_command(TARGET ${MYTARGET} POST_BUILD
                       COMMAND ${CMAKE_OBJCOPY} -O binary ${MYTARGET}${CMAKE_EXECUTABLE_SUFFIX_C} ${MYTARGET}.bin
                       DEPENDS ${MYTARGET}${CMAKE_EXECUTABLE_SUFFIX_C}
                       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                       COMMENT "Generating binary file" ${MYTARGET}.bin
                       )
endfunction(generate_binary_file)

function(attiny_add_executable EXECUTABLE_NAME)
    set(additional_source_files ${ARGN})
    list(LENGTH additional_source_files num_of_source_files)

    if (num_of_source_files LESS 1)
        message(FATAL_ERROR "No source files provided for ${EXECUTABLE_NAME}")
    endif ()

    set(ELF_OUTPUT_FILE "${EXECUTABLE_NAME}.elf")
    set(BIN_OUTPUT_FILE "${EXECUTABLE_NAME}.bin")
    set(UF2_OUTPUT_FILE "${EXECUTABLE_NAME}.uf2")
    set(MAP_OUTPUT_FILE "${EXECUTABLE_NAME}.map")

    add_executable(${EXECUTABLE_NAME} ${additional_source_files})

#    set_target_properties(${EXECUTABLE_NAME}
#                          PROPERTIES
#                          LINK_FLAGS ${MY_LINK_FLAGS})

    target_include_directories(${EXECUTABLE_NAME} PRIVATE $ENV{AVR_TOOLCHAIN_FOLDER})
#    target_link_libraries(${EXECUTABLE_NAME} ${MY_LIBS})

#    get_directory_property(
#            clean_files ADDITIONAL_MAKE_CLEAN_FILES
#    )

endfunction(attiny_add_executable)
