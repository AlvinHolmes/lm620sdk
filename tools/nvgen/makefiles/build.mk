#===========================================================================
# Generic Makefile for C/C++ Program
#
# Author:   
#
# Version:  
#===========================================================================

.PHONY : all clean install

MKDIR   = mkdir -p
CP      = cp -a

CC      = $(CROSS)gcc
LD      = $(CROSS)ld
AR      = $(CROSS)ar
NM      = $(CROSS)nm
RANLIB  = $(CROSS)ranlib
STRIP   = $(CROSS)strip

ifeq ($(DEBUG),1)
DBGTYPE = debug
else
DBGTYPE = release
endif

ifneq ($(CROSS),)
empty =
space = $(empty) $(empty)
CRSCMP = $(subst -, $(space), $(CROSS))
#ARCH = $(word 1, $(CRSCMP))
ARCH = $(CROSS)$(DBGTYPE)
else
ARCH = $(DBGTYPE)
endif

# source
PRJ_ROOT ?= $(shell pwd)
SRC_DIR := $(PRJ_ROOT)/src
SRC_SUBDIR := $(shell find $(SRC_DIR) -maxdepth 3 -type d)

# dest
BUILD_DIR := $(PRJ_ROOT)/build
DEP_DIR := $(BUILD_DIR)/$(ARCH)/deps
OBJ_DIR := $(BUILD_DIR)/$(ARCH)/obj
LIB_DIR := $(BUILD_DIR)/$(ARCH)/lib
BIN_DIR := $(BUILD_DIR)/$(ARCH)/bin

# directory
DIRS := $(OBJ_DIR) $(DEP_DIR)

# compile & link options
AFLAGS += -cr
ifeq ($(DEBUG),1)
CFLAGS += -O0 -g
else
CFLAGS += -O2
LDFLAGS += -s
endif
#CWARN_FLAGS += -Wno-unused-parameter -Wno-unused-result -Wno-missing-braces -Wno-switch
CFLAGS += -Wall -W -D_LIBC_REENTRANT
CFLAGS += $(CWARN_FLAGS)
CXXFLAGS += $(CFLAGS)
CXXFLAGS += $(CWARN_FLAGS)
CXXFLAGS += $(CXXWARN_FLAGS)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-rpath=.:lib:../lib
# LNK_LIBS += -lpthread

ifeq ($(PROJECT),)
    PROJECT := $(notdir $(PRJ_ROOT))
endif

ifeq ($(TGTTYPE),SLIB)
    DIRS += $(LIB_DIR)
    TARGET := lib$(PROJECT).a
    FINAL := $(LIB_DIR)/$(TARGET)
else ifeq ($(TGTTYPE),DLIB)
    DIRS += $(LIB_DIR)
    TARGET := lib$(PROJECT).so
    FINAL := $(LIB_DIR)/$(TARGET)
else
    DIRS += $(BIN_DIR)
    TARGET := $(PROJECT)
    FINAL := $(BIN_DIR)/$(TARGET)
endif

#add SRC_SUBDIR to VPATH
VPATH += $(SRC_SUBDIR)

#generate source files list
SRCEXTS := .c .cpp
SRCS := $(foreach d,$(SRC_SUBDIR),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))

#generate object files list
OBJS := $(foreach x,$(SRCEXTS),$(patsubst %$(x),%.o,$(filter %$(x),$(SRCS))))
OBJS := $(notdir $(OBJS))
OBJS := $(addprefix $(OBJ_DIR)/,$(OBJS))

#generate dependence files list
DEPS = $(foreach x,$(SRCEXTS),$(patsubst %$(x),%.d,$(filter %$(x),$(SRCS))))
DEPS := $(notdir $(DEPS))
DEPS := $(addprefix $(DEP_DIR)/,$(DEPS))

#save make files list before including $(DEPS)
DEPMKS := $(MAKEFILE_LIST)

ifeq ("$(wildcard $(DEP_DIR))", "")
D_DEP_DIR := $(DEP_DIR)
endif

all : $(DIRS) $(FINAL)

clean :
	rm -rf $(BUILD_DIR)

install : all
ifeq ($(TGTTYPE),EXE)
ifneq ($(IST_DIR),)
	$(MKDIR) $(IST_DIR)
	$(CP) $(BUILD_DIR)/$(ARCH)/bin/$(TARGET) $(IST_DIR)/
endif
else
ifneq ($(IST_DIR),)
	$(MKDIR) $(IST_DIR)
	$(MKDIR) $(IST_DIR)/$(ARCH)/inc
	$(CP) $(PRJ_ROOT)/inc/* $(IST_DIR)/$(ARCH)/inc
	$(MKDIR) $(IST_DIR)/$(ARCH)/lib
	$(CP) $(BUILD_DIR)/$(ARCH)/lib/* $(IST_DIR)/$(ARCH)/lib
endif
endif
	
# include dependent files
ifeq ($(MAKECMDGOALS),)
sinclude $(DEPS)
endif

ifeq ($(MAKECMDGOALS), all)
sinclude $(DEPS)
endif

ifeq ($(MAKECMDGOALS), install)
sinclude $(DEPS)
endif

# define function to generate dependent files
define gen_dep
set -e; rm -f $@; \
$(CC) -MM $(INC_DIR) $(CFLAGS) $< > $@.$$$$; \
sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
rm -f $@.$$$$
endef

# creat object file, use static regular ?
$(OBJ_DIR)/%.o : %.c $(DEPMKS)
#	@echo "source files:" $< $(DEPMKS)
#	@echo "object files:" $@
ifeq ($(TGTTYPE),SLIB)
	$(CC) $(INC_DIR) $(CFLAGS) -o $@ -c $<
else
	$(CC) $(INC_DIR) $(CFLAGS) -fPIC -o $@ -c $<
endif

$(OBJ_DIR)/%.o : %.cpp $(DEPMKS)
#	@echo "source files:" $< $(DEPMKS)
#	@echo "object files:" $@
ifeq ($(TGTTYPE),SLIB)
	$(CC) $(INC_DIR) $(CXXFLAGS) -o $@ -c $<
else
	$(CC) $(INC_DIR) $(CXXFLAGS) -fPIC -o $@ -c $<
endif

# create depandant file, use static regular ?
$(DEP_DIR)/%.d : %.c $(D_DEP_DIR)
	@echo "creating depend file ..." $@
	@$(gen_dep)

$(DEP_DIR)/%.d : %.cpp $(D_DEP_DIR)
	@echo "creating depend file ..." $@
	@$(gen_dep)
		
$(DIRS) :
	$(MKDIR) $(DIRS)

$(FINAL) : $(OBJS) $(DEPS)
ifeq ($(TGTTYPE),SLIB)
	$(AR) $(AFLAGS) $@ $(OBJS)
else ifeq ($(TGTTYPE),DLIB)
	$(CC) -shared -o $@ $(OBJS) $(LDFLAGS) $(LDIR_LNK) $(LNK_LIBS)
else
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS_PATH) $(LNK_LIBS)
endif
