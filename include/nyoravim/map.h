#ifndef _NV_MAP_H
#define _NV_MAP_H

#include <stdbool.h>
#include <stddef.h>

struct nv_map_callbacks {
    void* user;

    bool (*equals)(void* user, const void* lhs, const void* rhs);
    size_t (*hash)(void* user, const void* key);

    void (*free_key)(void* user, void* key);
    void (*free_value)(void* user, void* value);
};

struct nv_map_pair {
    const void* key;
    void* value;
};

typedef struct nv_map nv_map_t;

/* allocate hashmap with "capacity" number of buckets. */
nv_map_t* nv_map_alloc(size_t capacity, const struct nv_map_callbacks* callbacks);

/* destroys all data in hashmap. */
void nv_map_free(nv_map_t* map);

/* ensures that "capacity" number of buckets are allocated in hashmap. */
void nv_map_reserve(nv_map_t* map, size_t capacity);

/* returns the total number of key/value pairs in hashmap. */
size_t nv_map_size(const nv_map_t* map);

/* retrieves all data in the map.
 * data must be able to contain an adequate number of elements as per map_size(const map_t*) */
void nv_map_enumerate(const nv_map_t* map, struct nv_map_pair* data);

/* checks if key exists in hashmap.
 * returns true if key exists; otherwise false. */
bool nv_map_contains(const nv_map_t* map, const void* key);

/* retrieves a value from the hashmap into "value."
 * returns true if value was retrieved; otherwise false. */
bool nv_map_get(const nv_map_t* map, const void* key, void** value);

/* inserts a value into map if the key doesnt already exist.
 * returns true if able to insert; otherwise false.
 * takes ownership of key and value if returning true. */
bool nv_map_insert(nv_map_t* map, void* key, void* value);

/* sets an existing key to a value.
 * returns true if able to set; otherwise false.
 * takes ownership of value if returning true. */
bool nv_map_set(nv_map_t* map, const void* key, void* value);

/* sets key equal to value, regardless of if key already exists.
 * returns true if key previously did not exist.
 * always takes ownership of value.
 * takes ownership of key if returning true. */
bool nv_map_put(nv_map_t* map, void* key, void* value);

/* returns true if key existed and was removed. */
bool nv_map_remove(nv_map_t* map, const void* key);

#endif
