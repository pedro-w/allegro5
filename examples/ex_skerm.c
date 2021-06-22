#include <stdio.h>
#include <stdlib.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"

int main(int argc, char **argv)
{
  int i;
  al_init();
  al_init_image_addon();

  for (i=1; i<argc; ++i) {
    const char* f = argv[i];
    fprintf(stderr, "%s: ", f);
    ALLEGRO_BITMAP* b = al_load_bitmap(f);
    if (b) {
      fputs("LOAD\n", stderr);
      al_destroy_bitmap(b);
    } else {
      fputs("NOT\n", stderr);
    }
  }
  return 0;
}
    
