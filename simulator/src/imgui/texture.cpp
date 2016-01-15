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

#include "texture.h"

namespace eez {
namespace imgui {

Texture::Texture() {
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
}

Texture::~Texture() {
	//Deallocate
	free();
}

bool Texture::loadFromFile(std::string path, SDL_Renderer *renderer) {
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool Texture::loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer *renderer, TTF_Font *font) {
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, textureText.c_str(), textColor);
	if (textSurface != NULL) {
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (mTexture == NULL) {
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else {
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}

	//Return success
	return mTexture != NULL;
}

bool Texture::loadFromImageBuffer(unsigned char *image_buffer, int width, int height, SDL_Renderer *renderer) {
	//Get rid of preexisting texture
	free();

    SDL_Surface* rgbSurface = SDL_CreateRGBSurfaceFrom(image_buffer, width, height, 32, 4 * width, 0, 0, 0, 0);
	if (rgbSurface != NULL) {
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(renderer, rgbSurface);
		if (mTexture == NULL) {
			printf("Unable to create texture from image buffer! SDL Error: %s\n", SDL_GetError());
		}
		else {
			//Get image dimensions
			mWidth = rgbSurface->w;
			mHeight = rgbSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(rgbSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}


    //Return success
	return mTexture != NULL;
}

void Texture::free() {
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
	}
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending) {
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void Texture::setAlpha(Uint8 alpha) {
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void Texture::render(SDL_Renderer *renderer, int x, int y) {
	//Set rendering space and render to screen
	SDL_Rect src_rect = { 0, 0, mWidth, mHeight };
	SDL_Rect dst_rect = { x, y, mWidth, mHeight };

	//Render to screen
	SDL_RenderCopyEx(renderer, mTexture, &src_rect, &dst_rect, 0.0, NULL, SDL_FLIP_NONE);
}

void Texture::render(SDL_Renderer *renderer, int x, int y, int w, int h) {
	//Set rendering space and render to screen
	SDL_Rect src_rect = { 0, 0, mWidth, mHeight };
	SDL_Rect dst_rect = { x, y, w, h };

	//Render to screen
	SDL_RenderCopyEx(renderer, mTexture, &src_rect, &dst_rect, 0.0, NULL, SDL_FLIP_NONE);
}

int Texture::getWidth() {
	return mWidth;
}

int Texture::getHeight() {
	return mHeight;
}

bool Texture::lockTexture() {
	bool success = true;

	//Texture is already locked
	if (mPixels != NULL)
	{
		printf("Texture is already locked!\n");
		success = false;
	}
	//Lock texture
	else
	{
		if (SDL_LockTexture(mTexture, NULL, &mPixels, &mPitch) != 0)
		{
			printf("Unable to lock texture! %s\n", SDL_GetError());
			success = false;
		}
	}

	return success;
}

bool Texture::unlockTexture() {
	bool success = true;

	//Texture is not locked
	if (mPixels == NULL)
	{
		printf("Texture is not locked!\n");
		success = false;
	}
	//Unlock texture
	else
	{
		SDL_UnlockTexture(mTexture);
		mPixels = NULL;
		mPitch = 0;
	}

	return success;
}

void* Texture::getPixels() {
	return mPixels;
}

int Texture::getPitch() {
	return mPitch;
}

}
} // namespace eez::imgui