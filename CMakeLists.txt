# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(StabilityBot_MK1)
set(PARTITION_TABLE_CSV_PATH "partitions.csv")


add_custom_target(flash-web
    COMMAND ${ESPTOOLPY} --chip esp32 --port COM5 --baud 921600
            write_flash 0x190000 ${CMAKE_BINARY_DIR}/storage.bin
    DEPENDS spiffs_storage_bin
    COMMENT "Flashing SPIFFS web partition only"
)