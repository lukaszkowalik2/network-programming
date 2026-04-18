CXX       := g++
CXXFLAGS  := -std=c++17 -pedantic -Wall
BUILD_DIR := build

.PHONY: all clean zip

all: $(BUILD_DIR)/palindrome-server

$(BUILD_DIR)/palindrome-server: palindrome-server.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

zip: palindrome-server.cpp | $(BUILD_DIR)
	zip -j $(BUILD_DIR)/palindrome.zip palindrome-server.cpp

clean:
	rm -rf $(BUILD_DIR)
