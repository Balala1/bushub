# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/taoyuan/CLionProjects/bushub

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/taoyuan/CLionProjects/bushub/cmake-build-debug

# Include any dependencies generated for this target.
include src/CMakeFiles/thirdparty_murmur3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/thirdparty_murmur3.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/thirdparty_murmur3.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/thirdparty_murmur3.dir/flags.make

src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o: src/CMakeFiles/thirdparty_murmur3.dir/flags.make
src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o: ../third_party/murmur3/MurmurHash3.cpp
src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o: src/CMakeFiles/thirdparty_murmur3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/taoyuan/CLionProjects/bushub/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o -MF CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o.d -o CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o -c /Users/taoyuan/CLionProjects/bushub/third_party/murmur3/MurmurHash3.cpp

src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.i"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/taoyuan/CLionProjects/bushub/third_party/murmur3/MurmurHash3.cpp > CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.i

src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.s"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/taoyuan/CLionProjects/bushub/third_party/murmur3/MurmurHash3.cpp -o CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.s

# Object files for target thirdparty_murmur3
thirdparty_murmur3_OBJECTS = \
"CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o"

# External object files for target thirdparty_murmur3
thirdparty_murmur3_EXTERNAL_OBJECTS =

lib/libthirdparty_murmur3.dylib: src/CMakeFiles/thirdparty_murmur3.dir/__/third_party/murmur3/MurmurHash3.cpp.o
lib/libthirdparty_murmur3.dylib: src/CMakeFiles/thirdparty_murmur3.dir/build.make
lib/libthirdparty_murmur3.dylib: src/CMakeFiles/thirdparty_murmur3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/taoyuan/CLionProjects/bushub/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library ../lib/libthirdparty_murmur3.dylib"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/thirdparty_murmur3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/thirdparty_murmur3.dir/build: lib/libthirdparty_murmur3.dylib
.PHONY : src/CMakeFiles/thirdparty_murmur3.dir/build

src/CMakeFiles/thirdparty_murmur3.dir/clean:
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src && $(CMAKE_COMMAND) -P CMakeFiles/thirdparty_murmur3.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/thirdparty_murmur3.dir/clean

src/CMakeFiles/thirdparty_murmur3.dir/depend:
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/taoyuan/CLionProjects/bushub /Users/taoyuan/CLionProjects/bushub/src /Users/taoyuan/CLionProjects/bushub/cmake-build-debug /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/src/CMakeFiles/thirdparty_murmur3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/thirdparty_murmur3.dir/depend

