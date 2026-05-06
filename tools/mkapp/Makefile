# ========================== custom zone start =========================================================
PRJ_ROOT := .
IST_DIR ?= $(PRJ_ROOT)/_install

# comments the following line for buidling a debug project
DEBUG ?= 1

# header path

INC_DIR += -I$(PRJ_ROOT)/src/inc

# library path & libraries
#LIBS_PATH += -L$(PRJ_ROOT)/lib
#LNK_LIBS += -lpthread

# build type: SLIB(static library), DLIB(dynamic library), EXE(executable file)
TGTTYPE := EXE

# target name: using current directory name as target name if not set;
PROJECT := mkapp

# build flags


# open/close compile warning
CWARN_FLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-unused-result -Wno-unknown-pragmas #-Wno-missing-braces -Wno-switch
CWARN_FLAGS += -Wno-format
# ========================== custom zone end =========================================================

include $(PRJ_ROOT)/makefiles/build.mk
