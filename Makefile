CC = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  BREW_PREFIX := $(shell brew --prefix)
  CPPFLAGS = -I$(BREW_PREFIX)/include -DGL_SILENCE_DEPRECATION
  LDFLAGS  = -L$(BREW_PREFIX)/lib
  LIBS     = -lglfw -lGLEW -framework OpenGL
else
  CPPFLAGS = -I/usr/local/include
  LDFLAGS  = -L/usr/local/lib
  LIBS     = -lglfw -lGLEW -lGL
endif

BIN = a2-animate-minifig
SRCS = src/texture.cpp src/animation.cpp src/main.cpp src/gl_context.cpp src/scene.cpp src/minifig.cpp \
       src/node.cpp src/geometry.cpp src/shader_util.cpp
INCLUDES = src/animation.hpp src/texture.hpp src/gl_context.hpp src/scene.hpp src/minifig.hpp \
           src/node.hpp src/geometry.hpp src/shader_util.hpp 

all: $(BIN)

$(BIN): $(SRCS) $(INCLUDES)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $(SRCS) -o $(BIN) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(BIN) *.o

.PHONY: all clean test

test:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -Isrc tests/animation_test.cpp src/animation.cpp -o /tmp/cs675-a2-animation-test
	/tmp/cs675-a2-animation-test
