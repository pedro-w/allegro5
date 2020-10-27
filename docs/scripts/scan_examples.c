#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dawk.h"

#define SL_INITIAL_CAPACITY 64
#define EXAMPLES_PER_API 3

static dstr protos;

typedef struct {
  char **items;
  int count;
  int capacity;
} slist_t;

static void sl_init(slist_t *);
static void sl_free(slist_t *);
static void sl_append(slist_t *, const char *);

static void cleanup(void);
static void load_api_list(void);
static slist_t apis;

/* Table of which APIs are in which examples */
static int *table;
/* Lookup to index the best examples */
typedef struct {
   /* Index into table */
   int index;
   /* How many APIs are used in each example */
   int count;
} lookup_t;
static lookup_t *lookup;
static int compare(const void *pa, const void *pb) {
   return ((const lookup_t *) pa)->count - ((const lookup_t *) pb)->count;
}

int main(int argc, char* argv[])
{
  dstr line;
  int i, j;
  /* Handle --protos flag */
  if (argc >=3 && strcmp(argv[1], "--protos") == 0) {
     strcpy(protos, argv[2]);
     argc -= 2;
     memmove(&argv[1], &argv[3], argc * sizeof(char*));
  } else {
     strcpy(protos, "protos");
  }
  d_cleanup = cleanup;
  sl_init(&apis);
  load_api_list();
  table = calloc(apis.count * argc, sizeof(int));

  lookup = calloc(argc, sizeof(lookup_t));
  for (j = 0; j < argc; ++j) {
     lookup[j].index = j;
  }

  for (j = 1; j < argc; ++j) {
    d_open_input(argv[j]);
    while (d_getline(line)) {
      for (i = 0; i < apis.count; ++i) {
	if (strstr(line, apis.items[i])) {
	  int *ptr = &table[i + j * apis.count];
	  if (*ptr == 0) {
	    *ptr = d_line_num;
	    ++lookup[j].count;
	  }
	}
      }
    }
    d_close_input();
  }
  /* Sort the files */
  qsort(lookup, argc, sizeof(lookup_t), compare);
  /* Output the EXAMPLES_PER_API (three) 'best' examples */
  for (i = 0; i < apis.count; ++i) {
     int found = 0;
     for (j = 0; j < argc && found < EXAMPLES_PER_API; ++j) {
	int index = lookup[j].index;
	int line_num = table[i + index * apis.count];
	if (line_num != 0) {
	   if (found == 0) {
	      d_printf("%s: ", apis.items[i]);
	   }
	   ++found;
	   d_printf("%s:%d ", argv[index], line_num);
	}
     }
     if (found > 0) {
	d_print("\n");
     }
  }
  d_cleanup();
  return 0;
}

void cleanup(void)
{
  free(table);
  free(lookup);
  sl_free(&apis);
}

void sl_init(slist_t* s)
{
  s->items = NULL;
  s->count = s->capacity = 0;
}

void sl_append(slist_t *s, const char *item)
{
  if (s->count == s->capacity) {
    int capacity = s->capacity == 0 ? SL_INITIAL_CAPACITY : (s->capacity * 2);
    s->items = realloc(s->items, capacity * sizeof(char*));
    s->capacity = capacity;
  }
  s->items[s->count++] = strcpy(malloc(1+strlen(item)), item);
}

void sl_free(slist_t *s)
{
  int i;
  for (i = 0; i < s->count; ++i) {
    free(s->items[i]);
  }
  free(s->items);
  s->items = NULL;
  s->count = s->capacity = 0;
}

void load_api_list(void)
{
   dstr line;
   d_open_input(protos);
   while (d_getline(line)) {
      int i;
      bool found = false;
      char *ptr = line;
      strsep(&ptr, ":");
      for (i = apis.count - 1; i >=0; --i) {
	 if (strcmp(line, apis.items[i]) == 0) {
	    found = true;
	    break;
	 }
      }
      if (!found) {
	 sl_append(&apis, line);
      }
   }
  d_close_input();
}

/* Local Variables: */
/* c-basic-offset: 3 */
/* End: */
/* vim: set sts=3 sw=3 et: */
