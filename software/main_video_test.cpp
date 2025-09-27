#include "chu_init.h"
#include "vga_core.h"
#include "ps2_core.h"
#include "gpio_cores.h"
#include "xadc_core.h"
#include <cstdio>

// Core instances
SpriteCore ghost(get_sprite_addr(BRIDGE_BASE, V3_GHOST), 1024);
FrameCore frame(FRAME_BASE);
SpriteCore mouse(get_sprite_addr(BRIDGE_BASE, V1_MOUSE), 1024);
OsdCore osd(get_sprite_addr(BRIDGE_BASE, V2_OSD));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
XadcCore adc(get_slot_addr(BRIDGE_BASE, S5_XDAC));  // XADC slot for pot

void osd_write_char(int x, int y, const char* str, int invert = 0) {
   int i = x;
   while (*str && i < OsdCore::CHAR_X_MAX) {
      osd.wr_char(i++, y, *str++, invert);
   }
}

int main() {
   int x = 320, y = 240;
   int pen_color = 0x0F0;
   const char* color_name = "Green";
   bool pen_down = true;
   char ch;

   float filtered_adc = 2048.0f;
   float alpha = 0.1f;

   // Set up OSD
   ghost.bypass(1);  // Disable ghost sprite
   frame.clr_screen(0x000);
   frame.bypass(0);
   osd.bypass(0);
   osd.set_color(0xFFF, 0x000);
   osd.clr_screen();

   // Configure cursor sprite
   for (int i = 0; i < 32 * 32; i++) {
      mouse.wr_mem(i, 0x000);  // fully transparent
   }

   // Crosshairs in top-left corner
   int offset = 2;
   for (int i = -2; i <= 2; i++) {
      mouse.wr_mem((offset + i) * 32 + offset, 0xFFF);  // vertical line
      mouse.wr_mem(offset * 32 + (offset + i), 0xFFF);  // horizontal line
   }
   mouse.wr_ctrl(0x00);

   // Title setup
   const char* title = "  Vince's Etch-A-Sketch  ";
   int title_start = (80 - 26) / 2;  // = 27
   osd_write_char(title_start, 1, "  ----------------------  ");

   // Static OSD labels
   osd_write_char(70, 8, "Controls:", 1);
   osd_write_char(1, 13, "Color: ");
   osd_write_char(8, 13, color_name);
   osd_write_char(70, 9, "1-Red");
   osd_write_char(70, 10, "2-Green");
   osd_write_char(70, 11, "3-Blue");
   osd_write_char(70, 12, "4-Yellow");
   osd_write_char(70, 13, "5-White");
   osd_write_char(70, 14, "6-Black");
   osd_write_char(70, 15, "Space=Pen");
   osd_write_char(70, 16, "C=Clear");
   osd_write_char(1, 14, "Pen Size: ");

   auto update_osd_color = [&]() {
      for (int i = 8; i < 25; i++)
         osd.wr_char(i, 13, OsdCore::NULL_CHAR);
      osd_write_char(8, 13, color_name);
   };
   update_osd_color();

   int blink_timer = 0;
   bool invert = false;

   while (1) {
      // Read keyboard input
      if (ps2.get_kb_ch(&ch)) {
         switch (ch) {
            case ' ': pen_down = !pen_down; break;
            case 'c': frame.clr_screen(0x000); break;
            case '1': pen_color = 0xF00; color_name = "Red";    update_osd_color(); break;
            case '2': pen_color = 0x0F0; color_name = "Green";  update_osd_color(); break;
            case '3': pen_color = 0x00F; color_name = "Blue";   update_osd_color(); break;
            case '4': pen_color = 0xFF0; color_name = "Yellow"; update_osd_color(); break;
            case '5': pen_color = 0xFFF; color_name = "White";  update_osd_color(); break;
            case '6': pen_color = 0x000; color_name = "Black";  update_osd_color(); break;
            default: break;
         }
      }

      // ADC smoothing and pen position logic
      uint16_t raw_adc_size = adc.read_raw(0) >> 4;
      static float filtered_hpos = 2048.0f;
      static float filtered_vpos = 2048.0f;
      float alpha_pos = 0.1f;
      uint16_t raw_adc_hpos = adc.read_raw(1) >> 4;
      uint16_t raw_adc_vpos = adc.read_raw(2) >> 4;
      filtered_hpos = alpha_pos * raw_adc_hpos + (1.0f - alpha_pos) * filtered_hpos;
      filtered_vpos = alpha_pos * raw_adc_vpos + (1.0f - alpha_pos) * filtered_vpos;

      filtered_adc = alpha * raw_adc_size + (1.0f - alpha) * filtered_adc;
      int size = 1 + ((int(filtered_adc) >> 8) & 0x0E);  // 1, 3, 5, ..., 15
      int half = size / 2;

      x = (int(filtered_hpos) * FrameCore::HMAX) / 4096;
      y = (int(filtered_vpos) * FrameCore::VMAX) / 4096;

      // Draw pen stroke
      if (pen_down) {
         for (int dy = -half; dy <= half; dy++) {
            for (int dx = -half; dx <= half; dx++) {
               int px = x + dx;
               int py = y + dy;
               if (px >= 0 && px < FrameCore::HMAX && py >= 0 && py < FrameCore::VMAX)
                  frame.wr_pix(px, py, pen_color);
            }
         }
      }

      // Display brush size
      char size_text[16];
      sprintf(size_text, "%2d", size);
      osd.wr_char(11, 14, OsdCore::NULL_CHAR);
      osd.wr_char(12, 14, OsdCore::NULL_CHAR);
      osd_write_char(11, 14, size_text);

      // Update cursor
      mouse.move_xy(x - half, y - half);

      // Title blink animation
      if (++blink_timer >= 50) {  // ~500ms
         blink_timer = 0;
         invert = !invert;
         osd_write_char(title_start, 0, title, invert);
      }

      sleep_ms(10);
   }
}
