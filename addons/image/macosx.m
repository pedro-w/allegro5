#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <CoreImage/CoreImage.h>

#include "allegro5/allegro.h"
#include "allegro5/fshook.h"
#include "allegro5/allegro_image.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_image.h"

#include "iio.h"

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
typedef float CGFloat;
#endif

ALLEGRO_DEBUG_CHANNEL("OSXIIO")

// Just to make sure it's never al_malloc.
#define apple_malloc malloc

static ALLEGRO_BITMAP *really_load_image(char *buffer, int size, int flags)
{
   NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
   ALLEGRO_BITMAP *bmp = NULL;
   /* Note: buffer is now owned (and later freed) by the data object. */
   NSData *nsdata = [NSData dataWithBytesNoCopy:buffer length:size];
   CIImage* image = [CIImage imageWithData:nsdata];

   BOOL premul = (flags & ALLEGRO_NO_PREMULTIPLIED_ALPHA) ? NO : YES;

   if (!image)
      goto done;
   NSNumber* value = [NSNumber numberWithBool:premul];
   NSDictionary<CIContextOption, id>* options = [NSDictionary dictionaryWithObject: value
                                                                            forKey:kCIContextOutputPremultiplied];
   CIContext* context = [CIContext contextWithOptions:options];

   /* Get the actual size in pixels from the representation */

   int w = [image extent].size.width;
   int h = [image extent].size.height;

   ALLEGRO_DEBUG("Read image of size %dx%d\n", w, h);
   uint8_t* surf = al_calloc(w*h*4, sizeof(uint8_t));
   [context render:image
          toBitmap:surf
          rowBytes:w*4
            bounds:CGRectMake(0,0,w,h)
            format:kCIFormatRGBA8 colorSpace:nil];

   /* Then create a bitmap out of the memory buffer. */
   bmp = al_create_bitmap(w, h);
   if (bmp) {
      ALLEGRO_LOCKED_REGION *lock = al_lock_bitmap(bmp,
                                                   ALLEGRO_PIXEL_FORMAT_ABGR_8888, ALLEGRO_LOCK_WRITEONLY);
      int j;
      uint8_t* dest = (uint8_t*) lock->data;
      uint8_t* src = surf;
      for (j=0; j<h; ++j) {
         memcpy(dest, src, w*4);
         dest += lock->pitch;
         src += w*4;
      }

      al_unlock_bitmap(bmp);
   }
   al_free(surf);

done:
   [pool drain];
   return bmp;
}

static ALLEGRO_BITMAP *_al_osx_load_image_f(ALLEGRO_FILE *f, int flags)
{
   ALLEGRO_BITMAP *bmp;
   ASSERT(f);

   int64_t size = al_fsize(f);
   if (size <= 0) {
      // TODO: Read from stream until we have the whole image
      ALLEGRO_ERROR("Couldn't determine file size.\n");
      return NULL;
   }
   /* Note: This *MUST* be the Apple malloc and not any wrapper, as the
    * buffer will be owned and freed by the NSData object not us.
    */
   void *buffer = apple_malloc(size);
   al_fread(f, buffer, size);

   /* Really load the image now. */
   bmp = really_load_image(buffer, size, flags);
   return bmp;
}


static ALLEGRO_BITMAP *_al_osx_load_image(const char *filename, int flags)
{
   ALLEGRO_FILE *fp;
   ALLEGRO_BITMAP *bmp;

   ASSERT(filename);

   ALLEGRO_DEBUG("Using native loader to read %s\n", filename);
   fp = al_fopen(filename, "rb");
   if (!fp) {
      ALLEGRO_ERROR("Unable open %s for reading.\n", filename);
      return NULL;
   }

   bmp = _al_osx_load_image_f(fp, flags);

   al_fclose(fp);

   return bmp;
}


extern NSImage* NSImageFromAllegroBitmap(ALLEGRO_BITMAP* bmp);

bool _al_osx_save_image_f(ALLEGRO_FILE *f, const char *ident, ALLEGRO_BITMAP *bmp)
{
   NSBitmapImageFileType type;

   if (!strcmp(ident, ".bmp")) {
      type = NSBMPFileType;
   }
   else if (!strcmp(ident, ".jpg") || !strcmp(ident, ".jpeg")) {
      type = NSJPEGFileType;
   }
   else if (!strcmp(ident, ".gif")) {
      type = NSGIFFileType;
   }
   else if (!strcmp(ident, ".tif") || !strcmp(ident, ".tiff")) {
      type = NSTIFFFileType;
   }
   else if (!strcmp(ident, ".png")) {
      type = NSPNGFileType;
   }
   else {
      ALLEGRO_ERROR("Unsupported image format: %s.\n", ident);
      return false;
   }

   NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];

   NSImage *image = NSImageFromAllegroBitmap(bmp);
   NSArray *reps = [image representations];
   NSData *nsdata = [NSBitmapImageRep representationOfImageRepsInArray: reps usingType: type properties: [NSDictionary dictionary]];

   size_t size = (size_t)[nsdata length];
   bool ret = al_fwrite(f, [nsdata bytes], size) == size;

   [image release];

   [pool drain];

   return ret;
}


bool _al_osx_save_image(const char *filename, ALLEGRO_BITMAP *bmp)
{
   ALLEGRO_FILE *fp;
   bool retsave = false;
   bool retclose = false;

   fp = al_fopen(filename, "wb");
   if (fp) {
      ALLEGRO_PATH *path = al_create_path(filename);
      if (path) {
         retsave = _al_osx_save_image_f(fp, al_get_path_extension(path), bmp);
         al_destroy_path(path);
      }
      retclose = al_fclose(fp);
   }
   else {
      ALLEGRO_ERROR("Unable open %s for writing.\n", filename);
   }

   return retsave && retclose;
}


bool _al_osx_save_png_f(ALLEGRO_FILE *f, ALLEGRO_BITMAP *bmp)
{
   return _al_osx_save_image_f(f, ".png", bmp);
}

bool _al_osx_save_jpg_f(ALLEGRO_FILE *f, ALLEGRO_BITMAP *bmp)
{
   return _al_osx_save_image_f(f, ".jpg", bmp);
}

bool _al_osx_save_tif_f(ALLEGRO_FILE *f, ALLEGRO_BITMAP *bmp)
{
   return _al_osx_save_image_f(f, ".tif", bmp);
}

bool _al_osx_save_gif_f(ALLEGRO_FILE *f, ALLEGRO_BITMAP *bmp)
{
   return _al_osx_save_image_f(f, ".gif", bmp);
}


bool _al_osx_register_image_loader(void)
{
   NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
   bool success = false;
   int num_types;
   int i;

   /* Get a list of all supported image types */
   NSArray *file_types = [NSImage imageFileTypes];
   num_types = [file_types count];
   for (i = 0; i < num_types; i++) {
      NSString *str = @".";
      NSString *type_str = [str stringByAppendingString: [file_types objectAtIndex: i]];
      const char *s = [type_str UTF8String];

      /* Skip image types Allegro supports built-in */
      if (!_al_stricmp(s, ".tga") || !_al_stricmp(s, ".bmp") || !_al_stricmp(s, ".pcx")) {
         continue;
      }

      /* Unload previous loader, if any */
      al_register_bitmap_loader(s, NULL);
      al_register_bitmap_loader_f(s, NULL);

      ALLEGRO_DEBUG("Registering native loader for bitmap type %s\n", s);
      success |= al_register_bitmap_loader(s, _al_osx_load_image);
      success |= al_register_bitmap_loader_f(s, _al_osx_load_image_f);
   }

   char const *extensions[] = { ".tif", ".tiff", ".gif", ".png", ".jpg", ".jpeg", NULL };

   for (i = 0; extensions[i]; i++) {
      ALLEGRO_DEBUG("Registering native saver for bitmap type %s\n", extensions[i]);
      success |= al_register_bitmap_saver(extensions[i], _al_osx_save_image);
   }

   success |= al_register_bitmap_saver_f(".tif", _al_osx_save_tif_f);
   success |= al_register_bitmap_saver_f(".tiff", _al_osx_save_tif_f);
   success |= al_register_bitmap_saver_f(".gif", _al_osx_save_gif_f);
   success |= al_register_bitmap_saver_f(".png", _al_osx_save_png_f);
   success |= al_register_bitmap_saver_f(".jpg", _al_osx_save_jpg_f);
   success |= al_register_bitmap_saver_f(".jpeg", _al_osx_save_jpg_f);

   [pool drain];

   return success;
}
