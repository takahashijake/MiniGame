BUILD_DIR ?= build
BUILD_TYPE ?= Debug

.PHONY: all configure build run test clean format check-format

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) --parallel

run: build
	$(BUILD_DIR)/bin/minigame

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	cmake -E remove_directory $(BUILD_DIR)

format:
	clang-format -i $$(find include src tests -type f \( -name '*.hpp' -o -name '*.cpp' \))

check-format:
	clang-format --dry-run --Werror $$(find include src tests -type f \( -name '*.hpp' -o -name '*.cpp' \))
