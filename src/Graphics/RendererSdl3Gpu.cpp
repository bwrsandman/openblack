/******************************************************************************
 * Copyright (c) 2018-2026 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#define LOCATOR_IMPLEMENTATIONS

#include "RendererSdl3Gpu.h"

#include <cassert>
#include <cstdint>
#include <cstring>

#include <utility>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <spdlog/spdlog.h>

#include "3D/L3DMesh.h"
#include "3D/L3DSubMesh.h"
#include "3D/SkyInterface.h"
#include "Camera/Camera.h"
#include "Debug/Gui.h"
#include "GraphicsHandleSdl3Gpu.h"
#include "IndexBuffer.h"
#include "Locator.h"
#include "Mesh.h"
#include "Profiler.h"
#include "ShaderManager.h"
#include "Texture2d.h"
#include "VertexBuffer.h"
#include "generated/shaders/sky.slang.spv.h"

using namespace openblack::graphics;

#define VK_MAKE_VERSION(major, minor, patch) ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))

void RendererSdl3Gpu::SdlGpuDeleter::operator()(SDL_GPUDevice* device) const noexcept
{
	if (device != nullptr)
	{
		const auto& window = Locator::windowing::value();
		SDL_ReleaseWindowFromGPUDevice(device, reinterpret_cast<SDL_Window*>(window.GetHandle()));

		SDL_DestroyGPUDevice(device);
	}
}

SDL_GPUGraphicsPipeline* CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat targetFormat,
                                        const SDL_GPUShaderCreateInfo& vertexShaderCreateinfo,
                                        const SDL_GPUShaderCreateInfo& fragmentShaderCreateinfo,
                                        const SDL_GPUVertexInputState& vertexInputState)
{

	SDL_GPUColorTargetDescription colorTargetDescriptions = {
	    .format = targetFormat,
	    .blend_state =
	        {
	            .enable_blend = false,
	            .enable_color_write_mask = false,
	        },
	};

	SDL_GPUShader* vs = SDL_CreateGPUShader(device, &vertexShaderCreateinfo);
	SDL_GPUShader* fs = SDL_CreateGPUShader(device, &fragmentShaderCreateinfo);
	SDL_GPUGraphicsPipelineCreateInfo createInfo = {
	    .vertex_shader = vs,
	    .fragment_shader = fs,
	    .vertex_input_state = vertexInputState,
	    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
	    .rasterizer_state =
	        {
	            .enable_depth_bias = false,
	            .enable_depth_clip = false,
	        },
	    .multisample_state =
	        {
	            .sample_count = SDL_GPU_SAMPLECOUNT_1,
	            .sample_mask = 0,
	            .enable_mask = false,
	        },
	    .depth_stencil_state =
	        {
	            .enable_depth_write = true,
	        },
	    .target_info =
	        {
	            .color_target_descriptions = &colorTargetDescriptions,
	            .num_color_targets = 1,
	            .has_depth_stencil_target = false,
	        },
	};
	auto* result = SDL_CreateGPUGraphicsPipeline(device, &createInfo);
	SDL_ReleaseGPUShader(device, vs);
	SDL_ReleaseGPUShader(device, fs);
	return result;
}

std::unique_ptr<RendererInterface> RendererSdl3Gpu::Create() noexcept
{
	SDL_PropertiesID props = SDL_CreateProperties();
	auto vulkanOptions = SDL_GPUVulkanOptions {
	    .vulkan_api_version = VK_MAKE_VERSION(1, 4, 0),
	};
	SDL_SetPointerProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_OPTIONS_POINTER, &vulkanOptions);
	SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
	SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true);
	SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");
	// auto gpuDevice =
	//     std::unique_ptr<SDL_GPUDevice, GpuDeviceDeleter>(SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr));
	auto gpuDevice = std::unique_ptr<SDL_GPUDevice, SdlGpuDeleter>(SDL_CreateGPUDeviceWithProperties(props));
	if (gpuDevice == nullptr)
	{
		SPDLOG_LOGGER_CRITICAL(spdlog::get("graphics"), "Failed to create SDL3 GPU device: {}", SDL_GetError());
		return nullptr;
	}

	const auto& window = Locator::windowing::value();

	if (!SDL_ClaimWindowForGPUDevice(gpuDevice.get(), reinterpret_cast<SDL_Window*>(window.GetHandle())))
	{
		SPDLOG_LOGGER_CRITICAL(spdlog::get("graphics"), "Failed to claim window for GPU device: {}", SDL_GetError());
		return nullptr;
	}

	Pipelines pipelines = {
	    .sky = CreatePipeline(gpuDevice.get(), SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM, k_SkyVertexShaderCreateInfo,
	                          k_SkyFragmentShaderCreateInfo, k_SkyVertexInputStateByVertex),
	};

	return std::unique_ptr<RendererInterface> {
	    reinterpret_cast<RendererInterface*>(new RendererSdl3Gpu(std::move(gpuDevice), std::move(pipelines)))};
}

RendererSdl3Gpu::RendererSdl3Gpu(std::unique_ptr<SDL_GPUDevice, SdlGpuDeleter>&& device, Pipelines&& pipelines) noexcept
    : _gpuDevice(std::move(device))
    , _pipelines(pipelines)
// , _shaderManager(std::make_unique<ShaderManager>())
{
}

RendererSdl3Gpu::~RendererSdl3Gpu() noexcept = default;

void RendererSdl3Gpu::ConfigureView(RenderPass viewId, glm::u16vec2 resolution, uint32_t clearColor) const noexcept
{
	// TODO
}

void RendererSdl3Gpu::Reset(glm::u16vec2 resolution) const noexcept
{
	// TODO
}

void RendererSdl3Gpu::DrawScene(const DrawSceneDesc& drawDesc) noexcept
{
	auto& profiler = Locator::profiler::value();

	SDL_GPUCommandBuffer* cmdBufferMain = SDL_AcquireGPUCommandBuffer(_gpuDevice.get());
	// SDL_GPUCommandBuffer* cmdBufferReflection = SDL_AcquireGPUCommandBuffer(_gpuDevice.get());

	const auto& window = Locator::windowing::value();
	SDL_GPUTexture* swapchainTexture = nullptr;
	// TODO: SDL_WaitAndAcquireGPUSwapchainTexture ?
	if (!SDL_AcquireGPUSwapchainTexture(cmdBufferMain, reinterpret_cast<SDL_Window*>(window.GetHandle()), &swapchainTexture,
	                                    nullptr, nullptr))
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("graphics"), "Failed to acquire swapchain texture: {}", SDL_GetError());
		return;
	}
	SDL_GPUColorTargetInfo colorTargetMain = {.texture = swapchainTexture,
	                                          .clear_color = {.r = 0x27 / static_cast<float>(0xff),
	                                                          .g = 0x46 / static_cast<float>(0xff),
	                                                          .b = 0x59 / static_cast<float>(0xff),
	                                                          .a = 1.0f},
	                                          .load_op = SDL_GPU_LOADOP_CLEAR,
	                                          .store_op = SDL_GPU_STOREOP_STORE,
	                                          .cycle = false};

	auto* renderPassMain = SDL_BeginGPURenderPass(cmdBufferMain, &colorTargetMain, 1, nullptr);

	auto& camera = Locator::camera::value();
	auto view = camera.GetViewMatrix(Camera::Interpolation::Current);
	auto proj = camera.GetProjectionMatrix();
	auto viewProj = proj * view;

	{
		auto section = profiler.BeginScoped(Profiler::Stage::MainPassDrawSky);
		if (drawDesc.drawSky)
		{
			const auto& sky = Locator::skySystem::value();
			const auto skyType = sky.GetCurrentSkyType();
			struct Sky
			{
				float timeOfDay;
				float alignment;
			};
			Sky skyData = {
			    .timeOfDay = skyType,
			    .alignment = Locator::config::value().skyAlignment + 1.0f,
			};

			// SDL_

			// skyShader->SetTextureSampler("s_diffuse", 0, Locator::skySystem::value().GetTexture());
			// skyShader->SetUniformValue("u_typeAlignment", &u_typeAlignment);

			// L3DMeshSubmitDesc submitDesc = {};
			// submitDesc.viewId = desc.viewId;
			// submitDesc.program = skyShader;
			// submitDesc.state = k_BgfxDefaultStateInvertedZ;
			// if (!desc.cullBack)
			// {
			// 	submitDesc.state &= ~BGFX_STATE_CULL_MASK;
			// 	submitDesc.state |= BGFX_STATE_CULL_CCW;
			// }
			// submitDesc.modelMatrices = &modelMatrix;
			// submitDesc.matrixCount = 1;
			// submitDesc.isSky = true;

			// DrawMesh(Locator::skySystem::value().GetMesh(), submitDesc, 0);
			auto& skyMesh = Locator::skySystem::value().GetMesh();
			const auto& subMesh = *skyMesh.GetSubMeshes()[0].get();
			auto& mesh = subMesh.GetMesh();

			{
				auto& skyTexture = sky.GetTexture();
				SDL_GPUTextureSamplerBinding bindings = {
				    .texture = toSdl(skyTexture.handle),
				    .sampler = toSdl(skyTexture.samplerHandle),
				};
				SDL_BindGPUFragmentSamplers(renderPassMain, 0, &bindings, 1);
			}
			SDL_PushGPUVertexUniformData(cmdBufferMain, 0, &viewProj, sizeof(viewProj));
			SDL_PushGPUFragmentUniformData(cmdBufferMain, 0, &skyData, sizeof(skyData));
			DrawSubMesh(renderPassMain, _pipelines.sky, subMesh);
		}
	}

	{
		auto section = profiler.BeginScoped(Profiler::Stage::ReflectionDrawWater);
		if (drawDesc.drawWater)
		{
			assert(false);
		}
	}

	{
		auto section = profiler.BeginScoped(Profiler::Stage::MainPassDrawIsland);
		if (drawDesc.drawIsland)
		{
			assert(false);
		}
	}

	{
		auto section = profiler.BeginScoped(Profiler::Stage::MainPassDrawModels);
		if (drawDesc.drawEntities)
		{
			assert(false);
		}
	}

	SDL_EndGPURenderPass(renderPassMain);

	{
		auto section = profiler.BeginScoped(Profiler::Stage::GuiDraw);
		// TODO:
		// const bool screenshotThisFrame = _requestScreenshot.has_value() && _requestScreenshot->first == _frameCount;
		// // Skip drawing Debug UI for screenshots
		// if (screenshotThisFrame)
		// {
		// 	SPDLOG_LOGGER_INFO(spdlog::get("game"), "Requesting a screenshot at frame {}...", _frameCount);
		// 	Locator::rendererInterface::value().RequestScreenshot(_requestScreenshot->second);
		// }
		// else
		{
			Locator::debugGui::value().Draw(cmdBufferMain, swapchainTexture);
		}
	}

	SDL_SubmitGPUCommandBuffer(cmdBufferMain);
}

void RendererSdl3Gpu::DrawSubMesh(SDL_GPURenderPass* renderPass, SDL_GPUGraphicsPipeline* pipeline,
                                  const L3DSubMesh& subMesh) const
{
	const uint32_t numInstances = 1;
	auto& mesh = subMesh.GetMesh();
	{
		SDL_GPUBufferBinding bindings = {
		    .buffer = toSdl(mesh._vertexBuffer->handle),
		    .offset = 0,
		};
		SDL_BindGPUVertexBuffers(renderPass, 0, &bindings, 1);
	}
	if (mesh.IsIndexed())
	{
		SDL_GPUBufferBinding bindings = {
		    .buffer = toSdl(mesh._indexBuffer->handle),
		    .offset = 0,
		};
		SDL_BindGPUIndexBuffer(renderPass, &bindings, SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_16BIT);
	}
	SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
	SDL_DrawGPUIndexedPrimitives(renderPass, mesh._indexBuffer->count, numInstances, 0, 0, 0);
}

void RendererSdl3Gpu::Frame() noexcept
{
	const auto& window = Locator::windowing::value();
	SDL_WaitForGPUSwapchain(_gpuDevice.get(), reinterpret_cast<SDL_Window*>(window.GetHandle()));
}

void RendererSdl3Gpu::RequestScreenshot(const std::filesystem::path& filepath) noexcept
{
	// TODO
}

[[nodiscard]] bool RendererSdl3Gpu::GetDebug() const noexcept
{
	// TODO
	return false;
}

void RendererSdl3Gpu::SetDebug(bool value) noexcept
{
	// TODO
}

[[nodiscard]] bool RendererSdl3Gpu::GetProfile() const noexcept
{
	// TODO
	return false;
}

void RendererSdl3Gpu::SetProfile(bool value) noexcept
{
	// TODO
}

void RendererSdl3Gpu::DrawMesh(const L3DMesh& mesh, const L3DMeshSubmitDesc& desc, uint8_t subMeshIndex) const noexcept
{
	// TODO
}

[[nodiscard]] ShaderManager& RendererSdl3Gpu::GetShaderManager() const noexcept
{
	// TODO
	ShaderManager* s;
	return *s;
}

VertexBufferUniquePtr RendererSdl3Gpu::CreateVertexBuffer(std::string name, const void* memory, VertexDecl decl) noexcept
{
	assert(false);
	return {nullptr};
}

VertexBufferUniquePtr RendererSdl3Gpu::CreateVertexBuffer(std::string name, std::span<const uint8_t> memory,
                                                          VertexDecl decl) noexcept
{
	VertexBufferHandle handle;
	{
		SDL_GPUBufferCreateInfo info = {
		    .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		    .size = static_cast<uint32_t>(memory.size_bytes()),
		    .props = 0,
		};
		auto* buffer = SDL_CreateGPUBuffer(_gpuDevice.get(), &info);
		SDL_SetGPUBufferName(_gpuDevice.get(), buffer, name.c_str());
		handle = fromSdl<VertexBufferHandle>(buffer);

		// TODO: Maintain a transfer buffer to batch uploads
		SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
		    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		    .size = static_cast<uint32_t>(memory.size_bytes()),
		};
		SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(_gpuDevice.get(), &transferBufferCreateInfo);
		SDL_GPUTransferBufferLocation location = {
		    .transfer_buffer = transferBuffer,
		    .offset = 0,
		};
		SDL_GPUBufferRegion bufferRegion = {
		    .buffer = buffer,
		    .offset = 0,
		    .size = static_cast<uint32_t>(memory.size_bytes()),
		};

		void* cpuSide = SDL_MapGPUTransferBuffer(_gpuDevice.get(), transferBuffer, false);
		memcpy(cpuSide, memory.data(), memory.size_bytes());
		SDL_UnmapGPUTransferBuffer(_gpuDevice.get(), transferBuffer);

		auto* commandBuffer = SDL_AcquireGPUCommandBuffer(_gpuDevice.get());

		auto* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

		SDL_UploadToGPUBuffer(copyPass, &location, &bufferRegion, false);

		SDL_EndGPUCopyPass(copyPass);
		SDL_SubmitGPUCommandBuffer(commandBuffer);

		SDL_ReleaseGPUTransferBuffer(_gpuDevice.get(), transferBuffer);
	}

	return {new VertexBuffer(name, 0, 0, handle), DestroyVertexBuffer};
}

IndexBufferUniquePtr RendererSdl3Gpu::CreateIndexBuffer(std::string name, const void* memory, IndexBuffer::Type type) noexcept
{
	// TODO
	return {nullptr};
}

IndexBufferUniquePtr RendererSdl3Gpu::CreateIndexBuffer(std::string name, std::span<const uint8_t> memory,
                                                        IndexBuffer::Type type) noexcept
{
	const auto info = SDL_GPUBufferCreateInfo {
	    .usage = SDL_GPU_BUFFERUSAGE_INDEX,
	    .size = static_cast<uint32_t>(memory.size_bytes()),
	    .props = 0,
	};
	auto* buffer = SDL_CreateGPUBuffer(_gpuDevice.get(), &info);
	if (buffer == nullptr)
	{
		return nullptr;
	}

	SDL_SetGPUBufferName(_gpuDevice.get(), buffer, name.c_str());
	auto handle = fromSdl<IndexBufferHandle>(buffer);

	// TODO: Maintain a transfer buffer to batch uploads
	const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
	    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
	    .size = static_cast<uint32_t>(memory.size_bytes()),
	};
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(_gpuDevice.get(), &transferBufferCreateInfo);
	const SDL_GPUTransferBufferLocation location = {
	    .transfer_buffer = transferBuffer,
	    .offset = 0,
	};
	const SDL_GPUBufferRegion bufferRegion = {
	    .buffer = buffer,
	    .offset = 0,
	    .size = static_cast<uint32_t>(memory.size_bytes()),
	};

	void* cpuSide = SDL_MapGPUTransferBuffer(_gpuDevice.get(), transferBuffer, false);
	memcpy(cpuSide, memory.data(), memory.size_bytes());
	SDL_UnmapGPUTransferBuffer(_gpuDevice.get(), transferBuffer);

	auto* commandBuffer = SDL_AcquireGPUCommandBuffer(_gpuDevice.get());

	auto* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

	SDL_UploadToGPUBuffer(copyPass, &location, &bufferRegion, false);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);

	SDL_ReleaseGPUTransferBuffer(_gpuDevice.get(), transferBuffer);

	return {new IndexBuffer(name, static_cast<uint32_t>(memory.size_bytes() / IndexBuffer::GetTypeSize(type)), type, handle),
	        DestroyIndexBuffer};
}

Texture2dUniquePtr RendererSdl3Gpu::CreateTexture2d(std::string name, const void* memory, glm::u16vec2 resolution,
                                                    uint16_t layers, TextureFormat format, Wrapping wrapping,
                                                    Filter filter) noexcept
{
	// TODO
	assert(false);
}

Texture2dUniquePtr RendererSdl3Gpu::CreateTexture2d(std::string name, std::span<const uint8_t> memory, glm::u16vec2 resolution,
                                                    uint16_t layers, TextureFormat format, Wrapping wrapping,
                                                    Filter filter) noexcept
{
	const auto sdl3GpuAddressMode = toSdl(wrapping);
	const auto sdl3GpuFilter = toSdl(filter);

	const auto info = SDL_GPUTextureCreateInfo {
	    .type = layers == 1 ? SDL_GPU_TEXTURETYPE_2D : SDL_GPU_TEXTURETYPE_2D_ARRAY,
	    .format = toSdl(format),
	    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
	    .width = resolution.x,
	    .height = resolution.y,
	    .layer_count_or_depth = layers,
	    .num_levels = 1,
	    .sample_count = SDL_GPU_SAMPLECOUNT_1,
	};
	auto* texture = SDL_CreateGPUTexture(_gpuDevice.get(), &info);
	if (texture == nullptr)
	{
		return nullptr;
	}
	const auto samplerCreateInfo = SDL_GPUSamplerCreateInfo {
	    .min_filter = sdl3GpuFilter,
	    .mag_filter = sdl3GpuFilter,
	    .address_mode_u = sdl3GpuAddressMode,
	    .address_mode_v = sdl3GpuAddressMode,
	    .address_mode_w = sdl3GpuAddressMode,
	};
	auto* samplerHandle = SDL_CreateGPUSampler(_gpuDevice.get(), &samplerCreateInfo);
	if (samplerHandle == nullptr)
	{
		SDL_ReleaseGPUTexture(_gpuDevice.get(), texture);
		return nullptr;
	}

	SDL_SetGPUTextureName(_gpuDevice.get(), texture, name.c_str());
	auto handle = fromSdl(texture);

	// TODO: Maintain a transfer buffer to batch uploads
	SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
	    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
	    .size = static_cast<uint32_t>(memory.size_bytes()),
	};
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(_gpuDevice.get(), &transferBufferCreateInfo);
	SDL_GPUTextureTransferInfo location = {
	    .transfer_buffer = transferBuffer,
	    .offset = 0,
	    .pixels_per_row = resolution.x,
	    .rows_per_layer = resolution.y,
	};
	SDL_GPUTextureRegion textureRegion = {
	    .texture = texture,
	    .mip_level = 0,
	    .layer = 0,
	    .x = 0,
	    .y = 0,
	    .z = 0,
	    .w = resolution.x,
	    .h = resolution.y,
	    .d = 1,
	};
	const auto bytesPerLayer = memory.size_bytes() / layers;

	void* cpuSide = SDL_MapGPUTransferBuffer(_gpuDevice.get(), transferBuffer, false);
	memcpy(cpuSide, memory.data(), memory.size_bytes());
	SDL_UnmapGPUTransferBuffer(_gpuDevice.get(), transferBuffer);

	auto* commandBuffer = SDL_AcquireGPUCommandBuffer(_gpuDevice.get());

	auto* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

	for (uint16_t i = 0; i < layers; ++i)
	{
		location.offset = i * bytesPerLayer;
		textureRegion.layer = i;
		SDL_UploadToGPUTexture(copyPass, &location, &textureRegion, false);
	}

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);

	return {new Texture2d(name, handle, fromSdl(samplerHandle), resolution, layers, format), DestroyTexture2d};
}

void RendererSdl3Gpu::DestroyVertexBuffer(VertexBuffer* buffer)
{
	// delete[] toSdl(buffer->layoutHandle);
	SDL_ReleaseGPUBuffer(reinterpret_cast<RendererSdl3Gpu&>(Locator::rendererInterface::value())._gpuDevice.get(),
	                     toSdl(buffer->handle));
}

void RendererSdl3Gpu::DestroyIndexBuffer(IndexBuffer* buffer)
{
	SDL_ReleaseGPUBuffer(reinterpret_cast<RendererSdl3Gpu&>(Locator::rendererInterface::value())._gpuDevice.get(),
	                     toSdl(buffer->handle));
}

void RendererSdl3Gpu::DestroyTexture2d(Texture2d* texture)
{
	SDL_ReleaseGPUTexture(reinterpret_cast<RendererSdl3Gpu&>(Locator::rendererInterface::value())._gpuDevice.get(),
	                      toSdl(texture->handle));
}

void RendererSdl3Gpu::Bind(const VertexBuffer& buffer) const noexcept
{
	// TODO
}

void RendererSdl3Gpu::Bind(const IndexBuffer& buffer, uint32_t count, uint32_t startIndex) const noexcept
{
	// TODO
}
