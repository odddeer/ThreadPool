# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kent/threadpool

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kent/threadpool/build

# Include any dependencies generated for this target.
include CMakeFiles/threadpool.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/threadpool.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/threadpool.dir/flags.make

CMakeFiles/threadpool.dir/test.cpp.o: CMakeFiles/threadpool.dir/flags.make
CMakeFiles/threadpool.dir/test.cpp.o: ../test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kent/threadpool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/threadpool.dir/test.cpp.o"
	/opt/rh/devtoolset-7/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/threadpool.dir/test.cpp.o -c /home/kent/threadpool/test.cpp

CMakeFiles/threadpool.dir/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/threadpool.dir/test.cpp.i"
	/opt/rh/devtoolset-7/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kent/threadpool/test.cpp > CMakeFiles/threadpool.dir/test.cpp.i

CMakeFiles/threadpool.dir/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/threadpool.dir/test.cpp.s"
	/opt/rh/devtoolset-7/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kent/threadpool/test.cpp -o CMakeFiles/threadpool.dir/test.cpp.s

CMakeFiles/threadpool.dir/threadpool.cpp.o: CMakeFiles/threadpool.dir/flags.make
CMakeFiles/threadpool.dir/threadpool.cpp.o: ../threadpool.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kent/threadpool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/threadpool.dir/threadpool.cpp.o"
	/opt/rh/devtoolset-7/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/threadpool.dir/threadpool.cpp.o -c /home/kent/threadpool/threadpool.cpp

CMakeFiles/threadpool.dir/threadpool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/threadpool.dir/threadpool.cpp.i"
	/opt/rh/devtoolset-7/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kent/threadpool/threadpool.cpp > CMakeFiles/threadpool.dir/threadpool.cpp.i

CMakeFiles/threadpool.dir/threadpool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/threadpool.dir/threadpool.cpp.s"
	/opt/rh/devtoolset-7/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kent/threadpool/threadpool.cpp -o CMakeFiles/threadpool.dir/threadpool.cpp.s

# Object files for target threadpool
threadpool_OBJECTS = \
"CMakeFiles/threadpool.dir/test.cpp.o" \
"CMakeFiles/threadpool.dir/threadpool.cpp.o"

# External object files for target threadpool
threadpool_EXTERNAL_OBJECTS =

../bin/threadpool: CMakeFiles/threadpool.dir/test.cpp.o
../bin/threadpool: CMakeFiles/threadpool.dir/threadpool.cpp.o
../bin/threadpool: CMakeFiles/threadpool.dir/build.make
../bin/threadpool: CMakeFiles/threadpool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kent/threadpool/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../bin/threadpool"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/threadpool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/threadpool.dir/build: ../bin/threadpool

.PHONY : CMakeFiles/threadpool.dir/build

CMakeFiles/threadpool.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/threadpool.dir/cmake_clean.cmake
.PHONY : CMakeFiles/threadpool.dir/clean

CMakeFiles/threadpool.dir/depend:
	cd /home/kent/threadpool/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kent/threadpool /home/kent/threadpool /home/kent/threadpool/build /home/kent/threadpool/build /home/kent/threadpool/build/CMakeFiles/threadpool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/threadpool.dir/depend

