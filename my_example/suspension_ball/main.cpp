#include "global.h"
#include "draw.h"
#include "knobs/imgui-knobs.h"
#include "theme/themes.h"
#include "log/log.h"
#include "log/log_animation.h"
#include "icon.h"
#include "widget.h"
#include <iostream>
#include <windows.h>

ImFont* tab_title;
ImFont* icon;

static int tab = 0;

static float content_animation = 0.0f;
static float dpi_scale = 1.0f;

static char username[64];
static char password[64];

float accent_colour[4] = { 173 / 255.f, 57 / 255.f, 57 / 255.f, 1.f };


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void shut(const char* text, bool* close, float x)
{
    ImGui::SetCursorPos({ x - 170.0f,10.0f });
    if (ImGui::SmallButton(text))
       *close = true;
}

int main()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsClassic();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font = io.Fonts->AddFontFromMemoryTTF((void*)ZKBlackfont_data, ZKBlackfont_size, 25.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    static bool isBall = true;
    static bool isMenu = false;
    static bool open   = false;
    static bool close  = false;
    static bool mem    = true;
    static bool button = true;
    static bool bar = true;

    static float count = 0;
    static float winx      = 120;
    static float winy      = 60;
    static float memory_x  = 600;
    static float memory_y  = 300;

    static ImVec2 Pos = { 0, 0 };
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    //ImGuiWindow* G_window = nullptr;
    static float velocity = 1.f;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiStyle& Style = ImGui::GetStyle();
        auto Color = Style.Colors;

        time_t t = time(0);
        char tmp[32] = { NULL };
        static int Tab = 0;
        const char* text = "-";
        static double s0 = 0.1;
        enum Tab
        {
            Esp,
            Widget,
            Setting,
        };

        static int Theme = 0;
        enum Theme
        {
            black,
            colorful,
            gray,
            red
        };

        switch (Theme)
        {
        case Theme::black:
            ImCandy::Theme_Blender(&Style);
            break;
        case Theme::colorful:
            ImCandy::Theme_Cyberpunk(&Style);
            break;
        case Theme::gray:
            ImCandy::Theme_Nord(&Style);
            break;
        case Theme::red:
            ImCandy::Theme_Red(&Style);
            break;
        }

        if (tab == 0) {
            ImGui::SetNextWindowSize(ImVec2(800 * dpi_scale, 400 * dpi_scale));
            ImGui::Begin("loader", nullptr, ImGuiWindowFlags_NoDecoration);
            {
                auto draw = ImGui::GetWindowDrawList();
                ImVec2 pos = ImGui::GetWindowPos();
                ImVec2 size = ImGui::GetWindowSize();

                draw->AddText(icon, 14.0f, ImVec2(pos.x + 9, pos.y + 4), ImColor(accent_colour[0], accent_colour[1], accent_colour[2]), u8"至简");

                draw->AddLine(ImVec2(pos.x, pos.y + 23), ImVec2(pos.x + size.x, pos.y + 23), ImColor(46, 46, 46)); // upper line
                draw->AddRectFilledMultiColor(ImVec2(pos.x, pos.y + 24), ImVec2(pos.x + size.x, pos.y + 30), ImColor(0, 0, 0), ImColor(0, 0, 0), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0)); // upper 'glow'

                content_animation = ImLerp(content_animation, content_animation < 1.0f ? 1.0f : 0.0f, 0.07f * (1.0f - ImGui::GetIO().DeltaTime));

                draw->AddText(tab_title, 17.0f, ImVec2(pos.x + size.x / 2 - ImGui::CalcTextSize("Bilibili唯我至简").x / 2, pos.y + 45 * content_animation), ImColor(0.8f, 0.8f, 0.8f, content_animation), u8"Bilibili唯我至简");

                ImGui::SetCursorPos(ImVec2(200 * dpi_scale, 85 * content_animation));

                ImGui::BeginChild("log_window", ImVec2(size.x / 2, size.y - 140));
                {
                    ImGui::InputText(u8"用户名", username, sizeof(username));
                    ImGui::InputText(u8"密码", password, sizeof(password), ImGuiInputTextFlags_Password);
                    ImGui::Spacing();
                    ImGui::Spacing();

                    if (ImGui::Button(u8"登录", ImVec2(162 * dpi_scale, 25 * dpi_scale)))
                    {
                        Pos = ImGui::GetWindowPos();
                        ImGui::SetWindowPos("Progress", Pos, 1);
                        tab = 1;
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }
        if (tab == 1)
        {
            ImGui::Begin("Progress", &bar, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::SetWindowSize({ 400, 80 });

            const ImU32 col = ImGui::GetColorU32(ImVec4(0.83f, 0.13f, 0.07f, 1.0f));
            const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

            ImSpinner::SpinnerAngTriple("SpinnerAngTriple", 16, 13, 10, 1.3f, ImColor(255, 255, 255), ImColor(255, 0, 0), ImColor(255, 255, 255), 5 * velocity, 1.5f * IM_PI);
            ImGui::BufferingBar("buffer_bar", count / 400.0f, ImVec2(400, 6), bg, col);
            ImGui::End();
            ++count;
            if (count == 400)
                tab = 2;
        }
        
        if (tab == 2) {
            if (isBall)
            {
                ImGui::SetNextWindowBgAlpha(0.0f);
                ImGui::Begin("Ball", &isBall, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                ImGui::SetWindowSize({ 60, 60 });
                ImSpinner::SpinnerIngYang("my_ball", 16, 5, true, 3.f, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8f);
                if (ImGui::IsClickandDrag()) {
                    isMenu = true;
                    isBall = false;
                    open = false;
                    Pos = ImGui::GetWindowPos();
                    ImGui::SetWindowPos("Menu", Pos, 1);
                }
                ImGui::End();
            }

            if (isMenu)
            {

                ImGui::Begin("Menu", &isMenu, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
                ImGui::SetWindowSize({ winx, winy });
                Pos = ImGui::GetWindowPos();
                ImGui::SetWindowPos("Ball", Pos, 1);

                ImSpinner::SpinnerIngYang("my_ball", 16, 5, true, 3.f, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8f);
                ImGui::SameLine();
                ImGui::Text(u8"至简");

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::Esp ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Button(u8"测试", { 100.0f,40.0f }))
                {
                    Tab = Tab::Esp;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::Widget ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Button(u8"小组件", { 100.0f,40.0f }))
                {
                    Tab = Tab::Widget;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::Setting ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Button(u8"设置", { 100.0f,40.0f }))
                {
                    Tab = Tab::Setting;
                }
                ImGui::PopStyleColor();

                ImGui::SetCursorPos({ 120.0f,10.0f });
                ImGui::NextColumn();
                ImGui::BeginChild("Child", { winx - 130.0f,winy - 20.0f }, true);
                switch (Tab)
                {
                case Tab::Esp:
                    shut(text, &close, winx);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImCandy::Rainbow(s0));
                    if (ImGui::Button("FBI WARNNING ! ! !"))
                        ShellExecute(nullptr, "open", "https:///github.com/dalvqw/Imgui_FloatBall", nullptr, nullptr, SW_SHOWNORMAL);
                    ImGui::PopStyleColor();

                    ImGui::Text(u8"Frames (%.1fFPS)", ImGui::GetIO().Framerate);
                    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M", localtime(&t));
                    ImGui::TextColored(Color[ImGuiCol_Text], "%s", tmp);
                    break;
                case Tab::Widget:
                    shut(text, &close, winx);
                    ToggleButton("button", &button, Color[ImGuiCol_ButtonHovered], Color[ImGuiCol_Button]);

                    ImGuiKnobs::Knob(u8"Bilibili", &winx, 600.0f, 1200.0f, 2.0f, "%.1ff", ImGuiKnobVariant_Space);
                    ImGui::SameLine();
                    ImGuiKnobs::Knob(u8"唯我至简", &winy, 300.0f, 600.0f, 2.0f, "%.1ff", ImGuiKnobVariant_WiperDot);
                    if (mem)
                    {
                        memory_x = winx;
                        memory_y = winy;
                    }
                    break;
                case Tab::Setting:
                    shut(text, &close, winx);

                    ImGui::Text(u8"主题");
                    ImGui::SameLine();
                    ImGui::SetCursorPos({ 74.0f,35.0f });
                    ImGui::SetNextItemWidth(85.0f);
                    ImGui::Combo(u8" ", &Theme, u8"黑色\0炫彩\0灰色\0红白");

                    break;
                }
                ImGui::EndChild();

                if (!open)
                {
                    winx += 60;
                    winy += 30;
                    if (winx >= memory_x)
                    {
                        open = true;
                        mem  = true;
                    }
                }

                if (close)
                {
                    mem = false;
                    winx -= 60;
                    winy -= 30;
                    if (winx <= 120)
                    {
                        close = false;
                        isBall = true;
                        isMenu = false;
                    }
                }

                ImGui::End();
            }
        }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }


    // Cleanup
    ImGui::PopFont();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}