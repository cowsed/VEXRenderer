# VEXcode mkenv.mk 2019_06_06_01

# macros to help with windows paths that include spaces
sp :=
sp +=
qs = $(subst ?,$(sp),$1)
sq = $(subst $(sp),?,$1)

# default platform and build location
PLATFORM  = sim_vexv5
BUILD     = build

# version for clang headers
ifneq ("$(origin HEADERS)", "command line")
HEADERS = 8.0.0
endif

# Project name passed from app
ifeq ("$(origin P)", "command line")
PROJECT   = $(P)
else
PROJECT   = $(notdir $(call sq,$(abspath ${CURDIR})))
endif

# Toolchain path passed from app
# need to specify T on command line
#ifeq ("$(origin T)", "command line")
#TOOLCHAIN = $(T)
#endif
TOOLCHAIN = /home/richie/VEX/Sim/sdk


# Verbose flag passed from app
ifeq ("$(origin V)", "command line")
BUILD_VERBOSE=$(V)
endif

# allow verbose to be set by makefile if not set by app
ifndef BUILD_VERBOSE
ifndef VERBOSE
BUILD_VERBOSE = 0
else
BUILD_VERBOSE = $(VERBOSE)
endif
endif

# use verbose flag
ifeq ($(BUILD_VERBOSE),0)
Q = @
else
Q =
endif

# compile and link tools
CC      = clang-15
CXX     = clang-15
OBJCOPY = objcopy
SIZE    = size
LINK    = /usr/bin/ld
ARCH    = ar
ECHO    = @echo
DEFINES = -DSimVexV5

# platform specific macros
ifeq ($(OS),Windows_NT)
$(info windows build for platform $(PLATFORM))
SHELL = cmd.exe
MKDIR = md "$(@D)" 2> nul || :
RMDIR = rmdir /S /Q
CLEAN = $(RMDIR) $(BUILD) 2> nul || :
else
$(info unix build for platform $(PLATFORM))
MKDIR = mkdir -p "$(@D)" 2> /dev/null || :
RMDIR = rm -rf
CLEAN = $(RMDIR) $(BUILD) 2> /dev/null || :
endif

# toolchain include and lib locations
#LIBRARY_PATH=: /usr/lib/../lib/ /usr/lib/gcc/x86_64-pc-linux-gnu/12.2.1/../../../:/lib/:

#TOOL_INC  =  -I/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../include/c++/12.2.1 -I/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../include/c++/12.2.1/x86_64-pc-linux-gnu -I/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../include/c++/12.2.1/backward -I/usr/lib/clang/15.0.7/include
#TOOL_LIB  = -L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1 -L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/lib -L/usr/lib

#TOOL_INC =  -I/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../include/c++/12.2.1 -I/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../include/c++/12.2.1/x86_64-pc-linux-gnu -I/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../include/c++/12.2.1/backward -I/usr/lib/clang/15.0.7/include -I/usr/local/include -I/usr/include


# compiler flags
CFLAGS_CL =  -ggdb 
CFLAGS    = ${CFLAGS_CL} --std=gnu++20
# -Wall -Werror=return-type
# $(DEFINES)
CXX_FLAGS = ${CFLAGS_CL} $(DEFINES) -stdlib=libstdc++

# linker flags
LNK_FLAGS = -pie --build-id --eh-frame-hdr -m elf_x86_64 
# future statuc library
PROJECTLIB = lib$(PROJECT)
ARCH_FLAGS = rcs

# libraries
LIBS =  --start-group -lsimv5rt -lstdc++ -lc -lm -lgcc --end-group -lpthread -dynamic-linker /lib64/ld-linux-x86-64.so.2 /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../lib64/Scrt1.o /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../lib64/crti.o /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/crtbeginS.o -L/home/richie/VEX/Sim/sdk/sim_vexv5/ -L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1 -L/usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/lib -L/usr/lib -lsimv5rt -lstdc++ -lc -lm -lgcc -lglfw -lGLEW -lGL -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/crtendS.o /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/12.2.1/../../../../lib64/crtn.o

# include file paths
INC += $(addprefix -I, ${INC_F})
INC += "-I$(TOOLCHAIN)/$(PLATFORM)/include/"
INC += "-I$(TOOLCHAIN)/$(PLATFORM)/vendor/imgui"
INC += "-I$(TOOLCHAIN)/$(PLATFORM)/vendor/imgui/backends"
INC+= -include "$(TOOLCHAIN)/$(PLATFORM)/otherInclude/replacement.h"
INC += ${TOOL_INC}
