CXX     = g++
CC      = gcc
CXXFLAGS = -Wall -Wextra -std=c++17 -g
CFLAGS   = -Wall -Wextra -std=c11   -g
LDFLAGS  = -lglfw -lGL -ldl -lm

DEFINES  = -DIMGUI_IMPL_OPENGL_LOADER_GLAD

INCLUDES = -Iinclude \
           -Ilibs/imgui \
           -Ilibs/imgui/backends \
           -Ilibs/glad/include

# ── Sources ──────────────────────────────────────────────────

C_SRCS = \
    src/chip8.c \
    src/chip8_log.c \
    libs/glad/src/glad.c

CXX_SRCS = \
    src/main.cpp \
    src/chip8_ui.cpp \
    libs/imgui/imgui.cpp \
    libs/imgui/imgui_draw.cpp \
    libs/imgui/imgui_tables.cpp \
    libs/imgui/imgui_widgets.cpp \
    libs/imgui/imgui_demo.cpp \
    libs/imgui/backends/imgui_impl_glfw.cpp \
    libs/imgui/backends/imgui_impl_opengl3.cpp

# ── Object files ─────────────────────────────────────────────

C_OBJS   = $(C_SRCS:.c=.o)
CXX_OBJS = $(CXX_SRCS:.cpp=.o)

TARGET = chip8dbg

# ── Rules ────────────────────────────────────────────────────

all: $(TARGET)

$(TARGET): $(C_OBJS) $(CXX_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(C_OBJS) $(CXX_OBJS) $(TARGET)

.PHONY: all run clean