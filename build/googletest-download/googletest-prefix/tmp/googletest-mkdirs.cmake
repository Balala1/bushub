# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-src"
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-build"
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix"
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix/tmp"
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix/src/googletest-stamp"
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix/src"
  "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix/src/googletest-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix/src/googletest-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/taoyuan/CLionProjects/bushub/build/googletest-download/googletest-prefix/src/googletest-stamp${cfgdir}") # cfgdir has leading slash
endif()
