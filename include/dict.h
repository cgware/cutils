#ifndef dict_h
#define dict_h

#include "type.h"

#include <stddef.h>

struct bucket {
	struct bucket *next;

	const void *key;
	size_t ksize;
	u32 hash;
	void *value;
};

typedef struct dict_s {
	struct bucket *buckets;
	int capacity;
	int count;

	struct bucket *first;
	struct bucket *last;
} dict_t;

typedef void (*dict_callback)(void *key, size_t ksize, void *value, void *priv);
typedef void (*dict_callback_c)(void *key, size_t ksize, void *value, const void *priv);
typedef void (*dict_callback_hc)(void *key, size_t ksize, void *value, void *priv);

dict_t *dict_init(dict_t *map, int capacity);
void dict_free(dict_t *map);

void dict_set(dict_t *map, const void *key, size_t ksize, void *value);

int dict_get(const dict_t *map, const void *key, size_t ksize, void **out_val);

#define dict_foreach(_dict, _bucket) for (struct bucket *_bucket = (_dict)->first; _bucket != NULL; _bucket = _bucket->next)

#endif
