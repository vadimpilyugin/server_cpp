# Target: MAIN_TARGET(.c|.cpp) -> MAIN_TARGET
MAIN_TARGET = Server_v5

# File extension
FILE_EXT=.cpp

# Libraries: lib(.*).a -> -l(.*)
LIB_FILES = 

# Compiler options
CXX = g++
CXXFLAGS = -O2 -Wall -std=c++11 -Wno-unused-function # -I cpp_config/include

# Directories with source code
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
DEP_DIR = $(BUILD_DIR)/deps
LIBS_DIR = lib
TESTS_DIR = tests

# Add headers dirs to gcc search path
CXXFLAGS += -I $(INCLUDE_DIR)
# Add path with compiled libraries to gcc search path
CXXFLAGS += -L $(LIBS_DIR)
# Link libraries gcc flag: library will be searched with prefix "lib".
LDFLAGS = $(LIB_FILES)

# Helper macros
# subst is sensitive to leading spaces in arguments.
make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
# Takes path list with source files and returns pathes to related objects.
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
# Takes path list with object files and returns pathes to related dep. file.
# Dependency files will be generated with gcc -MM.
src_to_dep = $(call make_path,.d, $(SRC_DIR), $(DEP_DIR), $(1))

# All source files in our project that must be built into movable object code.
CXXFILES := $(wildcard $(SRC_DIR)/*$(FILE_EXT))
OBJFILES := $(call src_to_obj, $(CXXFILES))

# Default target (make without specified target).
.DEFAULT_GOAL := all

# Alias to make all targets.
.PHONY: all 
all:  $(BIN_DIR)/$(MAIN_TARGET)

.PHONY: clear
clear:
	clear; clear; clear;

# Suppress makefile rebuilding.
Makefile: ;

# Generate and include dependency information.
ifneq ($(MAKECMDGOALS), clean)
-include deps.mk
endif

# deps.mk contains redirect to dependency generation
deps.mk: 
	mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR) 
	echo '-include $(call src_to_dep, $(CXXFILES))' >deps.mk

# Rules for compiling targets
$(BIN_DIR)/$(MAIN_TARGET): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(filter %.o, $^)  -o $@ $(LDFLAGS)

# Pattern for generating dependency description files (*.d)
$(DEP_DIR)/%.d: $(SRC_DIR)/%$(FILE_EXT)
	$(CXX) $(CXXFLAGS) -E -MM -MT $(call src_to_obj, $<) -MT $@ -MF $@ $<

# Pattern for compiling object files (*.o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(FILE_EXT)
	$(CXX) $(CXXFLAGS) -c -o $(call src_to_obj, $<) $<

# Fictive target
.PHONY: clean
# Delete all temprorary and binary files
clean:
	rm -rf $(BUILD_DIR)
	rm -f deps.mk

# Additional targers for testing purposes

.PHONY: debug
debug: clear $(BIN_DIR)/$(prog)
	gdb $(BIN_DIR)/$(prog)
	
.PHONY: run
run: clear $(BIN_DIR)/$(prog)
	$(BIN_DIR)/$(prog)

#.PHONY: test
#test: $(BIN_DIR)/$(MAIN_TARGET)
#	make --makefile=tests/Makefile test

# If you still have "WTF?!" feeling, try reading teaching book
# by Mashechkin & Co. http://unicorn.ejudge.ru/instr.pdf
