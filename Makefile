BUILD_DIR ?= build
WEB_BUILD_DIR ?= build-web
CMAKE ?= cmake

.PHONY: all configure build test run web clean

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DMINIGAME_BUILD_TESTS=ON

build: configure
	$(CMAKE) --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./$(BUILD_DIR)/minigame

web:
	emcmake $(CMAKE) -S . -B $(WEB_BUILD_DIR) -DMINIGAME_BUILD_TESTS=OFF
	$(CMAKE) --build $(WEB_BUILD_DIR) --target minigame_web
	@echo "Web bundle: $(WEB_BUILD_DIR)/web-dist"

clean:
	$(CMAKE) -E remove_directory $(BUILD_DIR)
	$(CMAKE) -E remove_directory $(WEB_BUILD_DIR)
