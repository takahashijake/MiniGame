# Define the C++ compiler
CXX = g++

# Define C++ compiler flags
# -std=c++14: Specifies the C++14 standard
# -Wall: Enables all common warnings
# -Wextra: Enables extra warnings (more than -Wall)
# -pedantic: Issues warnings for non-standard C++
# -g: Includes debugging information
CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic -g

# Define the name of the executable
TARGET = a.out

# Define the source files
SRCS = main.cpp input_utils.cpp characters.cpp gameState.cpp

# Automatically generate the list of object files from the source files
# (e.g., main.cpp -> main.o)
OBJS = $(SRCS:.cpp=.o)

# The default target. When you run 'make', this target will be built.
# It depends on the executable being up-to-date.
all: $(TARGET)

# Rule to link the object files into the final executable.
# It depends on all the object files being compiled.
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Generic rule for compiling .cpp files into .o object files.
# This rule will be used for each .cpp file in the SRCS list.
# $<: The name of the prerequisite (the .cpp file)
# $@: The name of the target (the .o file)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Phony targets are targets that do not correspond to actual files.
# This prevents make from getting confused if a file named 'clean' or 'all' exists.
.PHONY: all clean

# Rule to clean up generated files (object files and the executable).
# To run this, type 'make clean' in your terminal.
clean:
	rm -f $(OBJS) $(TARGET)

run:
	./a.out