# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/samhi/OuterTracker/Ph2_ACF

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build

# Include any dependencies generated for this target.
include System/CMakeFiles/Ph2_System.dir/depend.make

# Include the progress variables for this target.
include System/CMakeFiles/Ph2_System.dir/progress.make

# Include the compile flags for this target's objects.
include System/CMakeFiles/Ph2_System.dir/flags.make

System/CMakeFiles/Ph2_System.dir/FileParser.cc.o: System/CMakeFiles/Ph2_System.dir/flags.make
System/CMakeFiles/Ph2_System.dir/FileParser.cc.o: ../System/FileParser.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object System/CMakeFiles/Ph2_System.dir/FileParser.cc.o"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Ph2_System.dir/FileParser.cc.o -c /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System/FileParser.cc

System/CMakeFiles/Ph2_System.dir/FileParser.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Ph2_System.dir/FileParser.cc.i"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System/FileParser.cc > CMakeFiles/Ph2_System.dir/FileParser.cc.i

System/CMakeFiles/Ph2_System.dir/FileParser.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Ph2_System.dir/FileParser.cc.s"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System/FileParser.cc -o CMakeFiles/Ph2_System.dir/FileParser.cc.s

System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.requires:

.PHONY : System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.requires

System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.provides: System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.requires
	$(MAKE) -f System/CMakeFiles/Ph2_System.dir/build.make System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.provides.build
.PHONY : System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.provides

System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.provides.build: System/CMakeFiles/Ph2_System.dir/FileParser.cc.o


System/CMakeFiles/Ph2_System.dir/SystemController.cc.o: System/CMakeFiles/Ph2_System.dir/flags.make
System/CMakeFiles/Ph2_System.dir/SystemController.cc.o: ../System/SystemController.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object System/CMakeFiles/Ph2_System.dir/SystemController.cc.o"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Ph2_System.dir/SystemController.cc.o -c /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System/SystemController.cc

System/CMakeFiles/Ph2_System.dir/SystemController.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Ph2_System.dir/SystemController.cc.i"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System/SystemController.cc > CMakeFiles/Ph2_System.dir/SystemController.cc.i

System/CMakeFiles/Ph2_System.dir/SystemController.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Ph2_System.dir/SystemController.cc.s"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System/SystemController.cc -o CMakeFiles/Ph2_System.dir/SystemController.cc.s

System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.requires:

.PHONY : System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.requires

System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.provides: System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.requires
	$(MAKE) -f System/CMakeFiles/Ph2_System.dir/build.make System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.provides.build
.PHONY : System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.provides

System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.provides.build: System/CMakeFiles/Ph2_System.dir/SystemController.cc.o


# Object files for target Ph2_System
Ph2_System_OBJECTS = \
"CMakeFiles/Ph2_System.dir/FileParser.cc.o" \
"CMakeFiles/Ph2_System.dir/SystemController.cc.o"

# External object files for target Ph2_System
Ph2_System_EXTERNAL_OBJECTS =

../lib/libPh2_System.so: System/CMakeFiles/Ph2_System.dir/FileParser.cc.o
../lib/libPh2_System.so: System/CMakeFiles/Ph2_System.dir/SystemController.cc.o
../lib/libPh2_System.so: System/CMakeFiles/Ph2_System.dir/build.make
../lib/libPh2_System.so: ../lib/libPh2_Interface.so
../lib/libPh2_System.so: ../lib/libPh2_Utils.so
../lib/libPh2_System.so: ../lib/libPh2_Description.so
../lib/libPh2_System.so: System/CMakeFiles/Ph2_System.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library ../../lib/libPh2_System.so"
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Ph2_System.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
System/CMakeFiles/Ph2_System.dir/build: ../lib/libPh2_System.so

.PHONY : System/CMakeFiles/Ph2_System.dir/build

System/CMakeFiles/Ph2_System.dir/requires: System/CMakeFiles/Ph2_System.dir/FileParser.cc.o.requires
System/CMakeFiles/Ph2_System.dir/requires: System/CMakeFiles/Ph2_System.dir/SystemController.cc.o.requires

.PHONY : System/CMakeFiles/Ph2_System.dir/requires

System/CMakeFiles/Ph2_System.dir/clean:
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System && $(CMAKE_COMMAND) -P CMakeFiles/Ph2_System.dir/cmake_clean.cmake
.PHONY : System/CMakeFiles/Ph2_System.dir/clean

System/CMakeFiles/Ph2_System.dir/depend:
	cd /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/samhi/OuterTracker/Ph2_ACF /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/System /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System /mnt/c/Users/samhi/OuterTracker/Ph2_ACF/build/System/CMakeFiles/Ph2_System.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : System/CMakeFiles/Ph2_System.dir/depend

