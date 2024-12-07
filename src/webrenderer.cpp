#include "webrenderer.h"

WebRenderer::WebRenderer() {
    
    // set up HelloIGui params
    m_params.appWindowParams.windowGeometry.size = {1200, 800};
    m_params.appWindowParams.windowTitle = "Web Renderer";
    m_params.appWindowParams.restorePreviousGeometry = false;

    // Menu bar
    m_params.imGuiWindowParams.showMenuBar = true;
    m_params.imGuiWindowParams.showStatusBar = true;
    m_params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;

    
}

void WebRenderer::run() {
    HelloImGui::Run(m_params);
}