COMPONENT ?= declarativeservices_test_bundles

SCM_SRC += ISpellCheckService.cpp
APPNAME := libmwISpellCheckService

ifndef DEBUG
# No need to create native projects for this test module in BaT
DISABLE_CREATE_VCXPROJ := 1
DISABLE_CREATE_XCODEPROJ := 1
endif

include $(MAKE_INCLUDE_DIR)/Makefile.module