//
//  Created by Bradley Austin Davis on 2019/09/18
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

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

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "GameWindow.h"

namespace xrs
{

namespace DebugUtilsEXT
{

using MessageSeverityFlagBits = xr::DebugUtilsMessageSeverityFlagBitsEXT;
using MessageTypeFlagBits = xr::DebugUtilsMessageTypeFlagBitsEXT;
using MessageSeverityFlags = xr::DebugUtilsMessageSeverityFlagsEXT;
using MessageTypeFlags = xr::DebugUtilsMessageTypeFlagsEXT;
using CallbackData = xr::DebugUtilsMessengerCallbackDataEXT;
using Messenger = xr::DebugUtilsMessengerEXT;

// Raw C callback
static XrBool32 debugCallback(XrDebugUtilsMessageSeverityFlagsEXT sev_, XrDebugUtilsMessageTypeFlagsEXT type_,
                              const XrDebugUtilsMessengerCallbackDataEXT* data_, void* userData)
{
	spdlog::debug("{}: {}", data_->functionName, data_->message);
	return XR_TRUE;
}

Messenger create(const xr::Instance& instance, const MessageSeverityFlags& severityFlags = MessageSeverityFlagBits::AllBits,
                 const MessageTypeFlags& typeFlags = MessageTypeFlagBits::AllBits, void* userData = nullptr)
{
	return instance.createDebugUtilsMessengerEXT({severityFlags, typeFlags, debugCallback, userData},
	                                             xr::DispatchLoaderDynamic {instance});
}

} // namespace DebugUtilsEXT

inline XrFovf toTanFovf(const XrFovf& fov)
{
	return {tanf(fov.angleLeft), tanf(fov.angleRight), tanf(fov.angleUp), tanf(fov.angleDown)};
}

inline glm::mat4 toGlm(const XrFovf& fov, float nearZ = 0.01f, float farZ = 10000.0f)
{
	auto tanFov = toTanFovf(fov);
	const auto& tanAngleRight = tanFov.angleRight;
	const auto& tanAngleLeft = tanFov.angleLeft;
	const auto& tanAngleUp = tanFov.angleUp;
	const auto& tanAngleDown = tanFov.angleDown;

	const float tanAngleWidth = tanAngleRight - tanAngleLeft;
	const float tanAngleHeight = (tanAngleDown - tanAngleUp);
	const float offsetZ = 0;

	glm::mat4 resultm {};
	float* result = &resultm[0][0];
	// normal projection
	result[0] = 2 / tanAngleWidth;
	result[4] = 0;
	result[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
	result[12] = 0;

	result[1] = 0;
	result[5] = 2 / tanAngleHeight;
	result[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
	result[13] = 0;

	result[2] = 0;
	result[6] = 0;
	result[10] = -(farZ + offsetZ) / (farZ - nearZ);
	result[14] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

	result[3] = 0;
	result[7] = 0;
	result[11] = -1;
	result[15] = 0;

	return resultm;
}

inline glm::quat toGlm(const XrQuaternionf& q)
{
	return glm::make_quat(&q.x);
}

inline glm::vec3 toGlm(const XrVector3f& v)
{
	return glm::make_vec3(&v.x);
}

inline glm::mat4 toGlm(const XrPosef& p)
{
	glm::mat4 orientation = glm::mat4_cast(toGlm(p.orientation));
	glm::mat4 translation = glm::translate(glm::mat4 {1}, toGlm(p.position));
	return translation * orientation;
}

} // namespace xrs

struct BgfxCallback: public bgfx::CallbackI
{
	~BgfxCallback() override = default;

	void fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* str) override
	{
		const static std::array CodeLookup = {
		    "DebugCheck", "InvalidShader", "UnableToInitialize", "UnableToCreateTexture", "DeviceLost",
		};
		spdlog::critical("bgfx: {}:{}: FATAL ({}): {}", filePath, line, CodeLookup[code], str);

		// Must terminate, continuing will cause crash anyway.
		throw std::runtime_error(std::string("bgfx: ") + filePath + ":" + std::to_string(line) + ": FATAL (" +
		                         CodeLookup[code] + "): " + str);
	}

	void traceVargs([[maybe_unused]] const char* filePath, [[maybe_unused]] uint16_t line, const char* format,
	                va_list argList) override
	{
		char temp[0x2000];
		char* out = temp;

		int32_t len = vsnprintf(out, sizeof(temp), format, argList);
		if (len > 0)
		{
			if ((int32_t)sizeof(temp) < len)
			{
				out = (char*)alloca(len + 1);
				len = vsnprintf(out, len, format, argList);
			}
			out[len] = '\0';
			if (len > 0 && out[len - 1] == '\n')
			{
				out[len - 1] = '\0';
			}
			spdlog::debug("bgfx: {}:{}: {}", filePath, line, out);
		}
	}
	void profilerBegin([[maybe_unused]] const char* name, [[maybe_unused]] uint32_t abgr, [[maybe_unused]] const char* filePath,
	                   [[maybe_unused]] uint16_t line) override
	{
	}
	void profilerBeginLiteral([[maybe_unused]] const char* name, [[maybe_unused]] uint32_t abgr,
	                          [[maybe_unused]] const char* filePath, [[maybe_unused]] uint16_t line) override
	{
	}
	void profilerEnd() override {}
	// Reading and writing to shader cache
	uint32_t cacheReadSize([[maybe_unused]] uint64_t id) override { return 0; }
	bool cacheRead([[maybe_unused]] uint64_t id, [[maybe_unused]] void* data, [[maybe_unused]] uint32_t size) override
	{
		return false;
	}
	void cacheWrite([[maybe_unused]] uint64_t id, [[maybe_unused]] const void* data, [[maybe_unused]] uint32_t size) override {}
	// Saving a screen shot
	void screenShot([[maybe_unused]] const char* filePath, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height,
	                [[maybe_unused]] uint32_t pitch, [[maybe_unused]] const void* data, [[maybe_unused]] uint32_t size,
	                [[maybe_unused]] bool yflip) override
	{
	}
	// Saving a video
	void captureBegin([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height, [[maybe_unused]] uint32_t pitch,
	                  [[maybe_unused]] bgfx::TextureFormat::Enum _format, [[maybe_unused]] bool yflip) override
	{
	}
	void captureEnd() override {}
	void captureFrame([[maybe_unused]] const void* data, [[maybe_unused]] uint32_t size) override {}
};

struct OpenXrExample
{
	bool quit {false};

	// Application main function
	void run()
	{
		// Startup work
		prepare();

		// Loop
		while (!quit)
		{
			frame();
		}

		// Teardown work
		destroy();
	}

	//////////////////////////////////////
	// One-time setup work              //
	//////////////////////////////////////

	// The top level prepare function, which is broken down by task
	void prepare()
	{
		spdlog::set_level(spdlog::level::trace);
		spdlog::stdout_color_mt("game");
		spdlog::stdout_color_mt("graphics");
		// The OpenXR instance and the OpenXR system provide information we'll require to create our window
		// and rendering backend, so it has to come first
		prepareXrInstance();
		prepareXrSystem();

		prepareWindow();
		prepareXrSession();
		prepareXrSwapchain();
		prepareXrCompositionLayers();
	}

	bool enableDebug {true};
	xr::Instance instance;
	xr::DispatchLoaderDynamic dispatch;
	xrs::DebugUtilsEXT::Messenger messenger;
	void prepareXrInstance()
	{
		// xrEnumerate*() functions are usually called once with CapacityInput = 0.
		// The function will write the required amount into CountOutput. We then have
		// to allocate an array to hold CountOutput elements and call the function
		// with CountOutput as CapacityInput.
		uint32_t ext_count = 0;
		auto result = xr::enumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr);
		if (result == XR_ERROR_RUNTIME_UNAVAILABLE)
		{
			throw std::runtime_error("No XR runtime Available");
		}
		else if (result != XR_SUCCESS)
		{
			throw std::runtime_error("Failed to enumerate xr instances");
		}
		else if (ext_count == 0)
		{
			throw std::runtime_error("No XR runtime Available");
		}
		std::vector<xr::ExtensionProperties> exts;
		exts.resize(ext_count);
		result = xr::enumerateInstanceExtensionProperties(nullptr, static_cast<uint32_t>(exts.size()), &ext_count,
		                                                  reinterpret_cast<XrExtensionProperties*>(exts.data()));
		if (result == XR_ERROR_RUNTIME_UNAVAILABLE)
		{
			throw std::runtime_error("No XR runtime Available");
		}
		else if (result != XR_SUCCESS)
		{
			throw std::runtime_error("Failed to enumerate xr instances");
		}
		std::unordered_map<std::string, xr::ExtensionProperties> discoveredExtensions;
		for (const auto& ext : exts)
		{
			spdlog::info("XR Extension found: {}", ext.extensionName);
			discoveredExtensions.insert({ext.extensionName, ext});
		}

#if !defined(SUPPRESS_DEBUG_UTILS)
		if (0 == discoveredExtensions.count(XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
		{
			enableDebug = false;
		}
#else
		enableDebug = false;
#endif

		std::vector<const char*> requestedExtensions;
		if (0 == discoveredExtensions.count(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME))
		{
			throw std::runtime_error(
			    fmt::format("Required Graphics API extension not available: {}", XR_KHR_OPENGL_ENABLE_EXTENSION_NAME));
		}
		requestedExtensions.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

		if (enableDebug)
		{
			requestedExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		xr::InstanceCreateInfo ici {{},
		                            {"gl_single_file_example", 0, "openXrSamples", 0, xr::Version::current()},
		                            0,
		                            nullptr,
		                            (uint32_t)requestedExtensions.size(),
		                            requestedExtensions.data()};

		xr::DebugUtilsMessengerCreateInfoEXT dumci;
		if (enableDebug)
		{
			dumci.messageSeverities = xr::DebugUtilsMessageSeverityFlagBitsEXT::AllBits;
			dumci.messageTypes = xr::DebugUtilsMessageTypeFlagBitsEXT::AllBits;
			dumci.userData = this;
			dumci.userCallback = &xrs::DebugUtilsEXT::debugCallback;
			ici.next = &dumci;
		}

		// Create the actual instance
		instance = xr::createInstance(ici);

		// Turn on debug logging
		if (enableDebug)
		{
			messenger = xrs::DebugUtilsEXT::create(instance);
		}

		// Having created the isntance, the very first thing to do is populate the dynamic dispatch, loading
		// all the available functions from the runtime
		dispatch = xr::DispatchLoaderDynamic::createFullyPopulated(instance, &xrGetInstanceProcAddr);

		// Log the instance properties
		xr::InstanceProperties instanceProperties = instance.getInstanceProperties();
		spdlog::info("OpenXR Runtime {} version {}.{}.{}", //
		             (const char*)instanceProperties.runtimeName, instanceProperties.runtimeVersion.major(),
		             instanceProperties.runtimeVersion.minor(), instanceProperties.runtimeVersion.patch());
	}

	xr::SystemId systemId;
	glm::uvec2 renderTargetSize;
	xr::GraphicsRequirementsOpenGLKHR graphicsRequirements;
	void prepareXrSystem()
	{
		// We want to create an HMD example, so we ask for a runtime that supposts that form factor
		// and get a response in the form of a systemId
		systemId = instance.getSystem(xr::SystemGetInfo {xr::FormFactor::HeadMountedDisplay});

		// Log the system properties
		{
			xr::SystemProperties systemProperties = instance.getSystemProperties(systemId);
			spdlog::info("OpenXR System {} max layers {} max swapchain image size {}x{}", //
			             (const char*)systemProperties.systemName, (uint32_t)systemProperties.graphicsProperties.maxLayerCount,
			             (uint32_t)systemProperties.graphicsProperties.maxSwapchainImageWidth,
			             (uint32_t)systemProperties.graphicsProperties.maxSwapchainImageHeight);
		}

		// Find out what view configurations we have available
		{
			uint32_t viewConfigCount = 0;
			auto result = instance.enumerateViewConfigurations(systemId, 0, &viewConfigCount, nullptr);
			if (result != XR_SUCCESS || viewConfigCount == 0)
			{
				throw std::runtime_error("Failed to enumerate view configurations");
			}
			std::vector<xr::ViewConfigurationType> viewConfigTypes;
			viewConfigTypes.resize(viewConfigCount);
			result =
			    instance.enumerateViewConfigurations(systemId, static_cast<uint32_t>(viewConfigTypes.size()), &viewConfigCount,
			                                         reinterpret_cast<XrViewConfigurationType*>(viewConfigTypes.data()));
			if (result != XR_SUCCESS || viewConfigCount == 0)
			{
				throw std::runtime_error("Failed to enumerate view configurations");
			}

			auto viewConfigType = viewConfigTypes[0];
			if (viewConfigType != xr::ViewConfigurationType::PrimaryStereo)
			{
				throw std::runtime_error("Example only supports stereo-based HMD rendering");
			}
			// xr::ViewConfigurationProperties viewConfigProperties =
			//     instance.getViewConfigurationProperties(systemId, viewConfigType);
			// logging::log(logging::Level::Info, fmt::format(""));
		}

		uint32_t viewConfigViewCount = 0;
		auto result = instance.enumerateViewConfigurationViews(systemId, xr::ViewConfigurationType::PrimaryStereo, 0,
		                                                       &viewConfigViewCount, nullptr);
		if (result != XR_SUCCESS || viewConfigViewCount == 0)
		{
			throw std::runtime_error("Failed to enumerate view configurations");
		}

		std::vector<xr::ViewConfigurationView> viewConfigViews;
		viewConfigViews.resize(viewConfigViewCount);

		result = instance.enumerateViewConfigurationViews(systemId, xr::ViewConfigurationType::PrimaryStereo,
		                                                  static_cast<uint32_t>(viewConfigViews.size()), &viewConfigViewCount,
		                                                  reinterpret_cast<XrViewConfigurationView*>(viewConfigViews.data()));
		if (result != XR_SUCCESS || viewConfigViewCount == 0)
		{
			throw std::runtime_error("Failed to enumerate view configurations");
		}

		// Instead of createing a swapchain per-eye, we create a single swapchain of double width.
		// Even preferable would be to create a swapchain texture array with one layer per eye, so that we could use the
		// VK_KHR_multiview to render both eyes with a single set of draws, but sadly the Oculus runtime doesn't currently
		// support texture array swapchains
		if (viewConfigViews.size() != 2)
		{
			throw std::runtime_error("Unexpected number of view configurations");
		}

		if (viewConfigViews[0].recommendedImageRectHeight != viewConfigViews[1].recommendedImageRectHeight)
		{
			throw std::runtime_error("Per-eye images have different recommended heights");
		}

		renderTargetSize = {viewConfigViews[0].recommendedImageRectWidth * 2, viewConfigViews[0].recommendedImageRectHeight};

		graphicsRequirements = instance.getOpenGLGraphicsRequirementsKHR(systemId, dispatch);
	}

	std::unique_ptr<openblack::GameWindow> window;
	glm::uvec2 windowSize;
	BgfxCallback debugMessageCallback;
	void* hdc;
	void prepareWindow()
	{
		assert(renderTargetSize.x != 0 && renderTargetSize.y != 0);
		windowSize = renderTargetSize;
		//windowSize /= 4;

		window = std::make_unique<openblack::GameWindow>("", windowSize.x, windowSize.y, openblack::DisplayMode::Windowed, 0);

		bgfx::PlatformData pd;
		window->GetNativeHandles(pd.nwh, pd.ndt, hdc);
		pd.context      = NULL;
		pd.backBuffer   = NULL;
		pd.backBufferDS = NULL;
		bgfx::setPlatformData(pd);

		bgfx::Init init;
		init.type = bgfx::RendererType::OpenGL;
		init.resolution.width = windowSize.x;
		init.resolution.height = windowSize.y;
		init.platformData.nwh = pd.nwh;
		init.platformData.ndt = pd.ndt;
		init.resolution.reset = BGFX_RESET_VSYNC;

		init.callback = dynamic_cast<bgfx::CallbackI*>(&debugMessageCallback);

		if (!bgfx::init(init))
		{
			throw std::runtime_error("Failed to initialize bgfx.");
		}

	}

	xr::Session session;
	void prepareXrSession()
	{
		auto internal_data = bgfx::getInternalData();
		xr::GraphicsBindingOpenGLWin32KHR graphicsBinding {(HDC)hdc, (HGLRC)internal_data->context};
		xr::SessionCreateInfo sci {{}, systemId};
		sci.next = &graphicsBinding;
		session = instance.createSession(sci);

		uint32_t spaceCount;
		auto result = session.enumerateReferenceSpaces(0, &spaceCount, nullptr);
		std::vector<xr::ReferenceSpaceType> referenceSpaces;
		referenceSpaces.resize(spaceCount);
		result = session.enumerateReferenceSpaces(static_cast<uint32_t>(referenceSpaces.size()), &spaceCount,
		                                          reinterpret_cast<XrReferenceSpaceType*>(referenceSpaces.data()));

		space = session.createReferenceSpace({xr::ReferenceSpaceType::Local, xr::Posef {}});

		uint32_t formatCount;
		result = session.enumerateSwapchainFormats(0, &formatCount, nullptr);
		std::vector<int64_t> swapchainFormats;
		swapchainFormats.resize(formatCount);
		result = session.enumerateSwapchainFormats(static_cast<uint32_t>(swapchainFormats.size()), &formatCount,
		                                           swapchainFormats.data());
		/*for (const auto& format : swapchainFormats)
		{
		    spdlog::info("\t{}", formatToString((GLenum)format));
		}*/
	}

	xr::SwapchainCreateInfo swapchainCreateInfo;
	xr::Swapchain swapchain;
	std::vector<xr::SwapchainImageOpenGLKHR> swapchainImages;
	std::vector<bgfx::TextureHandle> bgfx_textures;
	std::vector<bgfx::FrameBufferHandle> bgfx_framebuffers;
	void prepareXrSwapchain()
	{
		swapchainCreateInfo.usageFlags = xr::SwapchainUsageFlagBits::TransferDst;
		swapchainCreateInfo.format = (int64_t)0x8C43; // GL_SRGB8_ALPHA8;
		swapchainCreateInfo.sampleCount = 1;
		swapchainCreateInfo.arraySize = 1;
		swapchainCreateInfo.faceCount = 1;
		swapchainCreateInfo.mipCount = 1;
		swapchainCreateInfo.width = renderTargetSize.x;
		swapchainCreateInfo.height = renderTargetSize.y;

		swapchain = session.createSwapchain(swapchainCreateInfo);

		uint32_t imageCount;
		auto result = swapchain.enumerateSwapchainImages(0, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		result = swapchain.enumerateSwapchainImages(static_cast<uint32_t>(swapchainImages.size()), &imageCount,
		                                            reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data()));

		bgfx_textures.resize(swapchainImages.size());
		bgfx_framebuffers.resize(swapchainImages.size());
		for (uint32_t i = 0; i < swapchainImages.size(); ++i)
		{
			bgfx_textures[i] =
			    bgfx::createTexture2D(static_cast<uint16_t>(renderTargetSize.x), static_cast<uint16_t>(renderTargetSize.y),
			                          false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT);
			bgfx::frame();
			bgfx::overrideInternal(bgfx_textures[i], static_cast<uintptr_t>(swapchainImages[i].image));
			bgfx_framebuffers[i] = bgfx::createFrameBuffer(1, &bgfx_textures[i], false);
		}
		bgfx::frame();
	}

	std::array<xr::CompositionLayerProjectionView, 2> projectionLayerViews;
	xr::CompositionLayerProjection projectionLayer {{}, {}, 2, projectionLayerViews.data()};
	xr::Space& space {projectionLayer.space};
	std::vector<xr::CompositionLayerBaseHeader*> layersPointers;
	void prepareXrCompositionLayers()
	{
		// session.getReferenceSpaceBoundsRect(xr::ReferenceSpaceType::Local, bounds);
		projectionLayer.viewCount = 2;
		projectionLayer.views = projectionLayerViews.data();
		layersPointers.push_back(&projectionLayer);
		// Finish setting up the layer submission
		for (uint32_t eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
		{
			auto& layerView = projectionLayerViews[eyeIndex];
			layerView.subImage.swapchain = swapchain;
			layerView.subImage.imageRect.extent = xr::Extent2Di {(int32_t)renderTargetSize.x / 2, (int32_t)renderTargetSize.y};
			if (eyeIndex == 1)
			{
				layerView.subImage.imageRect.offset.x = layerView.subImage.imageRect.extent.width;
			}
		};
	}

	//////////////////////////////////////
	// Per-frame work                   //
	//////////////////////////////////////
	void frame()
	{
		pollSdlEvents();
		pollXrEvents();
		if (quit)
		{
			return;
		}
		if (startXrFrame())
		{
			updateXrViews();
			if (frameState.shouldRender)
			{
				render();
			}
			endXrFrame();
		}
	}

	void pollSdlEvents()
	{
		/*SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				break;
			}
		}*/
	}

	void pollXrEvents()
	{
		while (true)
		{
			xr::EventDataBuffer eventBuffer;
			auto pollResult = instance.pollEvent(eventBuffer);
			if (pollResult == xr::Result::EventUnavailable)
			{
				break;
			}

			switch (eventBuffer.type)
			{
			case xr::StructureType::EventDataSessionStateChanged:
				onSessionStateChanged(reinterpret_cast<xr::EventDataSessionStateChanged&>(eventBuffer));
				break;

			default:
				break;
			}
		}
	}

	xr::SessionState sessionState {xr::SessionState::Idle};
	void onSessionStateChanged(const xr::EventDataSessionStateChanged& sessionStateChangedEvent)
	{
		sessionState = sessionStateChangedEvent.state;
		switch (sessionState)
		{
		case xr::SessionState::Ready:
			if (!quit)
			{
				session.beginSession(xr::SessionBeginInfo {xr::ViewConfigurationType::PrimaryStereo});
			}
			break;

		case xr::SessionState::Stopping:
			session.endSession();
			quit = true;
			break;

		default:
			break;
		}
	}

	xr::FrameState frameState;
	bool startXrFrame()
	{
		switch (sessionState)
		{
		case xr::SessionState::Ready:
		case xr::SessionState::Focused:
		case xr::SessionState::Synchronized:
		case xr::SessionState::Visible:
			session.waitFrame(xr::FrameWaitInfo {}, frameState);
			return xr::Result::Success == session.beginFrame(xr::FrameBeginInfo {});

		default:
			break;
		}

		return false;
	}

	void endXrFrame()
	{
		xr::FrameEndInfo frameEndInfo {frameState.predictedDisplayTime, xr::EnvironmentBlendMode::Opaque, 0, nullptr};
		if (frameState.shouldRender)
		{
			for (uint32_t eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
			{
				auto& layerView = projectionLayerViews[eyeIndex];
				const auto& eyeView = eyeViewStates[eyeIndex];
				layerView.fov = eyeView.fov;
				layerView.pose = eyeView.pose;
			};
			frameEndInfo.layerCount = (uint32_t)layersPointers.size();
			frameEndInfo.layers = layersPointers.data();
		}
		session.endFrame(frameEndInfo);
	}

	std::vector<xr::View> eyeViewStates;
	std::array<glm::mat4, 2> eyeViews;
	std::array<glm::mat4, 2> eyeProjections;
	void updateXrViews()
	{
		xr::ViewState vs;
		xr::ViewLocateInfo vi {xr::ViewConfigurationType::PrimaryStereo, frameState.predictedDisplayTime, space};
		uint32_t viewCount;
		auto result = session.locateViews(vi, &(vs.operator XrViewState&()), 0, &viewCount, nullptr);
		eyeViewStates.resize(viewCount);
		result = session.locateViews(vi, &(vs.operator XrViewState&()), static_cast<uint32_t>(eyeViewStates.size()), &viewCount,
		                             reinterpret_cast<XrView*>(eyeViewStates.data()));
		for (uint32_t eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
		{
			const auto& viewState = eyeViewStates[eyeIndex];
			eyeProjections[eyeIndex] = xrs::toGlm(viewState.fov);
			eyeViews[eyeIndex] = glm::inverse(xrs::toGlm(viewState.pose));
		};
	}

	void render()
	{
		uint32_t swapchainIndex;

		swapchain.acquireSwapchainImage(xr::SwapchainImageAcquireInfo {}, &swapchainIndex);
		swapchain.waitSwapchainImage(xr::SwapchainImageWaitInfo {xr::Duration::infinite()});

		// Set view 0 default viewport.
		bgfx::setViewFrameBuffer(0, bgfx_framebuffers[swapchainIndex]);
		bgfx::setViewRect(0, 0, 0, renderTargetSize.x / 2, renderTargetSize.y);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00FF00FF);
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Set view 1 default viewport.
		bgfx::setViewFrameBuffer(1, bgfx_framebuffers[swapchainIndex]);
		bgfx::setViewRect(1, renderTargetSize.x / 2, 0, renderTargetSize.x / 2, renderTargetSize.y);
		bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0000FFFF);
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(1);

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		swapchain.releaseSwapchainImage(xr::SwapchainImageReleaseInfo {});
	}

	//////////////////////////////////////
	// Shutdown                         //
	//////////////////////////////////////
	void destroy()
	{
		for (uint32_t i = 0; i < bgfx_framebuffers.size(); ++i)
		{
			bgfx::destroy(bgfx_framebuffers[i]);
		}
		for (uint32_t i = 0; i < bgfx_textures.size(); ++i)
		{
			bgfx::destroy(bgfx_textures[i]);
		}
		
		// Shutdown bgfx.
		bgfx::shutdown();

		if (swapchain)
		{
			swapchain.destroy();
			swapchain = nullptr;
		}
		if (session)
		{
			session.destroy();
			session = nullptr;
		}

		window.reset();

		if (messenger)
		{
			messenger.destroy(dispatch);
		}
		if (instance)
		{
			instance.destroy();
			instance = nullptr;
		}

		//SDL_Quit();
	}
};

int main(int argc, char* argv[])
{
	try
	{
		OpenXrExample().run();
	}
	catch (const std::exception& err)
	{
		spdlog::error("{}", err.what());
	}
	return 0;
}
