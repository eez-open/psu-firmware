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

#ifndef EEZ_PSU_GUI_WINDOW_IMPL_H
#define EEZ_PSU_GUI_WINDOW_IMPL_H

#include "window.h"
#include "texture.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>
#include <map>

namespace eez {
namespace imgui {

/// Implementation of the Window.
class WindowImpl {
public:
    WindowImpl(WindowDefinition *window_definition);
    ~WindowImpl();

    bool init();

    bool pollEvent();

    void beginUpdate();

    void addImage(int x, int y, int w, int h, const char *image);
    void addImageBuffer(int x, int y, int w, int h, unsigned char *image_buffer);
    void addOnOffImage(int x, int y, int w, int h, bool value, const char *on_image, const char *off_image);
    void addText(int x, int y, int w, int h, const char *text);
    bool addButton(int x, int y, int w, int h, const char *normal_image, const char *pressed_image);

    void endUpdate();

private:
    Texture *getTexture(const char *path);

    WindowDefinition *window_definition;
    SDL_Window *sdl_window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    typedef std::map<std::string, Texture *> TextureMap;
    TextureMap textures;

    bool mouse_is_down;
    bool mouse_pressed;
    bool mouse_is_up;
    int mouse_x, mouse_y, mouse_down_x, mouse_down_y, mouse_up_x, mouse_up_y;

    bool pointInRect(int px, int py, int x, int y, int w, int h);
};

std::string getFullPath(std::string category, std::string path);

}
} // namespace eez::imgui


#endif // EEZ_PSU_GUI_WINDOW_IMPL_H