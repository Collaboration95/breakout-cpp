BUILD_DIR := build
BINARY    := $(BUILD_DIR)/breakout
CMAKE     := cmake
CXX       ?= c++

.PHONY: all run cleanup fresh

# Default target: configure (first time only) then build.
all: $(BINARY)

$(BINARY): $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR)

# Configure step — only runs when build/ doesn't contain a CMake cache yet.
$(BUILD_DIR)/CMakeCache.txt:
	$(CMAKE) -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Debug $(if $(filter-out c++,$(CXX)),-DCMAKE_CXX_COMPILER=$(CXX))


# Build then run the binary.
run: all
	./$(BINARY)

# Remove compiled artifacts but keep the CMake configuration.
# Next `make` skips configure and goes straight to recompile.
cleanup:
	$(CMAKE) --build $(BUILD_DIR) --target clean

# Wipe everything and start completely fresh.
fresh:
	rm -rf $(BUILD_DIR)
	$(MAKE) all
