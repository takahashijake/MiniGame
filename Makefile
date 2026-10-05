BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all configure build test run clean

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DMINIGAME_BUILD_TESTS=ON

build: configure
	$(CMAKE) --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./$(BUILD_DIR)/minigame

clean:
	$(CMAKE) -E remove_directory $(BUILD_DIR)
