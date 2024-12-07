#include "shader.h"

Shader::Shader(RenderPass *render_pass, const std::string &name, const std::string &vs_filename,
               const std::string &fs_filename) :
    m_render_pass(render_pass),
    m_name(name) {
}