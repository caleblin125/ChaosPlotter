# Compiler
CXX = mpic++
CC  = mpicc

CXXFLAGS = -std=c++17 -O3 -Iinclude
CFLAGS   = -O3 -Iinclude
LDFLAGS = -lglfw -lGL -lm

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
CPP_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
C_SRCS   = $(wildcard $(SRC_DIR)/*.c)

SRCS = $(CPP_SRCS) $(C_SRCS)

# Object files
CPP_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS))
C_OBJS   = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))

OBJS = $(CPP_OBJS) $(C_OBJS)

# Output binary
TARGET = $(BUILD_DIR)/chaosplotter

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile C++
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Run
run: all
	./$(TARGET)