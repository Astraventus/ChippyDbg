#include "../include/chip8_ui.h"

#include <cstdint>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === PRIVATE FUNCTIONS ===

static GLuint create_display_texture() {
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 0, GL_RGBA,  GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

static void upload_display_texture(Chip8UI* ui) {
    const bool* display = chip8_get_display(ui->chip);
    if (!display) return;

    static uint8_t pixels[CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT * 4];
    for (int i = 0; i < CHIP8_DISPLAY_HEIGHT * CHIP8_DISPLAY_WIDTH; i++) {
        uint8_t v = display[i] ? 0xFF : 0x00;
        pixels[i*4+0] = v;
        pixels[i*4+1] = v;
        pixels[i*4+2] = v;
        pixels[i*4+3] = 0xFF;
    }
    glBindTexture(GL_TEXTURE_2D, ui->display_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void open_rom_dialog(Chip8UI* ui) {
    const char* filter_patterns[] = { "*.ch8", "*.c8", "*.rom" };
    const char* path = tinyfd_openFileDialog(
        "Load CHIP-8 ROM", "", 3, filter_patterns, "CHIP-8 ROM files", 0
    );
    if (path) chip8_ui_load_rom(ui, path);
}

static void render_controls(Chip8UI* ui) {
    if (!ui || !ui->show_controls) return;

    ImGui::Begin("Controls", &ui->show_controls, ImGuiWindowFlags_AlwaysAutoResize);

    // ROM
    ImGui::SeparatorText("ROM");

    if (ImGui::Button("Load ROM", ImVec2(160, 0))) {
        open_rom_dialog(ui);
    }

    ImGui::SameLine(0, 8);

    bool has_rom = (ui->chip != nullptr);
    if (!has_rom) ImGui::BeginDisabled();

    if (ImGui::Button("Reload", ImVec2(80, 0))) {
        chip8_ui_load_rom(ui, ui->rom_path);
    }

    ImGui::SameLine(0, 8);

    if (ImGui::Button("Close", ImVec2(80, 0))) {
        chip8_ui_close_rom(ui);
    }

    if (!has_rom) ImGui::EndDisabled();

    if (has_rom) {
        ImGui::Text("Loaded: %s", ui->rom_path);
    } else {
        ImGui::TextDisabled("No ROM loaded");
    }

    // Execution
    ImGui::SeparatorText("Execution");

    if (!has_rom) ImGui::BeginDisabled();

    const char* run_label = ui->running ? "Pause##run" : "Run##run";
    if (ImGui::Button(run_label, ImVec2(80, 0))) {
        ui->running = !ui->running;
    }

    ImGui::SameLine(0, 8);

    if (ImGui::Button("Step", ImVec2(80, 0))) {
        ui->running = false;
        chip8_step(ui->chip);
    }

    ImGui::SameLine(0, 8);

    char step_n_label[32];
    snprintf(step_n_label, sizeof(step_n_label), "Step %d##stepn", ui->step_count);
    if (ImGui::Button(step_n_label, ImVec2(80, 0))) {
        ui->running = false;
        chip8_step_n(ui->chip, ui->step_count);
    }
    ImGui::SameLine(0, 8);
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("##step_count", &ui->step_count, 1, 10);

    if (!has_rom) ImGui::EndDisabled();

    // Status
    ImGui::SeparatorText("Status");

    if (has_rom) {
        const char* status;
        if (chip8_is_halted(ui->chip))  status = "HALTED";
        else if (ui->running)           status = "running";
        else                            status = "paused";

        ImGui::Text("State: %s", status);
        ImGui::Text("Cycles: %llu", (unsigned long long)chip8_get_cycle_count(ui->chip));
        ImGui::Text("Speed: %d cycles/frame", ui->cycles_per_frame);
        ImGui::SetNextItemWidth(160);
        ImGui::SliderInt("##speed", &ui->cycles_per_frame, 1, 100, "%d cycles/frame");
    } else {
        ImGui::TextDisabled("Load a ROM to begin");
    }

    ImGui::End();
}

static void render_cpu_state(Chip8UI* ui) {
    if (!ui || !ui->chip || !ui->show_cpu_state) return;

    ImGui::Begin("CPU State", &ui->show_cpu_state);

    ImGui::SeparatorText("Registers");
    ImGui::Text("PC : 0x%03X (%d)", chip8_get_pc(ui->chip), chip8_get_pc(ui->chip));
    ImGui::Text("I  : 0x%03X (%d)", chip8_get_i(ui->chip), chip8_get_i(ui->chip));
    ImGui::Text("SP : 0x%03X (%d)", chip8_get_sp(ui->chip), chip8_get_sp(ui->chip));

    ImGui::SeparatorText("Timers");
    ImGui::Text("Delay  : %d", chip8_get_delay_timer(ui->chip));
    ImGui::Text("Sound  : %d", chip8_get_sound_timer(ui->chip));

    ImGui::SeparatorText("V0-VF");
    ImGui::Columns(4, "vregs", true);
    ImGui::Text("Reg"); ImGui::NextColumn();
    ImGui::Text("Hex"); ImGui::NextColumn();
    ImGui::Text("Dec"); ImGui::NextColumn();
    ImGui::Text("Bin"); ImGui::NextColumn();
    ImGui::Separator();

    for (int i = 0; i < CHIP8_NUM_REGISTERS; i++) {
        uint8_t v = chip8_get_register(ui->chip, i);

        ImGui::Text("V%0X", i); ImGui::NextColumn();
        ImGui::Text("0x%02X", v); ImGui::NextColumn();
        ImGui::Text("%d", v); ImGui::NextColumn();
        ImGui::Text("%c%c%c%c%c%c%c%c",
            (v&0x80)?'1':'0',(v&0x40)?'1':'0',
            (v&0x20)?'1':'0',(v&0x10)?'1':'0',
            (v&0x08)?'1':'0',(v&0x04)?'1':'0',
            (v&0x02)?'1':'0',(v&0x01)?'1':'0'
        ); ImGui::NextColumn();
    }

    ImGui::Columns(1, nullptr, false);

    ImGui::SeparatorText("Call Stack");
    uint8_t sp = chip8_get_sp(ui->chip);
    if (sp == 0) {
        ImGui::TextDisabled("(Empty)");
    } else {
        for (int i = sp - 1; i >= 0; i--) {
            ImGui::Text("[%d] 0x%03X", i, chip8_get_stack(ui->chip, i));
        }
    }

    ImGui::End();
}


static void render_display(Chip8UI* ui) {
    if (!ui->show_display) return;

    ImGui::Begin("Display", &ui->show_display, 0);

    if (!ui->chip) {
        ImGui::TextDisabled("No ROM Loaded");
        ImGui::End();
        return;
    }

    if (chip8_should_draw(ui->chip)) {
        upload_display_texture(ui);
    }

    ImVec2 available = ImGui::GetContentRegionAvail();

    // Maintain 2:1 aspect ratio (64x32)
    float aspect = (float)CHIP8_DISPLAY_WIDTH / (float)CHIP8_DISPLAY_HEIGHT;
    float w = available.x;
    float h = w / aspect;

    if (h > available.y) {
        h = available.y;
        w = h * aspect;
    }

    // Center horizontally
    float offset_x = (available.x - w) * 0.5f;
    if (offset_x > 0)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

    ImGui::Image((ImTextureID)(intptr_t)ui->display_texture, ImVec2(w, h),
                 ImVec2(0, 0), ImVec2(1, 1));

    ImGui::End();
}

static void render_memory(Chip8UI* ui) {
    if (!ui || !ui->chip || !ui->show_memory) return;

    ImGui::Begin("Memory", &ui->show_memory);

    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("Cols##mem", &ui->memory_cols, 1, 16, "%d bytes/row");

    const uint8_t* mem = chip8_get_memory(ui->chip);
    uint16_t pc = chip8_get_pc(ui->chip);

    ImGui::BeginChild("MemScroll");

    int cols = ui->memory_cols;
    int rows = CHIP8_MEMORY_SIZE / cols;

    for (int r = 0; r < rows; r++) {
        int base = r * cols;
        
        bool is_pc_row = (pc >= base && pc < base + cols);

        if (is_pc_row) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        ImGui::Text("0x%03X |", base);
        ImGui::SameLine(0, 2);

        for (int c = 0; c < cols; c++) {
            uint16_t addr = (uint16_t)(base+c);

            if (pc == addr) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ">%02X", mem[addr]);
            } else {
                ImGui::Text("%02X", mem[addr]);
            }
            ImGui::SameLine(0, 2);
        }

        ImGui::Text("| ");
        ImGui::SameLine(0, 0);
        for (int c = 0; c < cols; c++)
        {
            uint8_t byte = mem[base + c];
            ImGui::Text("%c", (byte >= 32 && byte < 127) ? byte : '.');
            ImGui::SameLine(0, 0);
        }
        ImGui::NewLine();

        if (is_pc_row) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

static void render_disassembly(Chip8UI* ui) {
    if (!ui->show_disassembly || !ui->chip) return;

    ImGui::Begin("Disassembly", &ui->show_disassembly);
    ImGui::Checkbox("Follow PC", &ui->follow_pc);

    ImGui::BeginChild("DisasmScroll");

    uint16_t pc = chip8_get_pc(ui->chip);

    for (int addr = 0x200; addr < CHIP8_MEMORY_SIZE - 1; addr += 2) {
        bool is_pc = ((uint16_t)addr == pc);

        if (is_pc) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            if (ui->follow_pc)
                ImGui::SetScrollHereY(0.5f);
        }

        uint16_t opcode = chip8_read_opcode(ui->chip, (uint16_t)addr);
        char disasm[64];
        chip8_disassemble(ui->chip, (uint16_t)addr, disasm, sizeof(disasm));

        ImGui::Text("%c 0x%03X | %04X | %s",
                    is_pc ? '>' : ' ', addr, opcode, disasm);

        if (is_pc)
            ImGui::PopStyleColor();
    }

    ImGui::EndChild();
    ImGui::End();
}

static void render_keyboard(Chip8UI* ui) {
    if (!ui->show_keyboard) return;

    ImGui::Begin("Keyboard", &ui->show_keyboard,
                 ImGuiWindowFlags_AlwaysAutoResize);

    if (!ui->chip) {
        ImGui::TextDisabled("No ROM loaded");
        ImGui::End();
        return;
    }

    // CHIP-8 keypad layout
    static const uint8_t layout[4][4] = {
        {0x1, 0x2, 0x3, 0xC},
        {0x4, 0x5, 0x6, 0xD},
        {0x7, 0x8, 0x9, 0xE},
        {0xA, 0x0, 0xB, 0xF},
    };

    ImVec2 btn(55, 55);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            uint8_t key = layout[row][col];
            if (col > 0) ImGui::SameLine(0, 4);

            bool pressed = chip8_is_key_pressed(ui->chip, key);
            if (pressed)
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      ImVec4(0.2f, 0.7f, 0.2f, 1.0f));

            char label[8];
            snprintf(label, sizeof(label), "%X##k%X", key, key);
            ImGui::Button(label, btn);

            if (ImGui::IsItemActivated())   chip8_key_press(ui->chip, key);
            if (ImGui::IsItemDeactivated()) chip8_key_release(ui->chip, key);

            if (pressed) ImGui::PopStyleColor();
        }
    }

    ImGui::End();
}

// === PUBLIC API ===

Chip8UI* chip8_ui_create() {
    
    Chip8UI* ui = (Chip8UI*)calloc(1, sizeof(Chip8UI));

    if (!ui) {
        fprintf(stderr, "chip8_ui_create: Allocation failed!\n");
        return nullptr;
    }

    ui->chip = nullptr;
    ui->running = false;

    ui->display_texture = create_display_texture();
    ui->display_scale = 10.0f;

    ui->cycles_per_frame = 10;
    ui->step_count = 10;

    ui->memory_cols = 8;
    ui->follow_pc = true;

    ui->show_controls = true;
    ui->show_cpu_state = true;
    ui->show_display = true;
    ui->show_disassembly = true;
    ui->show_keyboard = true;
    ui->show_memory = true;

    return ui;
}

void chip8_ui_destroy(Chip8UI** ui_ptr) {
    if (!*ui_ptr || !ui_ptr) return;

    Chip8UI* ui = *ui_ptr;
    chip8_ui_close_rom(ui);
    glDeleteTextures(1, &ui->display_texture);
    free(ui);
    *ui_ptr = nullptr;
}

bool chip8_ui_load_rom(Chip8UI* ui, const char* path) {
    if (!ui || !path) return false;

    char path_copy[256];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    chip8_ui_close_rom(ui);

    ui->chip = chip8_create();
    if (!ui->chip) {
        fprintf(stderr, "chip8_ui_load_rom: failed to create chip8 instance\n");
        return false;
    }

    if (!chip8_load_rom(ui->chip, path_copy)) {
        fprintf(stderr, "chip8_ui_load_rom: Failed to load ROM into chip8 instance\n");
        chip8_destroy(&ui->chip);
        return false;
    }

    strncpy(ui->rom_path, path_copy, sizeof(ui->rom_path) - 1);
    ui->rom_path[sizeof(ui->rom_path) - 1] = '\0';
    ui->running = false;
    return true;
}

void chip8_ui_close_rom(Chip8UI* ui) {
    if (!ui) return;

    if (ui->chip) {
        chip8_destroy(&ui->chip);
    }
    ui->running = false;
    ui->rom_path[0] = '\0';
}

void chip8_ui_update(Chip8UI* ui) {
    if (!ui || !ui->chip || !ui->running) return;
    if (chip8_is_halted(ui->chip)) {return; }

    for (int i = 0; i < ui->cycles_per_frame; i++) {
        if (!chip8_step(ui->chip)) break;
    }

    chip8_update_timers(ui->chip);
}

void chip8_ui_render(Chip8UI* ui) {
    if (!ui) return;
    render_controls(ui);
    render_display(ui);
    render_cpu_state(ui);
    render_memory(ui);
    render_disassembly(ui);
    render_keyboard(ui);
}

void chip8_ui_process_keyboard(Chip8UI* ui, GLFWwindow* window) {
    if (!ui || !ui->chip) return;

    static const struct { int glfw; uint8_t chip8; } map[] = {
        { GLFW_KEY_1, 0x1 }, { GLFW_KEY_2, 0x2 },
        { GLFW_KEY_3, 0x3 }, { GLFW_KEY_4, 0xC },
        { GLFW_KEY_Q, 0x4 }, { GLFW_KEY_W, 0x5 },
        { GLFW_KEY_E, 0x6 }, { GLFW_KEY_R, 0xD },
        { GLFW_KEY_A, 0x7 }, { GLFW_KEY_S, 0x8 },
        { GLFW_KEY_D, 0x9 }, { GLFW_KEY_F, 0xE },
        { GLFW_KEY_Z, 0xA }, { GLFW_KEY_X, 0x0 },
        { GLFW_KEY_C, 0xB }, { GLFW_KEY_V, 0xF },
    };

    for (int i = 0; i < 16; i++) {
        if (glfwGetKey(window, map[i].glfw) == GLFW_PRESS)
            chip8_key_press(ui->chip,   map[i].chip8);
        else
            chip8_key_release(ui->chip, map[i].chip8);
    }
}

void render_dockspace(Chip8UI* ui) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Menu bar inside dockspace
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM...", "Ctrl+O")) {
                open_rom_dialog(ui);
            }
            if (ImGui::MenuItem("Close ROM", "Ctrl+W", false, ui->chip != nullptr)) {
                chip8_ui_close_rom(ui);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
                ui->exit_requested = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Controls",    nullptr, &ui->show_controls);
            ImGui::MenuItem("Display",     nullptr, &ui->show_display);
            ImGui::MenuItem("CPU State",   nullptr, &ui->show_cpu_state);
            ImGui::MenuItem("Memory",      nullptr, &ui->show_memory);
            ImGui::MenuItem("Disassembly", nullptr, &ui->show_disassembly);
            ImGui::MenuItem("Keyboard",    nullptr, &ui->show_keyboard);
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }

    // Create dockspace
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End();
}