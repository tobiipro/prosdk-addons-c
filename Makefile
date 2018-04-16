C_BINDING_SOURCES:=source/screen_based_calibration_validation.c \
	source/vectormath.c \
	source/stopwatch.c

C_BINDING_INTERFACE_FILES:=source/screen_based_calibration_validation.h \
	source/vectormath.h \
	source/stopwatch.h

TOBII_RESEARCH_LIB:=tobii_research_addons

OS=Windows

C_BINDING_OBJECTS:=$(C_BINDING_SOURCES:%.c=$(OBJECTSDIR)/%.o)

INCLUDE_PATHS:=	$(STAGE)/CRelease$(BITNESS)

DIRS+=$(OBJECTSDIR)
DIRS+=$(OUTPUT)
CLEANABLE_DIRS+=$(OBJECTSDIR)
CLEANABLE_DIRS+=$(wildcard $(STAGE)/C*$(BITNESS))
CLEANABLE_DIRS+=$(wildcard $(STAGE)/c_*$(BITNESS))

LN:=ln
# Library version
LIB_VER=$(versionmajor).$(versionminor).$(versionrelease)

TOBII_RESEARCH_LIB_NAME_Windows:=$(OUTPUT)/$(TOBII_RESEARCH_LIB).dll

INCLUDES:= $(addprefix -I, $(INCLUDE_PATHS))

CFLAGS_Windows:= /c /DTOBII_EXPORTING /Wall /wd4255 /wd4820 /WX /D_CRT_SECURE_NO_WARNINGS /EHsc /nologo
LDFLAGS_Windows+= /nologo

C_BINDING_HEADERS:=$(addprefix $(OUTPUT)/, $(notdir $(C_BINDING_INTERFACE_FILES)))

c_addons: release

%:
	@echo "Ignoring $@ target in the source Makefile"


$(OBJECTSDIR)/%.o:%.c
	@$(CC) $(CFLAGS) $(INCLUDES) $< $(C_OUTPUT_$(OS))$@

$(OUTPUT)/%.h:include$(PATH_SEPARATOR)%.h
	@$(CP) "$<" "$@"


C_BINDING_LIB=$(STAGE)/CRelease$(BITNESS)

$(C_BINDING_DEP): $(C_BINDING_LIB)
	@echo "copying $(filter %$(PATH_SEPARATOR)$(@F), $+) to $@"
	@$(CP) "$(filter %$(PATH_SEPARATOR)$(@F), $+)" "$@"

$(TOBII_RESEARCH_LIB_NAME_Windows):$(C_BINDING_HEADERS)
$(TOBII_RESEARCH_LIB_NAME_Windows):CFLAGS+=$(CFLAGS_Windows)
$(TOBII_RESEARCH_LIB_NAME_Windows):$(DIRS) $(C_BINDING_OBJECTS) $(C_BINDING_DEP)
	@echo "Linking $@"
	@$(LINKER) $(filter %.res, $+) $(filter %.o, $+) $(LDFLAGS_$(OS)) $(LD_OUTPUT_$(OS))"$@"

$(DIRS):
	@echo "Creating $@"
	@$(MKDIR) "$@"

.PHONY: c_addons_clean
c_addons_clean:
	@echo "Cleaning $(CLEANABLE_DIRS)"
ifeq ($(OS), Windows)
	$(foreach dir, $(wildcard $(CLEANABLE_DIRS)), rmdir /S /Q "$(dir)" &)
else
	@$(foreach dir, $(CLEANABLE_DIRS), $(RM) "$(dir)")
endif

.PHONY: release
release: print $(TOBII_RESEARCH_LIB_NAME_$(OS)) c_addons_cpplint
	@echo "Building source..." $(TOBII_RESEARCH_LIB_NAME_$(OS))

print:
	@echo "OBJECTSDIR":$(OBJECTSDIR)
	@echo "C_BINDING_OBJECTS":$(C_BINDING_OBJECTS)

.PHONY: c_addons_cpplint
c_addons_cpplint:
	@echo "CppLint"
	@$(PYTHON) -m cpplint $(SOURCES) $(C_BINDING_SOURCES) $(C_BINDING_INTERFACE_FILES)
