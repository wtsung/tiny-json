# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /Users/wu/Desktop/myjson-oop

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/wu/Desktop/myjson-oop/cmake-build-debug

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/wu/Desktop/myjson-oop/cmake-build-debug/CMakeFiles /Users/wu/Desktop/myjson-oop/cmake-build-debug//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/wu/Desktop/myjson-oop/cmake-build-debug/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named myjson_oop

# Build rule for target.
myjson_oop: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 myjson_oop
.PHONY : myjson_oop

# fast build rule for target.
myjson_oop/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/build
.PHONY : myjson_oop/fast

JSONValue.o: JSONValue.cpp.o

.PHONY : JSONValue.o

# target to build an object file
JSONValue.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/JSONValue.cpp.o
.PHONY : JSONValue.cpp.o

JSONValue.i: JSONValue.cpp.i

.PHONY : JSONValue.i

# target to preprocess a source file
JSONValue.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/JSONValue.cpp.i
.PHONY : JSONValue.cpp.i

JSONValue.s: JSONValue.cpp.s

.PHONY : JSONValue.s

# target to generate assembly for a file
JSONValue.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/JSONValue.cpp.s
.PHONY : JSONValue.cpp.s

t.o: t.cpp.o

.PHONY : t.o

# target to build an object file
t.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/t.cpp.o
.PHONY : t.cpp.o

t.i: t.cpp.i

.PHONY : t.i

# target to preprocess a source file
t.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/t.cpp.i
.PHONY : t.cpp.i

t.s: t.cpp.s

.PHONY : t.s

# target to generate assembly for a file
t.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/t.cpp.s
.PHONY : t.cpp.s

test.o: test.cpp.o

.PHONY : test.o

# target to build an object file
test.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/test.cpp.o
.PHONY : test.cpp.o

test.i: test.cpp.i

.PHONY : test.i

# target to preprocess a source file
test.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/test.cpp.i
.PHONY : test.cpp.i

test.s: test.cpp.s

.PHONY : test.s

# target to generate assembly for a file
test.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/myjson_oop.dir/build.make CMakeFiles/myjson_oop.dir/test.cpp.s
.PHONY : test.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... myjson_oop"
	@echo "... JSONValue.o"
	@echo "... JSONValue.i"
	@echo "... JSONValue.s"
	@echo "... t.o"
	@echo "... t.i"
	@echo "... t.s"
	@echo "... test.o"
	@echo "... test.i"
	@echo "... test.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

