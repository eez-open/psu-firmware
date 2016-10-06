case ILI9341_8:
	LCD_Write_COM_8(0x2A); //column
	LCD_Write_DATA(x1>>8,x1);
	LCD_Write_DATA(x2>>8,x2);
	LCD_Write_COM_8(0x2B); //page
	LCD_Write_DATA(y1>>8,y1);
	LCD_Write_DATA(y2>>8,y2);
	LCD_Write_COM_8(0x2C); //write
	break;
