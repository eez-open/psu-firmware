/*
 * EEZ Middleware
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

#pragma once

namespace eez {
namespace platform {
namespace simulator {
//! Simple reusable immediate mode GUI.
namespace imgui {

/// Description of the top level winodow, i.e. title, width, height, ...
struct WindowDefinition {
	const char *title;
	int width, height;
	int content_padding;
	const char *icon_path;
};

struct MouseData {
	int x, y;

	bool button1IsDown;
	bool button1IsPressed;
	bool button1IsUp;
	int button1DownX, button1DownY;
	int button1UpX, button1UpY;

	bool button2IsDown;
	bool button2IsPressed;
	bool button2IsUp;
};

struct UserWidget {
	int x;
	int y;
	int w;
	int h;

	int pixels_w;
	int pixels_h;
	unsigned char *pixels;

	MouseData mouseData;
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
	virtual void addOnOffImage(int x, int y, int w, int h, bool value, const char *on_image, const char *off_image);
	virtual void addText(int x, int y, int w, int h, const char *text);
	virtual bool addButton(int x, int y, int w, int h, const char *normal_image, const char *pressed_image);
	virtual void addUserWidget(UserWidget *user_widget);

	virtual void endUpdate();

	virtual void getMouseData(MouseData *mouseData);
	virtual void getMouseWheelData(int *x, int *y);

private:
	WindowImpl *impl;
};

Window *createWindow(WindowDefinition *windowDefinition);
int getDesktopResolution(int *w, int *h);

}
}
}
} // namespace eez::platform::simulator::imgui
