/* vga.h
 * Copyright (c) 2020 Jason Whitehorn
 * Copyright (c) 2019 Benson Chau, ngreenwald3, DannyFannyPack, tmaddali, DanielWygant
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

 #ifndef __INCLUDE_VGA_H
 #define __INCLUDE_VGA_H


#define VGA_ATTR_ADDR_REG        0x3C0
#define VGA_ATTR_DATA_REG        0x3C1
#define VGA_MISC_REG             0x3C2
#define VGA_SEQUENCER_ADDR_REG   0x3C4
#define VGA_SEQUENCER_DATA_REG   0x3C5
#define VGA_GRAPHICS_ADDR_REG    0x3CE
#define VGA_GRAPHICS_DATA_REG    0x3CF
#define VGA_CRTC_ADDR_REG        0x3D4
#define VGA_CRTC_DATA_REG        0x3D5
#define VGA_INSTAT_READ          0x3DA
#define VGA_SEQ_LEN              0x5
#define VGA_CRTC_LEN             0x19
#define VGA_GRAPHICS_LEN         0x9
#define VGA_ATTR_LEN             0xD
#define VGA_TEXT_MEM             0xB8000
#define VGA_VIDEO_MEM_ADDR       0xA0000

void vga_write_regs(uchar *mode);
void vga_setpixel4p(uint32 x, uint32 y, uint32 c);


#endif
