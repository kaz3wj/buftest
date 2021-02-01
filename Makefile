# Makefile for Basler pylon sample program
.PHONY: all clean

# The program to build
NAME       := buftest

OBJ_DIR		= ./obj
SRC_DIRS	= .


SRCS		= $(shell find $(SRC_DIRS) -name "*.cpp")
OBJS		= $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS		= $(OBJS:.o=.d)

# Installation directories for pylon
PYLON_ROOT ?= /opt/pylon



# Build tools and flags
LD         := $(CXX)


EXTERNAL_CFLAGS :=
EXTERNAL_LIBS :=

EXTERNAL_CFLAGS += $(shell pkg-config --cflags cudart-10.2)
EXTERNAL_LIBS += $(shell pkg-config --libs cudart-10.2)
EXTERNAL_CFLAGS += $(shell pkg-config --cflags visionworks)
EXTERNAL_LIBS += $(shell pkg-config --libs visionworks)

EXTERNAL_CFLAGS += -I/usr/include/opencv4

CPPFLAGS   += $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
CPPFLAGS   += $(EXTERNAL_CFLAGS)
CXXFLAGS   := -pthread 
#e.g., CXXFLAGS=-g -O0 for debugging

LDFLAGS    := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs)
LDLIBS		+= -L/usr/include/opencv4 -lopencv_calib3d -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_features2d
LDLIBS		+= -lpthread -lz -lX11
LDLIBS		+= -L/usr/local/cuda-10.2/targets/aarch64-linux/lib -lcudart
LDFLAGS		+= $(EXTERNAL_LIBS)

# Rules for building
all: $(NAME)

$(NAME): $(OBJS)
	$(MKDIR_P) $(dir $@)
	$(LD) $(LDFLAGS) -o $(NAME) $^ $(LDLIBS)

#	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# c source
$(OBJ_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
 
# c++ source
$(OBJ_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(OBJ_DIR) $(NAME)

MKDIR_P ?= mkdir -p
