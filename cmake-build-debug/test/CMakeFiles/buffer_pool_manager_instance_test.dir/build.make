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
include test/CMakeFiles/buffer_pool_manager_instance_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/buffer_pool_manager_instance_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/buffer_pool_manager_instance_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/buffer_pool_manager_instance_test.dir/flags.make

test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o: test/CMakeFiles/buffer_pool_manager_instance_test.dir/flags.make
test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o: ../test/buffer/buffer_pool_manager_instance_test.cpp
test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o: test/CMakeFiles/buffer_pool_manager_instance_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/taoyuan/CLionProjects/bushub/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o -MF CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o.d -o CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o -c /Users/taoyuan/CLionProjects/bushub/test/buffer/buffer_pool_manager_instance_test.cpp

test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.i"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/taoyuan/CLionProjects/bushub/test/buffer/buffer_pool_manager_instance_test.cpp > CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.i

test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.s"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/taoyuan/CLionProjects/bushub/test/buffer/buffer_pool_manager_instance_test.cpp -o CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.s

# Object files for target buffer_pool_manager_instance_test
buffer_pool_manager_instance_test_OBJECTS = \
"CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o"

# External object files for target buffer_pool_manager_instance_test
buffer_pool_manager_instance_test_EXTERNAL_OBJECTS =

test/buffer_pool_manager_instance_test: test/CMakeFiles/buffer_pool_manager_instance_test.dir/buffer/buffer_pool_manager_instance_test.cpp.o
test/buffer_pool_manager_instance_test: test/CMakeFiles/buffer_pool_manager_instance_test.dir/build.make
test/buffer_pool_manager_instance_test: lib/libbustub_shared.dylib
test/buffer_pool_manager_instance_test: lib/libgmock_main.1.12.1.dylib
test/buffer_pool_manager_instance_test: lib/libthirdparty_murmur3.dylib
test/buffer_pool_manager_instance_test: lib/libgmock.1.12.1.dylib
test/buffer_pool_manager_instance_test: lib/libgtest.1.12.1.dylib
test/buffer_pool_manager_instance_test: test/CMakeFiles/buffer_pool_manager_instance_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/taoyuan/CLionProjects/bushub/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable buffer_pool_manager_instance_test"
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/buffer_pool_manager_instance_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/buffer_pool_manager_instance_test.dir/build: test/buffer_pool_manager_instance_test
.PHONY : test/CMakeFiles/buffer_pool_manager_instance_test.dir/build

test/CMakeFiles/buffer_pool_manager_instance_test.dir/clean:
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test && $(CMAKE_COMMAND) -P CMakeFiles/buffer_pool_manager_instance_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/buffer_pool_manager_instance_test.dir/clean

test/CMakeFiles/buffer_pool_manager_instance_test.dir/depend:
	cd /Users/taoyuan/CLionProjects/bushub/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/taoyuan/CLionProjects/bushub /Users/taoyuan/CLionProjects/bushub/test /Users/taoyuan/CLionProjects/bushub/cmake-build-debug /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test /Users/taoyuan/CLionProjects/bushub/cmake-build-debug/test/CMakeFiles/buffer_pool_manager_instance_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/buffer_pool_manager_instance_test.dir/depend

