cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS /Users/andreasnorje/Code/esp/esp-idf-lib/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Update the below line to match the path to the ttn-esp32 library,
# e.g. list(APPEND EXTRA_COMPONENT_DIRS "/Users/me/Documents/ttn-esp32")
# list(APPEND EXTRA_COMPONENT_DIRS "/Users/andreasnorje/Code/ttn-esp32")

#add_definitions(-DLMIC_ENABLE_event_logging=1)

project("Smartlash")
