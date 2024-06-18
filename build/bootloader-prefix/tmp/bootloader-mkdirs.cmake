# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/andreasnorje/Code/esp/esp-idf/components/bootloader/subproject"
  "/Users/andreasnorje/Code/smartlash_c/build/bootloader"
  "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix"
  "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix/tmp"
  "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix/src"
  "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/andreasnorje/Code/smartlash_c/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
