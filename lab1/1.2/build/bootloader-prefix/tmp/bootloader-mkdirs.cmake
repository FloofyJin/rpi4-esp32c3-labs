# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/nymph/esp/esp-idf/components/bootloader/subproject"
  "/home/nymph/esp/1.2/build/bootloader"
  "/home/nymph/esp/1.2/build/bootloader-prefix"
  "/home/nymph/esp/1.2/build/bootloader-prefix/tmp"
  "/home/nymph/esp/1.2/build/bootloader-prefix/src/bootloader-stamp"
  "/home/nymph/esp/1.2/build/bootloader-prefix/src"
  "/home/nymph/esp/1.2/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/nymph/esp/1.2/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/nymph/esp/1.2/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
