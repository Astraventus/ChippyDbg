# ============================================================
# ChippyDbg — Makefile
# ============================================================

# ── Compilers ────────────────────────────────────────────────
CC  = gcc        # for .c files
CXX = g++        # for .cpp files (imgui / cimgui are C++)

# ── Flags ────────────────────────────────────────────────────
CFLAGS   = -Wall -Wextra -Werror -std=c17   -g
CXXFLAGS = -Wall -Wextra         -std=c++17 -g   # imgui doesn't play nice with -Werror

# Defines needed so cimgui_impl knows which backends are compiled in,
# and so imgui_impl_opengl3 uses glad as its OpenGL loader.
DEFINES  = -DCIMGUI_USE_GLFW \
           -DCIMGUI_USE_OPENGL3 \
           -DIMGUI_IMPL_OPENGL_LOADER_GLAD

# ── Include paths ─────────────────────────────────────────────
INCLUDES = -Iinclude \
           -Ilibs/imgui \
           -Ilibs/cimgui \
           -Ilibs/glad/include

# ── Linker flags ─────────────────────────────────────────────
# -lglfw    window + input
# -lGL      OpenGL
# -ldl      needed by glad at runtime (dynamic loading)
# -lm       math
# -lstdc++  C++ runtime (imgui is C++, our binary is C)
LDFLAGS  = -lglfw -lGL -ldl -lm -lstdc++

# ── Target ───────────────────────────────────────────────────
TARGET = chip8

# ── Source files ─────────────────────────────────────────────

# Your C sources
C_SRCS = src/main.c \
         src/chip8.c \
         src/chip8_log.c \
         src/chip8_ui.c \
         libs/glad/src/glad.c

# C++ sources — imgui core + glfw/opengl3 backends + cimgui C wrappers
CXX_SRCS = libs/imgui/imgui.cpp \
           libs/imgui/imgui_draw.cpp \
           libs/imgui/imgui_tables.cpp \
           libs/imgui/imgui_widgets.cpp \
           libs/imgui/imgui_demo.cpp \
           libs/imgui/imgui_impl_glfw.cpp \
           libs/imgui/imgui_impl_opengl3.cpp \
           libs/cimgui/cimgui.cpp \
           libs/cimgui/cimgui_impl.cpp

# ── Object files ─────────────────────────────────────────────
C_OBJS   = $(C_SRCS:.c=.o)
CXX_OBJS = $(CXX_SRCS:.cpp=.o)
OBJS     = $(C_OBJS) $(CXX_OBJS)

# ── Rules ─────────────────────────────────────────────────────

.PHONY: all clean run

all: $(TARGET)

# Link — gcc can link C++ .o files as long as we pass -lstdc++
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile .c → .o
%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<

# Compile .cpp → .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<

# ── Convenience ───────────────────────────────────────────────
run: all
	./$(TARGET) roms/IBM_Logo.ch8

clean:
	rm -f $(TARGET) $(OBJS)