//
//  Created by Bradley Austin Davis on 2019/09/18
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "OpenXrManager.h"

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "GameWindow.h"

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
		spdlog::set_level(spdlog::level::trace);
		spdlog::stdout_color_mt("game");
		spdlog::stdout_color_mt("graphics");

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
	openblack::OpenXrManager oxr;
	void* hdc;
	std::vector<bgfx::TextureHandle> bgfx_textures;
	std::vector<bgfx::FrameBufferHandle> bgfx_framebuffers;
	void prepare()
	{
		const auto windowSize = oxr.Prepare1();
		prepareWindow(windowSize);
		oxr.Prepare2(hdc, bgfx::getInternalData()->context);

		bgfx_textures.resize(oxr.GetSwapchainSize());
		bgfx_framebuffers.resize(oxr.GetSwapchainSize());
		for (uint32_t i = 0; i < oxr.GetSwapchainSize(); ++i)
		{
			bgfx_textures[i] = bgfx::createTexture2D(static_cast<uint16_t>(windowSize.x), static_cast<uint16_t>(windowSize.y),
			                                         false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT);
			bgfx::frame();
			bgfx::overrideInternal(bgfx_textures[i], oxr.GetSwapchainImage(i));
			bgfx_framebuffers[i] = bgfx::createFrameBuffer(1, &bgfx_textures[i], false);
		}
		bgfx::frame();
	}

	std::unique_ptr<openblack::GameWindow> window;
	glm::uvec2 windowSize;
	BgfxCallback debugMessageCallback;
	void prepareWindow(const glm::uvec2& renderTargetSize)
	{
		windowSize = renderTargetSize;
		window = std::make_unique<openblack::GameWindow>("", windowSize.x, windowSize.y, openblack::DisplayMode::Windowed, 0);

		bgfx::PlatformData pd;
		window->GetNativeHandles(pd.nwh, pd.ndt, hdc);
		pd.context = NULL;
		pd.backBuffer = NULL;
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

	//////////////////////////////////////
	// Per-frame work                   //
	//////////////////////////////////////
	void frame()
	{
		pollSdlEvents();
		quit |= oxr.PollXrEvents();
		if (quit)
		{
			return;
		}
		if (oxr.StartXrFrame())
		{
			oxr.UpdateXrViews();
			if (oxr.ShouldRender())
			{
				render();
			}
			oxr.EndXrFrame();
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

	void render()
	{
		uint32_t swapchainIndex = oxr.AcquireAndWaitSwapchainImage();

		// Set view 0 default viewport.
		bgfx::setViewFrameBuffer(0, bgfx_framebuffers[swapchainIndex]);
		bgfx::setViewRect(0, 0, 0, windowSize.x / 2, windowSize.y);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00FF00FF);
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Set view 1 default viewport.
		bgfx::setViewFrameBuffer(1, bgfx_framebuffers[swapchainIndex]);
		bgfx::setViewRect(1, windowSize.x / 2, 0, windowSize.x / 2, windowSize.y);
		bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0000FFFF);
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(1);

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();

		oxr.ReleaseSwapchainImage();
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

		window.reset();

		oxr.Destroy();

		// SDL_Quit();
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
