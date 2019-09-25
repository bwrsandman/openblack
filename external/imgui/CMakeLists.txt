# add imgui library to use for debugging
add_library(imgui STATIC
	imgui/imconfig.h
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_internal.h
	imgui/imgui_widgets.cpp
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
	# opengl helper
	imgui/examples/imgui_impl_sdl.h
	imgui/examples/imgui_impl_sdl.cpp
	# memory_editor
	imgui_club/imgui_memory_editor/imgui_memory_editor.h
	)
target_include_directories(imgui PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/examples)

target_link_libraries(imgui PUBLIC
	SDL2::SDL2
	${OPENGL_LIBRARIES})
