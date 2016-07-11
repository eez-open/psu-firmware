case ILI9341_8:
LCD_Write_COM_8(0xcf); 
LCD_Write_DATA_8(0x00);
LCD_Write_DATA_8(0xc1);
LCD_Write_DATA_8(0x30);

LCD_Write_COM_8(0xed); 
LCD_Write_DATA_8(0x64);
LCD_Write_DATA_8(0x03);
LCD_Write_DATA_8(0x12);
LCD_Write_DATA_8(0x81);

LCD_Write_COM_8(0xcb); 
LCD_Write_DATA_8(0x39);
LCD_Write_DATA_8(0x2c);
LCD_Write_DATA_8(0x00);
LCD_Write_DATA_8(0x34);
LCD_Write_DATA_8(0x02);

LCD_Write_COM_8(0xea); 
LCD_Write_DATA_8(0x00);
LCD_Write_DATA_8(0x00);

LCD_Write_COM_8(0xe8); 
LCD_Write_DATA_8(0x85);
LCD_Write_DATA_8(0x10);
LCD_Write_DATA_8(0x79);

LCD_Write_COM_8(0xC0); //Power control
LCD_Write_DATA_8(0x23); //VRH[5:0]

LCD_Write_COM_8(0xC1); //Power control
LCD_Write_DATA_8(0x11); //SAP[2:0];BT[3:0]

LCD_Write_COM_8(0xC2);
LCD_Write_DATA_8(0x11);

LCD_Write_COM_8(0xC5); //VCM control
LCD_Write_DATA_8(0x3d);
LCD_Write_DATA_8(0x30);

LCD_Write_COM_8(0xc7); 
LCD_Write_DATA_8(0xaa);

LCD_Write_COM_8(0x3A); 
LCD_Write_DATA_8(0x55);

LCD_Write_COM_8(0x36); // Memory Access Control
LCD_Write_DATA_8(0x08);

LCD_Write_COM_8(0xB1); // Frame Rate Control
LCD_Write_DATA_8(0x00);
LCD_Write_DATA_8(0x11);

LCD_Write_COM_8(0xB6); // Display Function Control
LCD_Write_DATA_8(0x0a);
LCD_Write_DATA_8(0xa2);

LCD_Write_COM_8(0xF2); // 3Gamma Function Disable
LCD_Write_DATA_8(0x00);

LCD_Write_COM_8(0xF7);
LCD_Write_DATA_8(0x20);

LCD_Write_COM_8(0xF1);
LCD_Write_DATA_8(0x01);
LCD_Write_DATA_8(0x30);

LCD_Write_COM_8(0x26); //Gamma curve selected
LCD_Write_DATA_8(0x01);

LCD_Write_COM_8(0xE0); //Set Gamma
LCD_Write_DATA_8(0x0f);
LCD_Write_DATA_8(0x3f);
LCD_Write_DATA_8(0x2f);
LCD_Write_DATA_8(0x0c);
LCD_Write_DATA_8(0x10);
LCD_Write_DATA_8(0x0a);
LCD_Write_DATA_8(0x53);
LCD_Write_DATA_8(0xd5);
LCD_Write_DATA_8(0x40);
LCD_Write_DATA_8(0x0a);
LCD_Write_DATA_8(0x13);
LCD_Write_DATA_8(0x03);
LCD_Write_DATA_8(0x08);
LCD_Write_DATA_8(0x03);
LCD_Write_DATA_8(0x00);

LCD_Write_COM_8(0xE1); //Set Gamma
LCD_Write_DATA_8(0x00);
LCD_Write_DATA_8(0x00);
LCD_Write_DATA_8(0x10);
LCD_Write_DATA_8(0x03);
LCD_Write_DATA_8(0x0f);
LCD_Write_DATA_8(0x05);
LCD_Write_DATA_8(0x2c);
LCD_Write_DATA_8(0xa2);
LCD_Write_DATA_8(0x3f);
LCD_Write_DATA_8(0x05);
LCD_Write_DATA_8(0x0e);
LCD_Write_DATA_8(0x0c);
LCD_Write_DATA_8(0x37);
LCD_Write_DATA_8(0x3c);
LCD_Write_DATA_8(0x0F);
LCD_Write_COM_8(0x11); //Exit Sleep
delay(120);
LCD_Write_COM_8(0x29); //display on
LCD_Write_COM_8(0x2c); //display on
delay(50);
break;


