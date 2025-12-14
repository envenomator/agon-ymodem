# Detect platform
UNAME_S := $(shell uname -s)

# Compiler
CC := gcc
CXX := g++

# Flags
CFLAGS  := -std=c11 -Wall -Wextra -O2
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
LDFLAGS :=

# OS-specific flags
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -ludev
endif

ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -framework IOKit -framework CoreFoundation
endif

# Automatically find all .c and .cpp files
SRCS_C := $(wildcard *.c)
SRCS_CPP := $(wildcard *.cpp)

# Object files
OBJS_C := $(SRCS_C:.c=.o)
OBJS_CPP := $(SRCS_CPP:.cpp=.o)
OBJS := $(OBJS_C) $(OBJS_CPP)

# Target
TARGET := ymodem

# Default target
all: $(TARGET)

# Link all objects with C++ compiler (needed if any .cpp files exist)
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

