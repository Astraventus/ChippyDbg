CXX     = g++
CC      = gcc
CXXFLAGS = -Wall -Wextra -std=c++17 -g
CFLAGS   = -Wall -Wextra -std=c11   -g
LDFLAGS  = -lglfw -lGL -ldl -lm
DEFINES  = -DIMGUI_IMPL_OPENGL_LOADER_GLAD
INCLUDES = -Iinclude \
           -Ilibs/imgui \
           -Ilibs/imgui/backends \
           -Ilibs/glad/include \
           -Ilibs/tinyfiledialogs

BUILD_DIR = build

# ── Sources ──────────────────────────────────────────────────
C_SRCS = \
    src/chip8.c \
    src/chip8_log.c \
    libs/glad/src/glad.c \
    libs/tinyfiledialogs/tinyfiledialogs.c

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
C_OBJS   = $(patsubst %.c,   $(BUILD_DIR)/%.o, $(C_SRCS))
CXX_OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CXX_SRCS))
TARGET   = $(BUILD_DIR)/chip8dbg

# ── Rules ────────────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(C_OBJS) $(CXX_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean