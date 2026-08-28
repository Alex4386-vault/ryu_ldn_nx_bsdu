# pull in common stratosphere sysmodule configuration
THIS_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIRECTORY := $(abspath $(dir $(THIS_MAKEFILE)))
include $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/../Atmosphere-libs/config/templates/stratosphere.mk

TARGET		:=	lsp_bsd_mitm
BUILD		:=	build
SOURCES		:=	source source/config source/debug source/bsd
DATA		:=	data
INCLUDES	:=	source include

CFLAGS		+= -DAMS_IMPL_ENABLE_LOG -DAMS_ENABLE_DETAILED_ASSERTIONS
CXXFLAGS	+= -DAMS_IMPL_ENABLE_LOG -DAMS_ENABLE_DETAILED_ASSERTIONS

ATMOSPHERE_SYSTEM_MODULE_TARGETS := nsp

ifneq ($(__RECURSIVE__),1)

export TOPDIR	:=	$(CURDIR)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

CFILES      :=	$(call FIND_SOURCE_FILES,$(SOURCES),c)
CPPFILES    :=	$(call FIND_SOURCE_FILES,$(SOURCES),cpp)
SFILES      :=	$(call FIND_SOURCE_FILES,$(SOURCES),s)
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			$(foreach dir,$(AMS_LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(ATMOSPHERE_BUILD_DIR)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) $(foreach dir,$(AMS_LIBDIRS),-L$(dir)/$(ATMOSPHERE_LIBRARY_DIR))
export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)
export APP_JSON := $(TOPDIR)/res/app.json

.PHONY: clean all check_lib

all: $(ATMOSPHERE_OUT_DIR) $(ATMOSPHERE_BUILD_DIR) $(ATMOSPHERE_LIBRARIES_DIR)/libstratosphere/$(ATMOSPHERE_LIBRARY_DIR)/libstratosphere.a
	@$(MAKE) __RECURSIVE__=1 OUTPUT=$(CURDIR)/$(ATMOSPHERE_OUT_DIR)/$(TARGET) \
	DEPSDIR=$(CURDIR)/$(ATMOSPHERE_BUILD_DIR) \
	--no-print-directory -C $(ATMOSPHERE_BUILD_DIR) \
	-f $(THIS_MAKEFILE)

$(ATMOSPHERE_LIBRARIES_DIR)/libstratosphere/$(ATMOSPHERE_LIBRARY_DIR)/libstratosphere.a: check_lib
	@$(SILENTCMD)echo "Checked library."

ifeq ($(ATMOSPHERE_CHECKED_LIBSTRATOSPHERE),1)
check_lib:
else
check_lib:
	@$(MAKE) --no-print-directory -C $(ATMOSPHERE_LIBRARIES_DIR)/libstratosphere -f $(ATMOSPHERE_LIBRARIES_DIR)/libstratosphere/libstratosphere.mk
endif

$(ATMOSPHERE_OUT_DIR) $(ATMOSPHERE_BUILD_DIR):
	@[ -d $@ ] || mkdir -p $@

clean:
	@echo clean ...
	@rm -fr $(ATMOSPHERE_OUT_DIR) $(ATMOSPHERE_BUILD_DIR)

else

.PHONY:	all
DEPENDS	:=	$(OFILES:.o=.d)

all	:	$(foreach target,$(ATMOSPHERE_SYSTEM_MODULE_TARGETS),$(OUTPUT).$(target))

$(OUTPUT).nsp :	$(OUTPUT).nso $(OUTPUT).npdm
$(OUTPUT).nso :	$(OUTPUT).elf

$(OUTPUT).elf :	$(OFILES)

$(OFILES) : $(ATMOSPHERE_LIBRARIES_DIR)/libstratosphere/$(ATMOSPHERE_LIBRARY_DIR)/libstratosphere.a

%.npdm  :   $(APP_JSON)
	@echo built ... $< $@
	@npdmtool $< $@
	@echo built ... $(notdir $@)

-include $(DEPENDS)

endif
