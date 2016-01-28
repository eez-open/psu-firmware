/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window_impl.h"

#include "dll.h"

namespace eez {
namespace imgui {

std::string getFullPath(std::string category, std::string path) {
	std::string fullPath = category + "/" + path;
	for (int i = 0; i < 5; ++i) {
		FILE *fp = fopen(fullPath.c_str(), "r");
		if (fp) {
			fclose(fp);
			return fullPath;
		}
		fullPath = std::string("../") + fullPath;
	}
	return path;
}

Window::Window(WindowDefinition *window_definition)
	: impl(new WindowImpl(window_definition))
{
}

Window::~Window() {
	delete impl;
}

bool Window::init() {
	return impl->init();
}

bool Window::pollEvent() {
	return impl->pollEvent();
}

void Window::beginUpdate() {
	impl->beginUpdate();
}

void Window::addImage(int x, int y, int w, int h, const char *image) {
	impl->addImage(x, y, w, h, image);
}

void Window::addOnOffImage(int x, int y, int w, int h, bool value, const char *on_image, const char *off_image) {
	impl->addOnOffImage(x, y, w, h, value, on_image, off_image);
}

void Window::addText(int x, int y, int w, int h, const char *text) {
	impl->addText(x, y, w, h, text);
}

bool Window::addButton(int x, int y, int w, int h, const char *normal_image, const char *pressed_image) {
	return impl->addButton(x, y, w, h, normal_image, pressed_image);
}

void Window::addUserWidget(UserWidget *user_widget) {
    impl->addUserWidget(user_widget);
}

void Window::endUpdate() {
	impl->endUpdate();
}

////////////////////////////////////////////////////////////////////////////////

WindowImpl::WindowImpl(WindowDefinition *window_definition_)
	: window_definition(window_definition_)
	, sdl_window(0)
	, renderer(0)
	, font(0)
{
    mouse_data.is_down = false;
    mouse_data.is_pressed = false;
    mouse_data.is_up = false;
}

WindowImpl::~WindowImpl() {
	for (TextureMap::iterator it = textures.begin(); it != textures.end(); ++it) {
		delete it->second;
	}

	if (font) {
		TTF_CloseFont(font);
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}

	if (sdl_window) {
		SDL_DestroyWindow(sdl_window);
	}
}

bool WindowImpl::init() {
	// Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		printf("Warning: Linear texture filtering not enabled!");
	}

	// Create window
	sdl_window = SDL_CreateWindow(
		window_definition->title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_definition->width + 2 * window_definition->content_padding,
		window_definition->height + 2 * window_definition->content_padding,
		SDL_WINDOW_HIDDEN /* | SDL_WINDOW_RESIZABLE*/);

	if (sdl_window == NULL) {
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Create renderer
	renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	// Initialize TTF
	if (TTF_Init() == -1) {
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	font = TTF_OpenFont(getFullPath("fonts", "OpenSansRegular.ttf").c_str(), 28);

	if (window_definition->icon_path) {
		SDL_Surface* icon_surface = IMG_Load(getFullPath("images", window_definition->icon_path).c_str());
		if (icon_surface) {
			SDL_SetWindowIcon(sdl_window, icon_surface);
			SDL_FreeSurface(icon_surface);
		}
	}

	SDL_ShowWindow(sdl_window);

	return true;
}

bool WindowImpl::pollEvent() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
			SDL_GetMouseState(&mouse_data.x, &mouse_data.y);
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				mouse_data.down_x = mouse_data.x;
				mouse_data.down_y = mouse_data.y;
				mouse_data.is_down = true;
				mouse_data.is_pressed = true;
			}
			else if (event.type == SDL_MOUSEBUTTONUP) {
				mouse_data.up_x = mouse_data.x;
				mouse_data.up_y = mouse_data.y;
				mouse_data.is_up = true;
				mouse_data.is_pressed = false;
			}
		}

		if (event.type == SDL_QUIT)
			return false;
	}
	return true;
}

void WindowImpl::beginUpdate() {
	// Clear screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);
}

void WindowImpl::endUpdate() {
	// Update screen
	SDL_RenderPresent(renderer);

	mouse_data.is_down = false;
	mouse_data.is_up = false;
}

void WindowImpl::addImage(int x, int y, int w, int h, const char *image) {
	x += window_definition->content_padding;
	y += window_definition->content_padding;

	getTexture(image)->render(renderer, x, y);
}

void WindowImpl::addOnOffImage(int x, int y, int w, int h, bool value, const char *on_image, const char *off_image) {
	x += window_definition->content_padding;
	y += window_definition->content_padding;

	if (value) {
		getTexture(on_image)->render(renderer, x, y);
	}
	else {
		getTexture(off_image)->render(renderer, x, y);
	}

}

void WindowImpl::addText(int x, int y, int w, int h, const char *text) {
	x += window_definition->content_padding;
	y += window_definition->content_padding;

	Texture tex;
	SDL_Color textColor = { 0, 0, 0 };
	if (tex.loadFromRenderedText(text, textColor, renderer, font)) {
		int src_w = tex.getWidth();
		int src_h = tex.getHeight();

		int dst_w = w;
		int dst_h = h;

		if ((double)src_w / src_h < (double)dst_w / dst_h) {
			w = (int)((double)src_w * dst_h / src_h);
			h = dst_h;
			x += (dst_w - w) / 2;
		}
		else {
			w = dst_w;
			h = (int)((double)src_h * dst_w / src_w);
			y += (dst_h - h) / 2;
		}

		tex.render(renderer, x, y, w, h);
	}
}

bool WindowImpl::addButton(int x, int y, int w, int h, const char *normal_image, const char *pressed_image) {
	x += window_definition->content_padding;
	y += window_definition->content_padding;

	bool is_pressed = mouse_data.is_pressed &&
		pointInRect(mouse_data.down_x, mouse_data.down_y, x, y, w, h) &&
		pointInRect(mouse_data.x, mouse_data.y, x, y, w, h);

	if (is_pressed) {
		getTexture(pressed_image)->render(renderer, x, y);
	}
	else {
		getTexture(normal_image)->render(renderer, x, y);
	}

	bool is_clicked = mouse_data.is_up &&
		pointInRect(mouse_data.down_x, mouse_data.down_y, x, y, w, h) &&
		pointInRect(mouse_data.up_x, mouse_data.up_y, x, y, w, h);

	return is_clicked;
}

Texture *WindowImpl::getTexture(const char *path) {
	TextureMap::iterator it = textures.find(path);
	if (it != textures.end()) {
		return it->second;
	}

	Texture *texture = new Texture();
	texture->loadFromFile(getFullPath("images", path), renderer);

	texture->setBlendMode(SDL_BLENDMODE_BLEND);
	//texture->setAlpha(100);

	textures.insert(std::make_pair(path, texture));

	return texture;
}

bool WindowImpl::pointInRect(int px, int py, int x, int y, int w, int h) {
	return px >= x && px < x + w && py >= y && py <= y + h;
}

void WindowImpl::addUserWidget(UserWidget *user_widget) {
    int x = user_widget->x + window_definition->content_padding;
    int y = user_widget->y + window_definition->content_padding;

	Texture tex;
    if (tex.loadFromImageBuffer(user_widget->pixels, user_widget->w, user_widget->h, renderer)) {
        tex.render(renderer, x, y);
    }

    memcpy(&user_widget->mouse_data, &mouse_data, sizeof(MouseData));
    
    user_widget->mouse_data.x -= x;
    user_widget->mouse_data.y -= y;

    user_widget->mouse_data.down_x -= x;
    user_widget->mouse_data.down_y -= y;

    user_widget->mouse_data.up_x -= x;
    user_widget->mouse_data.up_y -= y;
}

}
} // namespace eez::imgui

////////////////////////////////////////////////////////////////////////////////

using namespace eez::imgui;

EEZ_DLL_EXPORT Window *eez_imgui_create_window(WindowDefinition *windowDefinition) {
    Window *window = new Window(windowDefinition);

    if (!window->init()) {
        delete window;
        window = 0;
    }

    return window;
}

