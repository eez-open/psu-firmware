case ILI9341_16:
   LCD_Write_COM(0x2A);
   LCD_Write_DATA(x1>>8);
   LCD_Write_DATA(x1);
   LCD_Write_DATA(x2>>8);
   LCD_Write_DATA(x2);
   LCD_Write_COM(0x2B);
   LCD_Write_DATA(y1>>8);
   LCD_Write_DATA(y1);
   LCD_Write_DATA(y2>>8);
   LCD_Write_DATA(y2);	
   LCD_Write_COM(0x2c);
	break;
