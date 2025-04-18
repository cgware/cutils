#include "dict.h"

#include "mem.h"

#define DICT_MAX_LOAD	   0.75f
#define DICT_RESIZE_FACTOR 2

dict_t *dict_init(dict_t *map, int capacity)
{
	if (map == NULL) {
		return NULL;
	}

	map->buckets = mem_calloc(capacity, sizeof(struct bucket));
	if (map->buckets == NULL) {
		return NULL;
	}

	map->capacity = capacity;
	map->count    = 0;
	map->first    = NULL;
	map->last     = (struct bucket *)&map->first;

	return map;
}

void dict_free(dict_t *map)
{
	if (map == NULL) {
		return;
	}

	mem_free(map->buckets, map->capacity * sizeof(struct bucket));
}

static struct bucket *resize_entry(dict_t *map, struct bucket *old_entry)
{
	u32 index = old_entry->hash % map->capacity;
	for (;;) {
		struct bucket *entry = &map->buckets[index];

		if (entry->key == NULL) {
			*entry = *old_entry;
			return entry;
		}

		index = (index + 1) % map->capacity;
	}
}

static void dict_resize(dict_t *map)
{
	struct bucket *old_buckets = map->buckets;
	int old_capacity	   = map->capacity;

	map->capacity *= DICT_RESIZE_FACTOR;
	map->buckets = mem_calloc(map->capacity, sizeof(struct bucket));

	map->last = (struct bucket *)&map->first;

	do {
		map->last->next = resize_entry(map, map->last->next);
		map->last	= map->last->next;
	} while (map->last->next != NULL);

	mem_free(old_buckets, old_capacity * sizeof(struct bucket));
}

#define DICT_HASH_INIT 2166136261u

static inline u32 hash_data(const byte *data, size_t size)
{
	size_t nblocks = size / 8;
	u64 hash       = DICT_HASH_INIT;
	for (size_t i = 0; i < nblocks; ++i) {
		hash ^= (u64)data[0] << 0 | (u64)data[1] << 8 | (u64)data[2] << 16 | (u64)data[3] << 24 | (u64)data[4] << 32 |
			(u64)data[5] << 40 | (u64)data[6] << 48 | (u64)data[7] << 56;
		hash *= 0xbf58476d1ce4e5b9;
		data += 8;
	}

	u64 last = size & 0xff;
	switch (size % 8) {
	case 7: last |= (u64)data[6] << 56; /* falls through */
	case 6: last |= (u64)data[5] << 48; /* falls through */
	case 5: last |= (u64)data[4] << 40; /* falls through */
	case 4: last |= (u64)data[3] << 32; /* falls through */
	case 3: last |= (u64)data[2] << 24; /* falls through */
	case 2: last |= (u64)data[1] << 16; /* falls through */
	case 1:
		last |= (u64)data[0] << 8;
		hash ^= last;
		hash *= 0xd6e8feb86659fd93;
	}

	return (u32)(hash ^ hash >> 32);
}

static struct bucket *find_entry(const dict_t *map, const void *key, size_t ksize, u32 hash)
{
	u32 index = hash % map->capacity;

	for (;;) {
		struct bucket *entry = &map->buckets[index];

		if (entry->key == NULL || (entry->ksize == ksize && entry->hash == hash && mem_cmp(entry->key, key, ksize) == 0)) {
			return entry;
		}

		index = (index + 1) % map->capacity;
	}
}

void dict_set(dict_t *map, const void *key, size_t ksize, void *val)
{
	if (map == NULL || key == NULL) {
		return;
	}

	if (map->count + 1 > DICT_MAX_LOAD * map->capacity) {
		dict_resize(map);
	}

	u32 hash	     = hash_data(key, ksize);
	struct bucket *entry = find_entry(map, key, ksize, hash);
	if (entry->key == NULL) {
		map->last->next = entry;
		map->last	= entry;
		entry->next	= NULL;

		++map->count;

		entry->key   = key;
		entry->ksize = ksize;
		entry->hash  = hash;
	}
	entry->value = val;
}

int dict_get(const dict_t *map, const void *key, size_t ksize, void **out_val)
{
	if (map == NULL || key == NULL) {
		return 1;
	}

	u32 hash	     = hash_data(key, ksize);
	struct bucket *entry = find_entry(map, key, ksize, hash);

	if (out_val != NULL) {
		*out_val = entry->value;
	}

	return entry->key == NULL;
}
