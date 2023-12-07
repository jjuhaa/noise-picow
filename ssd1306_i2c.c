/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
//#include "raspberry26x32.h"
#include "ssd1306_i2c.h"

const uint8_t font[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Nothing
0x1e, 0x28, 0x48, 0x88, 0x48, 0x28, 0x1e, 0x00,  //A
0xfe, 0x92, 0x92, 0x92, 0x92, 0x92, 0xfe, 0x00,  //B
0x7e, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00,  //C
0xfe, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7e, 0x00,  //D
0xfe, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x00,  //E
0xfe, 0x90, 0x90, 0x90, 0x90, 0x80, 0x80, 0x00,  //F
0xfe, 0x82, 0x82, 0x82, 0x8a, 0x8a, 0xce, 0x00,  //G
0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0xfe, 0x00,  //H
0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,  //I
0x84, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00,  //J
0x00, 0xfe, 0x10, 0x10, 0x28, 0x44, 0x82, 0x00,  //K
0xfe, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,  //L
0xfe, 0x40, 0x20, 0x10, 0x20, 0x40, 0xfe, 0x00,  //M
0xfe, 0x40, 0x20, 0x10, 0x08, 0x04, 0xfe, 0x00,  //N
0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00,  //O
0xfe, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,  //P
0x7c, 0x82, 0x82, 0x92, 0x8a, 0x86, 0x7e, 0x00,  //Q
0xfe, 0x88, 0x88, 0x88, 0x8c, 0x8a, 0x70, 0x00,  //R
0x62, 0x92, 0x92, 0x92, 0x92, 0x0c, 0x00, 0x00,  //S
0x80, 0x80, 0x80, 0xfe, 0x80, 0x80, 0x80, 0x00,  //T
0xfc, 0x02, 0x02, 0x02, 0x02, 0x02, 0xfc, 0x00,  //U
0xf0, 0x08, 0x04, 0x02, 0x04, 0x08, 0xf0, 0x00,  //V
0xfe, 0x04, 0x08, 0x10, 0x08, 0x04, 0xfe, 0x00,  //W
0x00, 0x82, 0x44, 0x28, 0x28, 0x44, 0x82, 0x00,  //X
0x80, 0x40, 0x20, 0x1e, 0x20, 0x40, 0x80, 0x00,  //Y
0x82, 0x86, 0x9a, 0xa2, 0xc2, 0x82, 0x00, 0x00,  //Z
0x7c, 0x82, 0x82, 0x92, 0x82, 0x82, 0x7c, 0x00,  //0
0x00, 0x00, 0x42, 0xfe, 0x02, 0x00, 0x00, 0x00,  //1
0x0c, 0x92, 0x92, 0x92, 0x92, 0x62, 0x00, 0x00,  //2
0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00,  //3
0xfc, 0x04, 0x04, 0x1e, 0x04, 0x04, 0x00, 0x00,  //4
0xf2, 0x92, 0x92, 0x92, 0x92, 0x0c, 0x00, 0x00,  //5
0xfc, 0x12, 0x12, 0x12, 0x12, 0x12, 0x0c, 0x00,  //6
0x80, 0x80, 0x80, 0x86, 0x8c, 0xb0, 0xc0, 0x00,  //7
0x6c, 0x92, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00,  //8
0x60, 0x90, 0x90, 0x90, 0x90, 0x90, 0xfe, 0x00,  //9
};

/* Example code to talk to an SSD1306-based OLED display

   The SSD1306 is an OLED/PLED driver chip, capable of driving displays up to
   128x64 pixels.

   NOTE: Ensure the device is capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefore I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO PICO_DEFAULT_I2C_SDA_PIN (on Pico this is GP4 (pin 6)) -> SDA on display
   board
   GPIO PICO_DEFAULT_I2C_SCL_PIN (on Pico this is GP5 (pin 7)) -> SCL on
   display board
   3.3v (pin 36) -> VCC on display board
   GND (pin 38)  -> GND on display board
*/

void calc_render_area_buflen(struct render_area *area) {
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

void SSD1306_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(SSD1306_I2C_BLOCK, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num) {
    for (int i=0;i<num;i++)
        SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);

    i2c_write_blocking(SSD1306_I2C_BLOCK, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}

void SSD1306_init() {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t cmds[] = {
        SSD1306_SET_DISP,               // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                           // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        SSD1306_HEIGHT - 1,             // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number. 
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
        0x02,                           
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
        0x12,
#else
        0x02,
#endif
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
        0x80,                           // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,          // set pre-charge period
        0xF1,                           // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
        0x30,                           // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST,           // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,        // set charge pump
        0x14,                           // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01, // turn display on
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

void SSD1306_scroll(bool on) {
    // configure horizontal scrolling
    uint8_t cmds[] = {
        SSD1306_SET_HORIZ_SCROLL | 0x00,
        0x00, // dummy byte
        0x00, // start page 0
        0x00, // time interval
        0x03, // end page 3 SSD1306_NUM_PAGES ??
        0x00, // dummy byte
        0xFF, // dummy byte
        SSD1306_SET_SCROLL | (on ? 0x01 : 0) // Start/stop scrolling
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

void render(uint8_t *buf, struct render_area *area) {
    // update a portion of the display with a render area
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    
    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(buf, area->buflen);
}

void SetPixel(uint8_t *buf, int x,int y, bool on) {
    assert(x >= 0 && x < SSD1306_WIDTH && y >=0 && y < SSD1306_HEIGHT);

    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
    // byte 1 is x = 1, y=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    const int BytesPerRow = SSD1306_WIDTH ; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = buf[byte_idx];

    if (on)
        byte |=  1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    buf[byte_idx] = byte;
}

// Basic Bresenhams.
void DrawLine(uint8_t *buf, int x0, int y0, int x1, int y1, bool on) {

    int dx =  abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;
    int e2;

    while (true) {
        SetPixel(buf, x0, y0, on);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2*err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int GetFontIndex(uint8_t ch) {
    if (ch >= 'A' && ch <='Z') {
        return  ch - 'A' + 1;
    }
    else if (ch >= '0' && ch <='9') {
        return  ch - '0' + 27;
    }
    else return  0; // Not got that char so space.
}

static uint8_t reversed[sizeof(font)] = {0};

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void FillReversedCache() {
    // calculate and cache a reversed version of fhe font, because I defined it upside down...doh!
    for (int i=0;i<sizeof(font);i++)
        reversed[i] = reverse(font[i]);
}

void WriteChar(uint8_t *buf, int16_t x, int16_t y, uint8_t ch) {
    if (reversed[0] == 0) 
        FillReversedCache();

    if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y/8;

    ch = toupper(ch);
    int idx = GetFontIndex(ch);
    int fb_idx = y * 128 + x;

    for (int i=0;i<8;i++) {
        buf[fb_idx++] = reversed[idx * 8 + i];
    }
}

void WriteString(uint8_t *buf, int16_t x, int16_t y, char *str) {
    // Cull out any string off the screen
    if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
        return;

    while (*str) {
        WriteChar(buf, x, y, *str++);
        x+=8;
    }
}

void WriteStringSizeMult(uint8_t *buf, uint16_t x, uint16_t y, char *str, uint8_t scale) {
    int len = (int)strlen(str);
    
    for (int l=0; l<len; l++) {
        char ch = toupper(str[l]);
        int idx = GetFontIndex(ch);

        for (int i=0; i<8; i++) {
            for (int j=0; j<8; j++) {
                if (font[idx*8 + i] & 0x80>>j) {//0x1<<j) {//
                    DrawSquare(buf, x+i*scale, y+j*scale, scale, scale);
                }
            }
        }
        x += 8*scale + scale;
    }
}

void DrawSquare(uint8_t *buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (uint16_t i = 0; i < w; i++) {
        for (uint16_t j = 0; j < h; j++) {
            if (x+i >= SSD1306_WIDTH || y+j >= SSD1306_HEIGHT) {
                continue;
            }
            SetPixel(buf, x + i, y + j, true);
        }
    }
}
