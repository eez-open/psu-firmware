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

#ifndef EEZ_PSU_GUI_WINDOW_H
#define EEZ_PSU_GUI_WINDOW_H

namespace eez {
//! Simple reusable immediate mode GUI.
namespace imgui {

/// Description of the top level winodow, i.e. title, width, height, ...
struct WindowDefinition {
	const char *title;
	int width, height;
	int content_padding;
	const char *icon_path;
};

class WindowImpl;

/// Top level window, high level interface.
class Window {
public:
	Window(WindowDefinition *window_definition);
	virtual ~Window();

	bool init();

	virtual bool pollEvent();

	virtual void beginUpdate();

	virtual void addImage(int x, int y, int w, int h, const char *image);
    virtual void addImageBuffer(int x, int y, int w, int h, unsigned char *image_buffer);
	virtual void addOnOffImage(int x, int y, int w, int h, bool value, const char *on_image, const char *off_image);
	virtual void addText(int x, int y, int w, int h, const char *text);
	virtual bool addButton(int x, int y, int w, int h, const char *normal_image, const char *pressed_image);

	virtual void endUpdate();

private:
	WindowImpl *impl;
};

typedef Window *(*create_window_ptr_t)(WindowDefinition *windowDefinition);

}
} // namespace eez::imgui

#endif // EEZ_PSU_GUI_WINDOW_H