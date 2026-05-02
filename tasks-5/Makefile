CXX       := g++
CXXFLAGS  := -std=c++17 -pedantic -Wall
LDLIBS    := -lcurl
BUILD_DIR := build

.PHONY: all clean zip

all: $(BUILD_DIR)/site-checker

$(BUILD_DIR)/site-checker: site-checker.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

zip: site-checker.cpp | $(BUILD_DIR)
	zip -j $(BUILD_DIR)/site-checker.zip site-checker.cpp

clean:
	rm -rf $(BUILD_DIR)
