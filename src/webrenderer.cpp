#include "webrenderer.h"
#include "shader.h"
#include "renderpass.h"
#include "hello_imgui/hello_imgui.h"
#include <iostream>

WebRenderer::WebRenderer()
    : m_render_pass(),
      m_triangle_shader(nullptr)
{
    // Initialize the shader with dummy vertex and fragment shader file paths.
    m_triangle_shader = new Shader(
        &m_render_pass,
        "TriangleShader",
        "shaders/triangle.vert",
        "shaders/triangle.frag");

    // Configure HelloImGui runner parameters
    m_params.appWindowParams.windowTitle = "WebRenderer";
    m_params.appWindowParams.windowGeometry.size = {800, 600};
    m_params.callbacks.ShowGui = [this]() { draw_background(); };

    std::cout << "WebRenderer initialized." << std::endl;
}

WebRenderer::~WebRenderer()
{
    //delete m_triangle_shader;
    //m_triangle_shader = nullptr;

    std::cout << "WebRenderer destroyed." << std::endl;
}

void WebRenderer::draw_background()
{
    // Set up the render pass for background rendering
    m_render_pass.begin();

    // Clear with a solid color
    m_render_pass.set_viewport({0, 0}, {800, 600});
    m_render_pass.end();

    // Use the shader to draw a simple triangle
    m_triangle_shader->begin();

    // Example vertex buffer data (triangle vertices in normalized device coordinates)
    std::vector<float3> vertices = {
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.0f,  0.5f, 0.0f}
    };

    m_triangle_shader->set_buffer("positions", vertices);
    m_triangle_shader->draw_array(Shader::PrimitiveType::Triangle, 0, vertices.size());

    m_triangle_shader->end();

    std::cout << "Background drawn." << std::endl;
}

void WebRenderer::run()
{
    HelloImGui::Run(m_params);

    std::cout << "WebRenderer running." << std::endl;
}
