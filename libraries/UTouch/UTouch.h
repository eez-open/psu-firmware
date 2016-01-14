/*
  UTouch.h - Arduino/chipKit library support for Color TFT LCD Touch screens 
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
  
  Basic functionality of this library are based on the demo-code provided by
  ITead studio.

  You can find the latest version of the library at 
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/

#ifndef UTouch_h
#define UTouch_h

#define UTOUCH_VERSION	130

#if defined(__AVR__)
	#include "Arduino.h"
	#include "hardware/avr/HW_AVR_defines.h"
#elif defined(__PIC32MX__)
	#include "WProgram.h"
	#include "hardware/pic32/HW_PIC32_defines.h"
#elif defined(__arm__)
	#include "Arduino.h"
	#include "hardware/arm/HW_ARM_defines.h"
#endif

#define PORTRAIT			0
#define LANDSCAPE			1

#define PREC_LOW			1
#define PREC_MEDIUM			2
#define PREC_HI				3
#define PREC_EXTREME		4

class UTouch
{
	public:
		int16_t	TP_X ,TP_Y;

				UTouch(byte tclk, byte tcs, byte tdin, byte dout, byte irq);

		void	InitTouch(byte orientation = LANDSCAPE);
		void	read();
		bool	dataAvailable();
		int16_t	getX();
		int16_t	getY();
		void	setPrecision(byte precision);

		void	calibrateRead();
    
    private:
		regtype *P_CLK, *P_CS, *P_DIN, *P_DOUT, *P_IRQ;
		regsize B_CLK, B_CS, B_DIN, B_DOUT, B_IRQ;
		byte	T_CLK, T_CS, T_DIN, T_DOUT, T_IRQ;
		long	_default_orientation;
		byte	orient;
		byte	prec;
		byte	display_model;
		long	disp_x_size, disp_y_size, default_orientation;
		long	touch_x_left, touch_x_right, touch_y_top, touch_y_bottom;

		void	touch_WriteData(byte data);
		word	touch_ReadData();

#if defined(ENERGIA)
		volatile uint32_t* portOutputRegister(int value);
		volatile uint32_t* portInputRegister(int value);
#endif
};

#endif