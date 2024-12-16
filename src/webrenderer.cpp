#include "webrenderer.h"
#include "shader.h"
#include "renderpass.h"

#include "imgui_ext.h"
#include "imgui_internal.h"

#include <cmath>
#include <fmt/core.h>
#include <fstream>
#include <utility>

using std::pair;
using std::to_string;

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
                { 0.0f,  0.5f, 0.0f},
                { 0.5f, -0.5f, 0.0f},
                {-0.5f, -0.5f, 0.0f}
            };
            
            // Upload the vertex data to the shader buffer
            m_triangle_shader->set_buffer("position", vertices);
            
            HelloImGui::Log(HelloImGui::LogLevel::Info, "Successfully initialized GL!");
            
        } catch (const std::exception &e)
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
    auto &io = ImGui::GetIO();
    try
    {
        //
        // calculate the viewport sizes
        // fbsize is the size of the window in pixels while accounting for dpi factor on retina screens.
        // for retina displays, io.DisplaySize is the size of the window in points (logical pixels)
        // but we need the size in pixels. So we scale io.DisplaySize by io.DisplayFramebufferScale
        int2 fbscale         = io.DisplayFramebufferScale;
        auto fbsize          = int2{io.DisplaySize} * fbscale;
        int2 viewport_offset = {0, 0};
        int2 viewport_size   = io.DisplaySize;
        if (auto id = m_params.dockingParams.dockSpaceIdFromName("MainDockSpace"))
        {
            auto central_node = ImGui::DockBuilderGetCentralNode(*id);
            viewport_size     = int2{int(central_node->Size.x), int(central_node->Size.y)};
            viewport_offset   = int2{int(central_node->Pos.x), int(central_node->Pos.y)};
        }
        
        m_render_pass.set_viewport(viewport_offset * fbscale, viewport_size * fbscale);

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
    } catch (const std::exception &e)
    {
        fmt::print(stderr, "OpenGL drawing failed:\n\t{}.", e.what());
        HelloImGui::Log(HelloImGui::LogLevel::Error, "OpenGL drawing failed:\n\t%s.", e.what());
    }
}

void WebRenderer::run() {
    // Setup HelloImGui runner
    HelloImGui::Run(m_params);
}
