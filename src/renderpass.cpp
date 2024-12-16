#include "hello_imgui/hello_imgui_include_opengl.h" // cross-platform way to include OpenGL headers
#include "opengl_check.h"
#include "renderpass.h"

#include <fmt/core.h>
#include <stdexcept>

RenderPass::RenderPass(bool clear) : m_clear(clear), m_clear_color(0, 0, 0, 0), m_viewport_offset(0), m_viewport_size(0), m_framebuffer_size(0), m_active(false) {
    m_viewport_size = m_framebuffer_size = int2{0, 0};
}

RenderPass::~RenderPass() {
    
}

void RenderPass::begin() {
    m_active = true;
    
    // Save the current viewport
    CHK(glGetIntegerv(GL_VIEWPORT, &m_viewport_backup[0]));
    
    // Set the viewport for rendering
    set_viewport(m_viewport_offset, m_viewport_size);
    
    // clear the color buffer
    CHK(glClearColor(m_clear_color.x, m_clear_color.y, m_clear_color.z, m_clear_color.w));
    CHK(glClear(GL_COLOR_BUFFER_BIT));
    
}

void RenderPass::end() {
    // Restore the original viewport
    CHK(glViewport(m_viewport_backup[0], m_viewport_backup[1], m_viewport_backup[2], m_viewport_backup[3]));
    
    m_active = false;
}

void RenderPass::set_viewport(const int2 &offset, const int2 &size) {
    m_viewport_offset = offset;
    m_viewport_size = size;
    
    if(m_active) {
        int ypos = m_framebuffer_size.y - m_viewport_size.y - m_viewport_offset.y;
        CHK(glViewport(m_viewport_offset.x, ypos, m_viewport_size.x, m_viewport_size.y));
    }
}
