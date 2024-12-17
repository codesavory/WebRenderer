/*
Inspired by the works of Wojciech Jarosz in https://github.com/wkjarosz/SamplinSafari/
*/
#pragma once

#include "linalg.h"

#include <unordered_map>

using namespace linalg::aliases;

// An abstraction fo rendering passes that work with OpenGL and Meta

class RenderPass {
public:
    /**
     * Create a new render pass for rendering to the main color and (optionally) depth buffer.
     *
     * * \param clear
     *     Should \ref enter() begin by clearing all buffers?
     */
    RenderPass(bool clear = true);
    ~RenderPass();
    
    /**
     * Begin the render pass
     *
     * The specified drawing state (e.g. depth tests, culling mode, blending mode) are automatically set up at this
     * point. Later changes between \ref begin() and \ref end() are possible but cause additional OpenGL/GLES/Metal API
     * calls.
     */
    void begin();
    
    // Finish the render pass
    void end();
    
    /// Set the clear color for a given color attachment
    void set_clear_color(const float4 &color);
    
    /// Resize all texture targets attached to the render pass
    void resize(const int2 &size);
    
    // Set the pixel offset and size of the viewport region
    void set_viewport(const int2 &offset, const int2 &size);
    
    /// Return the pixel offset and size of the viewport region
    std::pair<int2, int2> viewport()
    {
        return {m_viewport_offset, m_viewport_size};
    }

protected:
    bool m_clear;
    float4 m_clear_color;
    int2 m_viewport_offset;
    int2 m_viewport_size;
    int2 m_framebuffer_size;
    bool m_active;
    
    // FUTURE: depth buffer, culling, blending and scissor test
    
    //#if defined(HELLOIMGUI_HAS_OPENGL)
    int4 m_viewport_backup;
};
