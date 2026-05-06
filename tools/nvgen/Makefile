# ========================== custom zone start =========================================================
PRJ_ROOT := .
IST_DIR ?= $(PRJ_ROOT)/_install

# comments the following line for buidling a debug project
DEBUG ?= 1

# header path
MODEM_PRJDIR ?= $(PRJ_ROOT)/../../project/mdl
COMPONETS_DIR := $(PRJ_ROOT)/../../components

ifeq ($(wildcard src/nvmodem/),)
# SDK版本
INC_DIR += -I$(PRJ_ROOT)/inc
INC_DIR += -I$(MODEM_PRJDIR)/nv
INC_DIR += -I../../open/config/inc
INC_DIR += -I$(MODEM_PRJDIR)/inc
INC_DIR += -I$(MODEM_DIR)
INC_DIR += -I$(MODEM_DIR)/nv
else
MODEM_DIR := ../../modem
INC_DIR += -I$(PRJ_ROOT)/inc
INC_DIR += -I$(MODEM_PRJDIR)/nv
INC_DIR += -I../../open/config/inc
INC_DIR += -I$(MODEM_DIR)/pub/inc
INC_DIR += -I$(MODEM_DIR)/pub/inc/nv
INC_DIR += -I$(MODEM_DIR)/phy
INC_DIR += -I$(MODEM_DIR)/phy/top/inc
INC_DIR += -I$(MODEM_DIR)/ps
INC_DIR += -I$(MODEM_DIR)/ps/src/com/inc
INC_DIR += -I$(COMPONETS_DIR)/kernel/include
endif

# library path & libraries
#LIBS_PATH += -L$(PRJ_ROOT)/lib
#LNK_LIBS += -lpthread

# build type: SLIB(static library), DLIB(dynamic library), EXE(executable file)
TGTTYPE := EXE

# target name: using current directory name as target name if not set;
PROJECT := nvgen

# build flags

# check if src/nvmodem exist
ifeq (0, $(shell if [ -d "$(PRJ_ROOT)/src/nvmodem" ]; then echo "1"; else echo "0"; fi;))
    CFLAGS += -DBUILD_IN_SDK
endif

# open/close compile warning
CWARN_FLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-unused-result -Wno-unknown-pragmas #-Wno-missing-braces -Wno-switch
CWARN_FLAGS += -Wno-format
# ========================== custom zone end =========================================================

include $(PRJ_ROOT)/makefiles/build.mk
