#include <cstdio>
#include <stdlib.h>


#include "../libs/glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "../libs/imgui/imgui.h"
#include "../libs/imgui/backends/imgui_impl_glfw.h"
#include "../libs/imgui/backends/imgui_impl_opengl3.h"

#include "chip8_ui.h"

#define WINDOW_H 720
#define WINDOW_W 1280

static void glfw_error_cb(int code, const char* desc) {
    fprintf(stderr, "GLFW error %d: %s\n", code, desc);
} 

int main(int argc, char* argv[]) {
    glfwSetErrorCallback(glfw_error_cb);

    // Init GLFW
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(WINDOW_W, WINDOW_H, "ChippyDbg v1.0", NULL, NULL);
    if (!win) {
        fprintf(stderr, "GLFW: Failed to initialize window!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "GLAD: Failed to load OpenGL!\n");
        glfwTerminate();
        return 1;
    }

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;                // No rounded corners on floating windows
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;   // Full opacity background
    }

    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // UI creation
    Chip8UI* ui = chip8_ui_create();
    if (!ui) {
        fprintf(stderr, "Failed to create UI\n"); 
        glfwTerminate(); 
        return 1; 
    }

    if (argc > 1) {
        chip8_ui_load_rom(ui, argv[1]);
    }

    // Main Loop
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        chip8_ui_process_keyboard(ui, win);
        chip8_ui_update(ui);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_dockspace(ui);

        if (ui->exit_requested) {
            glfwSetWindowShouldClose(win, true);
        }

        chip8_ui_render(ui);

        ImGui::Render();

        int fb_w, fb_h;
        glfwGetFramebufferSize(win, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h); 
        glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            // Save current context before messing with other windows
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();      // Update floating windows
            ImGui::RenderPlatformWindowsDefault(); // Render them
            glfwMakeContextCurrent(backup_current_context);  // Restore context
        }

        // Swap buffers (double buffering - show what we just drew)
        glfwSwapBuffers(win);
    }

    // Cleanup

    chip8_ui_destroy(&ui);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup GLFW
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}