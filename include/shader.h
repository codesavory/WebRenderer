#include "renderpass.h"

class Shader {
public:
    Shader(RenderPass* render_pass, const std::string &name, const std::string &vs_filename,
            const std::string &fs_filename);

    // Return the render pass associated with this shader
    RenderPass* render_pass() {
        return m_render_pass;
    }

protected:
    enum BufferType {
        Unknown = 0,
        VertexBuffer,
        VertexTexture,
        VertexSampler,
        FragmentBuffer,
        FramentTexture,
        FragmentSampler,
        UniformBuffer,
        IndexBuffer
    };

    struct Buffer {
        void *buffer = nullptr;
        BufferType type = Unknown;
        //VariableType dtype = VariableType::Invalid;
        int          index  = 0;
        size_t       ndim   = 0;
        size_t       shape[3]{0, 0, 0};
        size_t       size             = 0;
        size_t       instance_divisor = 0;
        size_t       pointer_offset   = 0;
        bool         dirty            = false;

        std::string to_string() const;
    };

    // Release all resources
    virtual ~Shader();

protected:
    RenderPass* m_render_pass;
    std::string m_name;
    std::unordered_map<std::string, Buffer> m_buffers;
};