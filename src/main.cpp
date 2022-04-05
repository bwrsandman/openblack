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
#include "Renderer.h"

struct OpenXrExample
{
	bool quit {false};
	openblack::OpenXrManager oxr;
	std::unique_ptr<openblack::GameWindow> window;
	std::unique_ptr<openblack::Renderer> renderer;
	glm::uvec2 windowSize;
	std::vector<bgfx::TextureHandle> bgfx_textures;
	std::vector<bgfx::FrameBufferHandle> bgfx_framebuffers;

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
	void prepare()
	{
		windowSize = oxr.Prepare1();
		bgfx::RendererType::Enum rendererType = bgfx::RendererType::OpenGL;
		bool vsync = true;
		window = std::make_unique<openblack::GameWindow>("", windowSize.x, windowSize.y, openblack::DisplayMode::Windowed, 0);
		renderer = std::make_unique<openblack::Renderer>(window.get(), rendererType, vsync);
		oxr.Prepare2(renderer->hdc, bgfx::getInternalData()->context);

		bgfx_textures.resize(oxr.GetSwapchainSize());
		bgfx_framebuffers.resize(oxr.GetSwapchainSize());
		for (uint32_t i = 0; i < oxr.GetSwapchainSize(); ++i)
		{
			bgfx_textures[i] = bgfx::createTexture2D(static_cast<uint16_t>(windowSize.x), static_cast<uint16_t>(windowSize.y),
			                                         false, 2, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT);
			bgfx::frame();
			bgfx::overrideInternal(bgfx_textures[i], oxr.GetSwapchainImage(i));
			bgfx_framebuffers[i] = bgfx::createFrameBuffer(1, &bgfx_textures[i], false);
		}
		bgfx::frame();
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
		bgfx::setViewRect(0, 0, 0, windowSize.x, windowSize.y);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00FF00FF);
		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Set view 1 default viewport.
		bgfx::setViewFrameBuffer(1, bgfx_framebuffers[swapchainIndex]);
		bgfx::setViewRect(1, windowSize.x, 0, windowSize.x / 2, windowSize.y);
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
