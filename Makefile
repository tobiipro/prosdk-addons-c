CC = gcc
CFLAGS = -Wall -Werror -I$(SDK_DIR)/$(BITNESS)/include
LDFLAGS = -Wl,-rpath='$$ORIGIN'
MKDIR_P = mkdir -p
RM = rm -f

BITNESS = $(shell getconf LONG_BIT)

BUILD_DIR = ./build
SDK_DIR = ./sdk

TARGET_LIB = libtobii_research_addons.so

OBJS = $(BUILD_DIR)/screen_based_calibration_validation.o \
	$(BUILD_DIR)/vectormath.o \
	$(BUILD_DIR)/stopwatch.o

.PHONY: all
all: directories $(BUILD_DIR)/${TARGET_LIB} $(BUILD_DIR)/sample

directories: $(BUILD_DIR)

$(BUILD_DIR):
	@$(MKDIR_P) $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET_LIB): $(OBJS)
	@$(CC) ${LDFLAGS} -shared -o $@ $^
	@cp $(SDK_DIR)/$(BITNESS)/lib/*.* $(BUILD_DIR)

$(BUILD_DIR)/sample: $(BUILD_DIR)/sample.o
	@$(CC) $(LDFLAGS) -L$(BUILD_DIR) -o $@ $^ -ltobii_research_addons -ltobii_research -lm

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
	@${RM} -r $(BUILD_DIR)

