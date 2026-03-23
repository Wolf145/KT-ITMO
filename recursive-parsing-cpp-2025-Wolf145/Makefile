EXECUTABLE_NAME := recursive-parsing
BUILD_DIR := build
BUILD_TYPE := Debug

.PHONY: all build clean rebuild run
.SILENT: all build clean rebuild run
.ONESHELL:

all: rebuild

build:
	cmake . -D CMAKE_BUILD_TYPE=$(BUILD_TYPE) -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR) && ln -sfv $(BUILD_DIR)/$(EXECUTABLE_NAME) $(EXECUTABLE_NAME)

clean:
	rm -fv $(EXECUTABLE_NAME)
	rm -rfv $(BUILD_DIR)
	rm -fv *.png *.dot

rebuild: clean build

run: $(EXECUTABLE_NAME)
	./$(EXECUTABLE_NAME) || exit 255
	test -f tree.dot && dot tree.dot -Tpng > tree.png || true
