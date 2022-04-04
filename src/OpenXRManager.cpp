#include "OpenXrManager.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>

using namespace openblack;

namespace xrs::DebugUtilsEXT
{

using MessageSeverityFlagBits = xr::DebugUtilsMessageSeverityFlagBitsEXT;
using MessageTypeFlagBits = xr::DebugUtilsMessageTypeFlagBitsEXT;
using MessageSeverityFlags = xr::DebugUtilsMessageSeverityFlagsEXT;
using MessageTypeFlags = xr::DebugUtilsMessageTypeFlagsEXT;
using CallbackData = xr::DebugUtilsMessengerCallbackDataEXT;

// Raw C callback
static XrBool32 debugCallback(XrDebugUtilsMessageSeverityFlagsEXT sev_, XrDebugUtilsMessageTypeFlagsEXT type_,
                              const XrDebugUtilsMessengerCallbackDataEXT* data_, void* userData)
{
	SPDLOG_LOGGER_DEBUG(spdlog::get("graphics"), "{}: {}", data_->functionName, data_->message);
	return XR_TRUE;
}

static xr::DebugUtilsMessengerEXT create(const xr::Instance& instance,
                                         const MessageSeverityFlags& severityFlags = MessageSeverityFlagBits::AllBits,
                                         const MessageTypeFlags& typeFlags = MessageTypeFlagBits::AllBits,
                                         void* userData = nullptr)
{
	return instance.createDebugUtilsMessengerEXT({severityFlags, typeFlags, debugCallback, userData},
	                                             xr::DispatchLoaderDynamic {instance});
}

} // namespace xrs::DebugUtilsEXT

void OpenXrManager::PrepareXrInstance()
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

glm::uvec2 OpenXrManager::PrepareXrSystem()
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
		result = instance.enumerateViewConfigurations(systemId, static_cast<uint32_t>(viewConfigTypes.size()), &viewConfigCount,
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

	assert(renderTargetSize.x != 0 && renderTargetSize.y != 0);
	return renderTargetSize;
}

void OpenXrManager::PrepareXrSession(void* hdc, void* context)
{
	xr::GraphicsBindingOpenGLWin32KHR graphicsBinding {(HDC)hdc, (HGLRC)context};
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

void OpenXrManager::PrepareXrSwapchain()
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
}

void OpenXrManager::PrepareXrCompositionLayers()
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

void OpenXrManager::Destroy()
{
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

	if (messenger)
	{
		messenger.destroy(dispatch);
	}
	if (instance)
	{
		instance.destroy();
		instance = nullptr;
	}
}

bool OpenXrManager::PollXrEvents()
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
			OnSessionStateChanged(reinterpret_cast<xr::EventDataSessionStateChanged&>(eventBuffer));
			break;

		default:
			break;
		}
	}

	return quit;
}

void OpenXrManager::OnSessionStateChanged(const xr::EventDataSessionStateChanged& sessionStateChangedEvent)
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

bool OpenXrManager::StartXrFrame()
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

void OpenXrManager::EndXrFrame()
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

namespace xrs
{

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

void OpenXrManager::UpdateXrViews()
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

uint32_t OpenXrManager::AcquireAndWaitSwapchainImage()
{
	uint32_t swapchainIndex;
	swapchain.acquireSwapchainImage(xr::SwapchainImageAcquireInfo {}, &swapchainIndex);
	swapchain.waitSwapchainImage(xr::SwapchainImageWaitInfo {xr::Duration::infinite()});
	return swapchainIndex;
}

void OpenXrManager::ReleaseSwapchainImage()
{
	swapchain.releaseSwapchainImage(xr::SwapchainImageReleaseInfo {});
}