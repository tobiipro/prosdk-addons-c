UNAME:=$(shell uname -s)
ifeq ($(UNAME), Linux)
	OS=LINUX
	LIB_EXT=so
	CC:=gcc
	LINKER:=g++
else ifeq ($(UNAME), Darwin)
	OS=OSX
	LIB_EXT=dylib
	CC:=clang
	CXX:=clang++
else
	$(error "Operating System not supported.")
endif

BITNESS=$(shell getconf LONG_BIT)

MKDIR_P=mkdir -p
RM=rm -f

BUILD_DIR=./build
SDK_DIR=./sdk

CFLAGS=-Wall -Werror -I$(SDK_DIR)/$(BITNESS)/include
LDFLAGS_LINUX=-Wl,-rpath='$$ORIGIN'
LDFLAGS_OSX=-m$(BITNESS) -Wl,-rpath,@executable_path -Wl,-L$(SDK_DIR)/$(BITNESS)/lib

LDFLAGS_$(OS)+=-ltobii_research -lm

TARGET_LIB=libtobii_research_addons.$(LIB_EXT)

OBJS=$(BUILD_DIR)/screen_based_calibration_validation.o \
	$(BUILD_DIR)/vectormath.o \
	$(BUILD_DIR)/stopwatch.o

.PHONY: all
all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET_LIB) $(BUILD_DIR)/sample

$(BUILD_DIR):
	@$(MKDIR_P) $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET_LIB): $(OBJS)
	@$(CC) $(LDFLAGS_$(OS)) -shared -o $@ $^
	@cp $(SDK_DIR)/$(BITNESS)/lib/*.* $(BUILD_DIR)

$(BUILD_DIR)/sample: $(BUILD_DIR)/sample.o
	@$(CC) $(LDFLAGS_$(OS)) -L$(BUILD_DIR) -o $@ $^ -ltobii_research_addons

$(BUILD_DIR)/sample.o: source/sample.c source/screen_based_calibration_validation.h
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/screen_based_calibration_validation.o: source/screen_based_calibration_validation.c source/screen_based_calibration_validation.h
	@$(CC) -c -fPIC $(CFLAGS) $< -o $@

$(BUILD_DIR)/vectormath.o: source/vectormath.c source/vectormath.h
	@$(CC) -c -fPIC $(CFLAGS) $< -o $@

$(BUILD_DIR)/stopwatch.o: source/stopwatch.c source/stopwatch.h
	@$(CC) -c -fPIC $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@$(RM) -r $(BUILD_DIR)


%:
	@echo "Ignoring $@ target in C Addons Makefile"