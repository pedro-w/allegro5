#include "allegro5/allegro.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_image.h"
#include <stdlib.h>
#include <time.h>

#include "common.c"

const int W = 100;
const int H = 100;

/* Cycle through some different named positions for the windows */
static void make_position(ALLEGRO_MONITOR_INFO *info, int *px, int *py, const char **caption)
{
   static int type = 0;
   int x, y;
   int monitor_w, monitor_h;
   x = info->x1;
   y = info->y1;
   monitor_w = info->x2 - info->x1;
   monitor_h = info->y2 - info->y1;
   switch (type++ % 16) {
      case 0:
      case 4:
      case 8:
      case 12:
         *caption = "Center";
         *px = x + (monitor_w - W) / 2;
         *py = y + (monitor_h - H) / 2;
         break;
      case 1:
         *caption = "Edge";
         *px = x + (monitor_w - W) / 2;
         *py = y;
         break;
      case 5:
         *caption = "Edge";
         *px = x + (monitor_w - W);
         *py = y + (monitor_h - H) / 2;
         break;
      case 9:
         *caption = "Edge";
         *px = x + (monitor_w - W) / 2;
         *py = y + (monitor_h - H);
         break;
      case 13:
         *caption = "Edge";
         *px = x;
         *py = y + (monitor_h - H) / 2;
         break;
      case 2:
         *caption = "Thirds";
         *px = x + (monitor_w - W) / 3;
         *py = y + (monitor_h - H) / 3;
      break;
      case 6:
         *caption = "Thirds";
         *px = x + (monitor_w - W) * 2 / 3;
         *py = y + (monitor_h - H) / 3;
      break;
      case 10:
         *caption = "Thirds";
         *px = x + (monitor_w - W) / 3;
         *py = y + (monitor_h - H) * 2 / 3;
      break;
      case 14:
         *caption = "Thirds";
         *px = x + (monitor_w - W) * 2 / 3;
         *py = y + (monitor_h - H) * 2 / 3;
      break;
      case 3:
      case 7:
      case 11:
      case 15:
         *caption = "Random";
         *px = x + rand() % (monitor_w - W);
         *py = y + rand() % (monitor_h - H);
         break;
   }
}
int main(int argc, char **argv)
{
   ALLEGRO_DISPLAY *displays[2];
   const char* captions[2];
   ALLEGRO_MONITOR_INFO *info;
   int adapter_count;
   int x, y;
   ALLEGRO_FONT *myfont;
   ALLEGRO_EVENT_QUEUE *events;
   ALLEGRO_EVENT event;
   int i;

   (void)argc;
   (void)argv;

   srand(time(NULL));

   if (!al_init()) {
      abort_example("Could not init Allegro.\n");
   }

   al_install_mouse();
   al_init_font_addon();
   al_init_image_addon();

   adapter_count = al_get_num_video_adapters();

   if (adapter_count == 0) {
      abort_example("No adapters found!\n");
   }

   info = malloc(adapter_count * sizeof(ALLEGRO_MONITOR_INFO));

   for (i = 0; i < adapter_count; i++) {
      al_get_monitor_info(i, &info[i]);
   }

   x = ((info[0].x2 - info[0].x1) / 3) - (W / 2);
   y = ((info[0].y2 - info[0].y1) / 2) - (H / 2);

   al_set_new_window_position(x, y);

   displays[0] = al_create_display(W, H);
   captions[0] = "Click me...";
   x *= 2;
   al_set_new_window_position(x, y);

   displays[1] = al_create_display(W, H);
   captions[1] = "Click me...";
   if (!displays[0] || !displays[1]) {
      abort_example("Could not create displays.\n");
   }

   al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
   myfont = al_load_font("data/fixed_font.tga", 0, 0);
   if (!myfont) {
      abort_example("Could not load font.\n");
   }

   events = al_create_event_queue();
   al_register_event_source(events, al_get_mouse_event_source());
   al_register_event_source(events, al_get_display_event_source(displays[0]));
   al_register_event_source(events, al_get_display_event_source(displays[1]));

   for (;;) {
      for (i = 0; i < 2; i++) {
        al_set_target_backbuffer(displays[i]);
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
        if (i == 0)
           al_clear_to_color(al_map_rgb(255, 0, 255));
        else
           al_clear_to_color(al_map_rgb(155, 255, 0));
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
        al_draw_textf(myfont, al_map_rgb(0, 0, 0), 50, 50, ALLEGRO_ALIGN_CENTRE, "%s", captions[i]);
        al_flip_display();
      }

      if (al_wait_for_event_timed(events, &event, 1)) {
         if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
         }
         else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            int a = rand() % adapter_count;
            int d = event.mouse.display == displays[0] ? 0 : 1;
            make_position(&info[a], &x, &y, &captions[d]);
            al_set_window_position(event.mouse.display, x, y);
         }
      }
   }

   al_destroy_event_queue(events);

   al_destroy_display(displays[0]);
   al_destroy_display(displays[1]);

   free(info);

   return 0;
}

