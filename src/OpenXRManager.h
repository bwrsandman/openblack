#pragma once

#include <array>
#include <vector>

#define XR_USE_GRAPHICS_API_OPENGL
#if defined(WIN32)
#define XR_USE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <unknwn.h>
#elif defined(__ANDROID__)
#define XR_USE_PLATFORM_ANDROID
#else
#define XR_USE_PLATFORM_XLIB
#endif

#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace openblack
{
class OpenXrManager
{
public:

    /// The OpenXR instance and the OpenXR system provide information we'll require to create our window
    /// and rendering backend, so it has to come first
	glm::uvec2 Prepare1()
    {
        PrepareXrInstance();
        return PrepareXrSystem();
    }
    void Prepare2(void* hdc, void* context)
    {
		PrepareXrSession(hdc, context);
		PrepareXrSwapchain();
		PrepareXrCompositionLayers();
    }
    void Destroy();

    bool PollXrEvents();
    bool StartXrFrame();
    void EndXrFrame();
    void UpdateXrViews();

    bool ShouldRender() const 
    {
        return frameState.shouldRender != XR_FALSE;
    }

    uint32_t GetSwapchainSize() const
    {
        return static_cast<uint32_t>(swapchainImages.size());
    }

    uintptr_t GetSwapchainImage(uint32_t index) const
    {
        return static_cast<uintptr_t>(swapchainImages[index].image);
    }

    uint32_t AcquireAndWaitSwapchainImage();
    void ReleaseSwapchainImage();

private:
    void PrepareXrInstance();
    glm::uvec2 PrepareXrSystem();
    void PrepareXrSession(void* hdc, void* context);
    void PrepareXrSwapchain();
    void PrepareXrCompositionLayers();
    void OnSessionStateChanged(const xr::EventDataSessionStateChanged& sessionStateChangedEvent);

    bool enableDebug {true};
    bool quit {false};

private:
	xr::Instance instance;
	xr::DispatchLoaderDynamic dispatch;
	xr::DebugUtilsMessengerEXT messenger;
	xr::SystemId systemId;
	glm::uvec2 renderTargetSize;
	xr::GraphicsRequirementsOpenGLKHR graphicsRequirements;
    xr::Session session;
	std::array<xr::CompositionLayerProjectionView, 2> projectionLayerViews;
	xr::CompositionLayerProjection projectionLayer {{}, {}, 2, projectionLayerViews.data()};
	xr::Space& space {projectionLayer.space};
	xr::SwapchainCreateInfo swapchainCreateInfo;
	xr::Swapchain swapchain;
	std::vector<xr::SwapchainImageOpenGLKHR> swapchainImages;
	std::vector<xr::CompositionLayerBaseHeader*> layersPointers;
    xr::SessionState sessionState {xr::SessionState::Idle};
    xr::FrameState frameState;
	std::vector<xr::View> eyeViewStates;
	std::array<glm::mat4, 2> eyeViews;
	std::array<glm::mat4, 2> eyeProjections;
};
} // namespace openblack