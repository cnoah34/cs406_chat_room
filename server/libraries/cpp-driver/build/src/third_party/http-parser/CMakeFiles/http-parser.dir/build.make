# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/colby/code/school/cs406_chat_room/cpp-driver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/colby/code/school/cs406_chat_room/cpp-driver/build

# Include any dependencies generated for this target.
include src/third_party/http-parser/CMakeFiles/http-parser.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/third_party/http-parser/CMakeFiles/http-parser.dir/compiler_depend.make

# Include the progress variables for this target.
include src/third_party/http-parser/CMakeFiles/http-parser.dir/progress.make

# Include the compile flags for this target's objects.
include src/third_party/http-parser/CMakeFiles/http-parser.dir/flags.make

src/third_party/http-parser/CMakeFiles/http-parser.dir/codegen:
.PHONY : src/third_party/http-parser/CMakeFiles/http-parser.dir/codegen

src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.o: src/third_party/http-parser/CMakeFiles/http-parser.dir/flags.make
src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.o: /home/colby/code/school/cs406_chat_room/cpp-driver/src/third_party/http-parser/http_parser.c
src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.o: src/third_party/http-parser/CMakeFiles/http-parser.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/colby/code/school/cs406_chat_room/cpp-driver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.o"
	cd /home/colby/code/school/cs406_chat_room/cpp-driver/build/src/third_party/http-parser && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.o -MF CMakeFiles/http-parser.dir/http_parser.c.o.d -o CMakeFiles/http-parser.dir/http_parser.c.o -c /home/colby/code/school/cs406_chat_room/cpp-driver/src/third_party/http-parser/http_parser.c

src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/http-parser.dir/http_parser.c.i"
	cd /home/colby/code/school/cs406_chat_room/cpp-driver/build/src/third_party/http-parser && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/colby/code/school/cs406_chat_room/cpp-driver/src/third_party/http-parser/http_parser.c > CMakeFiles/http-parser.dir/http_parser.c.i

src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/http-parser.dir/http_parser.c.s"
	cd /home/colby/code/school/cs406_chat_room/cpp-driver/build/src/third_party/http-parser && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/colby/code/school/cs406_chat_room/cpp-driver/src/third_party/http-parser/http_parser.c -o CMakeFiles/http-parser.dir/http_parser.c.s

http-parser: src/third_party/http-parser/CMakeFiles/http-parser.dir/http_parser.c.o
http-parser: src/third_party/http-parser/CMakeFiles/http-parser.dir/build.make
.PHONY : http-parser

# Rule to build all files generated by this target.
src/third_party/http-parser/CMakeFiles/http-parser.dir/build: http-parser
.PHONY : src/third_party/http-parser/CMakeFiles/http-parser.dir/build

src/third_party/http-parser/CMakeFiles/http-parser.dir/clean:
	cd /home/colby/code/school/cs406_chat_room/cpp-driver/build/src/third_party/http-parser && $(CMAKE_COMMAND) -P CMakeFiles/http-parser.dir/cmake_clean.cmake
.PHONY : src/third_party/http-parser/CMakeFiles/http-parser.dir/clean

src/third_party/http-parser/CMakeFiles/http-parser.dir/depend:
	cd /home/colby/code/school/cs406_chat_room/cpp-driver/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/colby/code/school/cs406_chat_room/cpp-driver /home/colby/code/school/cs406_chat_room/cpp-driver/src/third_party/http-parser /home/colby/code/school/cs406_chat_room/cpp-driver/build /home/colby/code/school/cs406_chat_room/cpp-driver/build/src/third_party/http-parser /home/colby/code/school/cs406_chat_room/cpp-driver/build/src/third_party/http-parser/CMakeFiles/http-parser.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/third_party/http-parser/CMakeFiles/http-parser.dir/depend

