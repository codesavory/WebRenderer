/*
Inspired by the works of Wojciech Jarosz in https://github.com/wkjarosz/SamplinSafari/
*/
#pragma once

#include "linalg.h"
#include <unordered_map>

// An abstraction fo rendering passes that work with OpenGL and Meta

class RenderPass {
public:
    RenderPass();
    ~RenderPass();

    void begin();
    void end();

protected:

};
