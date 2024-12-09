//#if defined(HELLOIMGUI_HAS_OPENGL)

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h" // cross-platform way to include OpenGL headers
#include "opengl_check.h"
#include "shader.h"

#include <fmt/core.h>

using std::string;

static GLuint compile_gl_shader(GLenum type, const std::string &name, const std::string& shader_string) {
    if (shader_string.empty())
        return (GLuint)0;
    
    GLuint id;
    CHK(id = glCreateShader(type));
    const GLchar* files[] = {
#ifdef __ESCRIPTEN__
        "version 300 es\n",
#else
        "version 330 core\n",
#endif
        shader_string.c_str()
    };
    
    CHK(glShaderSource(id, 2, files, nullptr));
    CHK(glCompileShader(id));
    
    GLint status;
    CHK(glGetShaderiv(id, GL_COMPILE_STATUS, &status));
    
    if(status!=GL_TRUE) {
        const char* type_str = nullptr;
        if (type == GL_VERTEX_SHADER)
            type_str = "vertex shader";
        else if (type == GL_FRAGMENT_SHADER)
            type_str = "fragment shader";
#ifndef __EMSCRIPTEN__
        else if (type == GL_GEOMETRY_SHADER)
            type_str = "geometry shader";
#endif
        else
            type_str = "unknown shader type";
        
        char error_shader[4096];
        CHK(glGetShaderInfoLog(id, sizeof(error_shader), nullptr, error_shader));
        
        string msg = string("compler_gl_shader(): unable to compile shader") + type_str + "\"" + name + "\":\n\n" + error_shader;
        throw std::runtime_error(msg);
    }
    
    return id;
}

Shader::Shader(RenderPass *render_pass, const std::string &name, const std::string &vs_filename,
               const std::string &fs_filename) :
    m_render_pass(render_pass),
    m_name(name) {

    string vertex_shader, fragment_shader;

    // Load shader files
    auto load_shader_file = [](const string &filename) {
        auto shader_txt = HelloImGui::LoadAssetFileData(filename.c_str());
        if(shader_txt.data == nullptr)
            throw std::runtime_error(fmt::format("Cannot load shader from file \"{}\"", filename));
        return string((char*)shader_txt.data, shader_txt.dataSize);
    };
        
    vertex_shader = load_shader_file(vs_filename);
    fragment_shader = load_shader_file(fs_filename);
    
    // Compile shaders
    GLuint vertex_shader_handle = compile_gl_shader(GL_VERTEX_SHADER, name, vertex_shader);
    GLuint fragment_shader_handle = compile_gl_shader(GL_FRAGMENT_SHADER, name, fragment_shader);
    
    // Create shader program
    m_shader_handle = glCreateProgram();
    glAttachShader(m_shader_handle, vertex_shader_handle);
    glAttachShader(m_shader_handle, fragment_shader_handle);
    glLinkProgram(m_shader_handle);
    
    // Clean up shader program
    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);
    
    // Check for linking errors
    GLint status;
    glGetProgramiv(m_shader_handle, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char error_shader[4096];
        glGetProgramInfoLog(m_shader_handle, sizeof(error_shader), nullptr, error_shader);
        m_shader_handle = 0;
        throw std::runtime_error("Shader::Shader(name=\"" + name + "\"): unable to link shader!\n\n" + error_shader);
    }
    
    // Register buffer for "indices"
    Buffer &buf = m_buffers["indices"];
    buf.index = -1;
    buf.ndim = 1;
    buf.shape[0] = 0;
    buf.shape[1] = buf.shape[2] = 1;
    buf.type = IndexBuffer;
    buf.dtype = VariableType::UInt32;
}

Shader::~Shader() {
    CHK(glDeleteProgram(m_shader_handle));
}

// TODO: learn this code: begin(), end(), draw_array(), set_buffer()
void Shader::set_buffer(const std::string &name, VariableType dtype, size_t ndim, const size_t *shape, const void *data)
{
    auto it = m_buffers.find(name);
    if (it == m_buffers.end())
        throw std::runtime_error("Shader::set_buffer(): could not find argument named \"" + name + "\"");

    Buffer &buf = m_buffers[name];

    bool mismatch = ndim != buf.ndim || dtype != buf.dtype;
    for (size_t i = (buf.type == UniformBuffer ? 0 : 1); i < ndim; ++i)
        mismatch |= shape[i] != buf.shape[i];

    if (mismatch)
    {
        Buffer arg;
        arg.type = buf.type;
        arg.ndim = ndim;
        for (size_t i = 0; i < 3; ++i)
            arg.shape[i] = i < arg.ndim ? shape[i] : 1;
        arg.dtype = dtype;
        throw std::runtime_error("Buffer::set_buffer(\"" + name + "\"): shape/dtype mismatch: expected " +
                                 buf.to_string() + ", got " + arg.to_string());
    }

    size_t size = type_size(dtype);
    for (size_t i = 0; i < 3; ++i)
    {
        buf.shape[i] = i < ndim ? shape[i] : 1;
        size *= buf.shape[i];
    }

    if (buf.type == UniformBuffer)
    {
        if (buf.buffer && buf.size != size)
        {
            delete[] (uint8_t *)buf.buffer;
            buf.buffer = nullptr;
        }
        if (!buf.buffer)
            buf.buffer = new uint8_t[size];
        memcpy(buf.buffer, data, size);
    }
    else
    {
        GLuint buffer_id = 0;
        if (buf.buffer)
        {
            buffer_id = (GLuint)((uintptr_t)buf.buffer);
        }
        else
        {
            CHK(glGenBuffers(1, &buffer_id));
            buf.buffer = (void *)((uintptr_t)buffer_id);
        }
        GLenum buf_type = (name == "indices") ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        CHK(glBindBuffer(buf_type, buffer_id));
        CHK(glBufferData(buf_type, size, data, GL_DYNAMIC_DRAW));
    }

    buf.dtype = dtype;
    buf.ndim  = ndim;
    buf.size  = size;
    buf.dirty = true;
}

void Shader::begin()
{
    int texture_unit = 0;

    CHK(glUseProgram(m_shader_handle));
    for (auto &[key, buf] : m_buffers)
    {
        bool indices = key == "indices";
        if (!buf.buffer)
        {
            if (!indices)
                fprintf(stderr,
                        "Shader::begin(): shader \"%s\" has an unbound "
                        "argument \"%s\"!\n",
                        m_name.c_str(), key.c_str());
            continue;
        }

        GLuint buffer_id = (GLuint)((uintptr_t)buf.buffer);
        GLenum gl_type   = 0;

        bool uniform_error = false;
        switch (buf.type)
        {
        case IndexBuffer: CHK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id)); break;

        case VertexBuffer:
            CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_id));
            CHK(glEnableVertexAttribArray(buf.index));

            switch (buf.dtype)
            {
            case VariableType::Int8: gl_type = GL_BYTE; break;
            case VariableType::UInt8: gl_type = GL_UNSIGNED_BYTE; break;
            case VariableType::Int16: gl_type = GL_SHORT; break;
            case VariableType::UInt16: gl_type = GL_UNSIGNED_SHORT; break;
            case VariableType::Int32: gl_type = GL_INT; break;
            case VariableType::UInt32: gl_type = GL_UNSIGNED_INT; break;
            case VariableType::Float16: gl_type = GL_HALF_FLOAT; break;
            case VariableType::Float32: gl_type = GL_FLOAT; break;
            default: throw std::runtime_error("Shader::begin(): unsupported vertex buffer type!");
            }

            if (buf.ndim != 2)
                throw std::runtime_error("\"" + m_name + "\": vertex attribute \"" + key +
                                         "\" has an invalid shapeension (expected ndim=2, got " +
                                         std::to_string(buf.ndim) + ")");

            CHK(glVertexAttribPointer(buf.index, (GLint)buf.shape[1], gl_type, GL_FALSE, 0,
                                      (const void *)buf.pointer_offset));
            CHK(glVertexAttribDivisor(buf.index, (GLuint)buf.instance_divisor));
            break;

        case VertexTexture:
        case FragmentTexture:
            CHK(glActiveTexture(GL_TEXTURE0 + texture_unit));
            CHK(glBindTexture(GL_TEXTURE_2D, (GLuint)((uintptr_t)buf.buffer)));
            if (buf.dirty)
                CHK(glUniform1i(buf.index, texture_unit));
            texture_unit++;
            break;

        case UniformBuffer:
            if (buf.ndim > 2)
                throw std::runtime_error("\"" + m_name + "\": uniform attribute \"" + key +
                                         "\" has an invalid shape (expected ndim=0/1/2, got " +
                                         std::to_string(buf.ndim) + ")");
            switch (buf.dtype)
            {
            case VariableType::Float32:
                if (buf.ndim < 2)
                {
                    const float *v = (const float *)buf.buffer;
                    switch (buf.shape[0])
                    {
                    case 1: CHK(glUniform1f(buf.index, v[0])); break;
                    case 2: CHK(glUniform2f(buf.index, v[0], v[1])); break;
                    case 3: CHK(glUniform3f(buf.index, v[0], v[1], v[2])); break;
                    case 4: CHK(glUniform4f(buf.index, v[0], v[1], v[2], v[3])); break;
                    default: uniform_error = true; break;
                    }
                }
                else if (buf.ndim == 2 && buf.shape[0] == buf.shape[1])
                {
                    const float *v = (const float *)buf.buffer;
                    switch (buf.shape[0])
                    {
                    case 2: CHK(glUniformMatrix2fv(buf.index, 1, GL_FALSE, v)); break;
                    case 3: CHK(glUniformMatrix3fv(buf.index, 1, GL_FALSE, v)); break;
                    case 4: CHK(glUniformMatrix4fv(buf.index, 1, GL_FALSE, v)); break;
                    default: uniform_error = true; break;
                    }
                }
                else
                {
                    uniform_error = true;
                }
                break;

#if defined(HELLOIMGUI_USE_GLES2) || defined(HELLOIMGUI_USE_GLES3)
            case VariableType::UInt32:
#endif
            case VariableType::Int32:
            {
                const int32_t *v = (const int32_t *)buf.buffer;
                if (buf.ndim < 2)
                {
                    switch (buf.shape[0])
                    {
                    case 1: CHK(glUniform1i(buf.index, v[0])); break;
                    case 2: CHK(glUniform2i(buf.index, v[0], v[1])); break;
                    case 3: CHK(glUniform3i(buf.index, v[0], v[1], v[2])); break;
                    case 4: CHK(glUniform4i(buf.index, v[0], v[1], v[2], v[3])); break;
                    default: uniform_error = true; break;
                    }
                }
                else
                {
                    uniform_error = true;
                }
            }
            break;

#if !defined(HELLOIMGUI_USE_GLES2) && !defined(HELLOIMGUI_USE_GLES3)
            case VariableType::UInt32:
            {
                const uint32_t *v = (const uint32_t *)buf.buffer;
                if (buf.ndim < 2)
                {
                    switch (buf.shape[0])
                    {
                    case 1: CHK(glUniform1ui(buf.index, v[0])); break;
                    case 2: CHK(glUniform2ui(buf.index, v[0], v[1])); break;
                    case 3: CHK(glUniform3ui(buf.index, v[0], v[1], v[2])); break;
                    case 4: CHK(glUniform4ui(buf.index, v[0], v[1], v[2], v[3])); break;
                    default: uniform_error = true; break;
                    }
                }
                else
                {
                    uniform_error = true;
                }
            }
            break;
#endif

            case VariableType::Bool:
            {
                const uint8_t *v = (const uint8_t *)buf.buffer;
                if (buf.ndim < 2)
                {
                    switch (buf.shape[0])
                    {
                    case 1: CHK(glUniform1i(buf.index, v[0])); break;
                    case 2: CHK(glUniform2i(buf.index, v[0], v[1])); break;
                    case 3: CHK(glUniform3i(buf.index, v[0], v[1], v[2])); break;
                    case 4: CHK(glUniform4i(buf.index, v[0], v[1], v[2], v[3])); break;
                    default: uniform_error = true; break;
                    }
                }
                else
                {
                    uniform_error = true;
                }
            }
            break;

            default: uniform_error = true; break;
            }

            if (uniform_error)
                throw std::runtime_error("\"" + m_name + "\": uniform attribute \"" + key +
                                         "\" has an unsupported dtype/shape configuration: " + buf.to_string());
            break;

        default:
            throw std::runtime_error("\"" + m_name + "\": uniform attribute \"" + key +
                                     "\" has an unsupported dtype/shape configuration:" + buf.to_string());
        }

        buf.dirty = false;
    }

/*#ifndef __EMSCRIPTEN__
    if (m_uses_point_size)
        CHK(glEnable(GL_PROGRAM_POINT_SIZE));
#endif*/
}

void Shader::end()
{
#ifndef __EMSCRIPTEN__
    CHK(glBindVertexArray(0));
#else
    for (const auto &[key, buf] : m_buffers)
    {
        if (buf.type != VertexBuffer)
            continue;
        CHK(glDisableVertexAttribArray(buf.index));
    }
#endif
    CHK(glUseProgram(0));
}

void Shader::draw_array(PrimitiveType primitive_type, size_t offset, size_t count, bool indexed, size_t instances)
{
    GLenum primitive_type_gl;
    switch (primitive_type)
    {
    case PrimitiveType::Point: primitive_type_gl = GL_POINTS; break;
    case PrimitiveType::Line: primitive_type_gl = GL_LINES; break;
    case PrimitiveType::LineStrip: primitive_type_gl = GL_LINE_STRIP; break;
    case PrimitiveType::LineLoop: primitive_type_gl = GL_LINE_LOOP; break;
    case PrimitiveType::Triangle: primitive_type_gl = GL_TRIANGLES; break;
    case PrimitiveType::TriangleStrip: primitive_type_gl = GL_TRIANGLE_STRIP; break;
    case PrimitiveType::TriangleFan: primitive_type_gl = GL_TRIANGLE_FAN; break;
    default: throw std::runtime_error("Shader::draw_array(): invalid primitive type!");
    }

    if (!indexed)
    {
        if (instances == 0)
            CHK(glDrawArrays(primitive_type_gl, (GLint)offset, (GLsizei)count));
        else
            CHK(glDrawArraysInstanced(primitive_type_gl, (GLint)offset, (GLsizei)count, instances));
    }
    else
    {
        if (instances == 0)
            CHK(glDrawElements(primitive_type_gl, (GLsizei)count, GL_UNSIGNED_INT, (const void *)(offset * sizeof(uint32_t))));
        else
            CHK(glDrawElementsInstanced(primitive_type_gl, (GLsizei)count, GL_UNSIGNED_INT, (const void *)(offset * sizeof(uint32_t)), instances));
    }
}

std::string Shader::Buffer::to_string() const
{
    std::string result = "Buffer[type=";
    switch (type)
    {
    case BufferType::VertexBuffer: result += "vertex"; break;
    case BufferType::FragmentBuffer: result += "fragment"; break;
    case BufferType::UniformBuffer: result += "uniform"; break;
    case BufferType::IndexBuffer: result += "index"; break;
    default: result += "unknown"; break;
    }
    result += ", dtype=";
    result += type_name(dtype);
    result += ", shape=[";
    for (size_t i = 0; i < ndim; ++i)
    {
        result += std::to_string(shape[i]);
        if (i + 1 < ndim)
            result += ", ";
    }
    result += "]]";
    return result;
}
