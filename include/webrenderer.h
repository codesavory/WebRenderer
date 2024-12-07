#pragma once

#include "renderpass.h"
#include "shader.h"
#include "hello_imgui/hello_imgui.h"

class WebRenderer {
public:
    WebRenderer();
    ~WebRenderer();

    void draw_background();
    void run();
private:
    RenderPass m_render_pass;
    Shader *m_texture_shader = nullptr;
    HelloImGui::RunnerParams m_params;
};
