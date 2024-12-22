#ifndef __SH1106_H__
#define __SH1106_H__

#ifdef LINUX
/*  Include linux related header files  */
#endif /*   LINUX   */

/* Refer Protocol Section on PgNo. 13 of the datasheet  */
#define OLED_CNT_BYTE_CMD_SINGLE    0x80  //  Co = 1, D/C = 0, (1000 0000)b
#define OLED_CNT_BYTE_CMD_STREAM    0x00  //  Co = 0, D/C = 0, (0000 0000)b
#define OLED_CNT_BYTE_DATA_SINGLE   0xC0 //  Co = 1, D/C = 1, (1100 0000)b
#define OLED_CNT_BYTE_DATA_STREAM   0x40 //  Co = 0, D/C = 1, (0100 0000)b

#define OLED_SET_DISPLAY_OFF        0xAE     // Alawys initialize with display off
#define OLED_SET_DISPLAY_CLK_DIV    0xD5 // DB CMD: Follow with 0x80
#define OLED_SET_MULTIPLEX_RATIO    0xA8 // follow with 0x3F for 128x64 bit display
#define OLED_SET_DISPLAY_OFFSET     0xD3  // DB CMD: Follow with 0x00
#define OLED_SET_DISPLAY_START_L    0x40 // Set the RAM Display line addr for COM0

#define OLED_SET_SEG_REMAP_L        0xA0 // SEG0 = COL0 == 00H
#define OLED_SET_SEG_REMAP_H        0xA1 // SEG0 = COL0 == 83H

#define OLED_SET_COM_OP_SCAN        0xC0   // Do not flip vertically, refer pg. 24
#define OLED_SET_COM_OP_SCAN_V      0xC8 // Flip vertically.

#define OLED_SET_COM_PAD_HW         0xDA // DB CMD: Follow with 0x12

#define OLED_SET_CONTRAST           0x81 // Follow with 0x80 (default), for more info refer pg 20.

#define OLED_SET_PRECHARGE          0xD9 // DB CMD: Follow with 0x22

#define OLED_SET_VCOM_LEVEL         0xDB // DB CMD: Follow with 0x35

#define OLED_SET_ENT_DISPLAY_RAM    0xA4 // Normal display status
#define OLED_SET_DISPLAY_NORMAL     0xA6

#define OLED_SET_PAGE_ADR           0xB0 // B0 -> Page 1..... B7 -> Page 2

#define OLED_SET_DISPLAY_ON         0xAF // Initialize DC-DC then use this command to turn on the display

#endif