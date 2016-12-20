/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
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

#ifndef EEZ_PSU_GUI_TEXTURE_H
#define EEZ_PSU_GUI_TEXTURE_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <string>

namespace eez {
namespace imgui {

/// Image and font texture that can be rendered using SDL_Renderer.
class Texture
{
public:
    //Initializes variables
    Texture();

    //Deallocates memory
    ~Texture();

    //Loads image at specified path
    bool loadFromFile(std::string path, SDL_Renderer *renderer);

    //Creates image from font string
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer *renderer, TTF_Font *font);

    //Creates image from image buffer
    bool loadFromImageBuffer(unsigned char *image_buffer, int width, int height, SDL_Renderer *renderer);

    //Deallocates texture
    void free();

    //Set color modulation
    void setColor(Uint8 red, Uint8 green, Uint8 blue);

    //Set blending
    void setBlendMode(SDL_BlendMode blending);

    //Set alpha modulation
    void setAlpha(Uint8 alpha);

    //Renders texture at given point
    void render(SDL_Renderer *renderer, int x, int y);
    void render(SDL_Renderer *renderer, int x, int y, int w, int h);

    //Gets image dimensions
    int getWidth();
    int getHeight();

    //Pixel manipulators
    bool lockTexture();
    bool unlockTexture();
    void* getPixels();
    int getPitch();

private:
    //The actual hardware texture
    SDL_Texture* mTexture;
    void* mPixels;
    int mPitch;

    //Image dimensions
    int mWidth;
    int mHeight;
};

}
} // namespace eez::imgui

#endif // EEZ_PSU_GUI_TEXTURE_H