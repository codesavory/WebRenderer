#include "webrenderer.h"
#include "shader.h"
#include "renderpass.h"
#include "hello_imgui/hello_imgui.h"

#include <fmt/core.h>

WebRenderer::WebRenderer() {
    // Initialize HelloImGui parameters (for example, UI window dimensions)
    m_params.appWindowParams.windowTitle = "WebRenderer";
    m_params.appWindowParams.windowGeometry.size = {800, 600};
    
    m_params.callbacks.PostInit = [this]()
    {
        try
        {
            // Create a Shader (example: vertex and fragment shader files need to be valid)
            m_triangle_shader = new Shader(
                                           &m_render_pass,
                                           "TriangleShader",
                                           "shaders/triangle.vert",
                                           "shaders/triangle.frag");
            // Define the triangle vertices
            // Example vertex buffer data (triangle vertices in normalized device coordinates)
            std::vector<float3> vertices = {
                {-0.5f, -0.5f, 0.0f},
                { 0.5f, -0.5f, 0.0f},
                { 0.0f,  0.5f, 0.0f}
            };
            
            // Upload the vertex data to the shader buffer
            m_triangle_shader->set_buffer("vertices", vertices);
        }
        
        catch (const std::exception &e)
        {
            fmt::print(stderr, "Shader initialization failed!:\n\t{}.", e.what());
            HelloImGui::Log(HelloImGui::LogLevel::Error, "Shader initialization failed!:\n\t%s.", e.what());
        }
    };
    
    // Setup other initialization here if needed
    m_params.callbacks.CustomBackground        = [this]() { draw_background(); };
}

WebRenderer::~WebRenderer() {
    // Cleanup
    if (m_triangle_shader) {
        delete m_triangle_shader;
        m_triangle_shader = nullptr;
    }
}

void WebRenderer::draw_background() {
    // Start render pass
    m_render_pass.begin();

    // Set up shader (for example, use a simple color or texture for the background)
    m_triangle_shader->begin();

    // Example drawing operation: draw a triangle
    m_triangle_shader->draw_array(Shader::PrimitiveType::Triangle, 0, 3);

    // End shader usage
    m_triangle_shader->end();

    // End render pass
    m_render_pass.end();
}

void WebRenderer::run() {
    // Setup HelloImGui runner
    HelloImGui::Run(m_params);
}
