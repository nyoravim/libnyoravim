#include <nyoravim/map.h>
#include <nyoravim/util.h>

#include <string.h>
#include <assert.h>

static size_t hash(void* user, const void* key) { return nv_hash_string(key); }
static bool equals(void* user, const void* lhs, const void* rhs) { return strcmp(lhs, rhs) == 0; }

static const char* s_string_keys[] = { "foo", "bar", "baz", "grep", "bash", "vim", "clang" };

static void populate_string_map(nv_map_t* map) {
    size_t key_count = sizeof(s_string_keys) / sizeof(*s_string_keys);
    for (size_t i = 0; i < key_count; i++) {
        const char* key = s_string_keys[i];

        /* we can cast to void* because we dont free keys */
        assert(nv_map_insert(map, (void*)key, (void*)i));
    }

    assert(nv_map_size(map) == key_count);
}

static void test_string_map(const nv_map_t* map) {
    size_t key_count = sizeof(s_string_keys) / sizeof(*s_string_keys);
    for (size_t i = 0; i < key_count; i++) {
        const char* key = s_string_keys[i];
        assert(nv_map_contains(map, key));

        void* value;
        assert(nv_map_get(map, key, &value));
        assert(value == (void*)i);
    }

    struct nv_map_pair data[key_count];
    nv_map_enumerate(map, data);

    for (size_t i = 0; i < key_count; i++) {
        struct nv_map_pair* pair = &data[i];
        size_t index = (size_t)pair->value;

        const char* expected = s_string_keys[index];
        assert(strcmp(pair->key, expected) == 0);
    }
}

static void test_string_keys(size_t buckets) {
    struct nv_map_callbacks callbacks;
    memset(&callbacks, 0, sizeof(struct nv_map_callbacks));

    callbacks.hash = hash;
    callbacks.equals = equals;

    nv_map_t* map = nv_map_alloc(buckets, &callbacks);
    assert(map);

    populate_string_map(map);
    test_string_map(map);

    nv_map_reserve(map, buckets * 3 / 2);
    test_string_map(map);

    nv_map_free(map);
}

static void test_integer_keys(size_t buckets) {
    nv_map_t* map = nv_map_alloc(buckets, NULL);
    assert(map);

    size_t element_count = 512;
    for (size_t i = 1; i <= element_count; i++) {
        void* key = (void*)i;
        void* value = (void*)(i * 7);

        assert(nv_map_insert(map, key, value));
    }

    assert(nv_map_size(map) == element_count);
    for (size_t i = 1; i < element_count; i++) {
        void* key = (void*)i;
        assert(nv_map_contains(map, key));

        void* value;
        assert(nv_map_get(map, key, &value));

        void* expected = (void*)(i * 7);
        assert(value == expected);
    }

    nv_map_free(map);
}

static void test_invalid_map_size() {
    nv_map_t* map = nv_map_alloc(0, NULL);
    assert(!map);
}

static void test_bad_insert() {
    nv_map_t* map = nv_map_alloc(8, NULL);
    assert(map);

    assert(nv_map_insert(map, NULL, (void*)1));
    assert(!nv_map_insert(map, NULL, (void*)2));

    void* value;
    assert(nv_map_get(map, NULL, &value));
    assert(value == (void*)1);

    nv_map_free(map);
}

static void test_bad_set() {
    nv_map_t* map = nv_map_alloc(8, NULL);
    assert(map);

    assert(!nv_map_set(map, NULL, (void*)1));
    assert(!nv_map_contains(map, NULL));
}

static void test_map_set_and_insert() {
    nv_map_t* map = nv_map_alloc(8, NULL);
    assert(map);

    assert(!nv_map_contains(map, NULL));
    assert(nv_map_insert(map, NULL, (void*)1));
    assert(nv_map_contains(map, NULL));
    
    void* value;
    assert(nv_map_get(map, NULL, &value));
    assert(value == (void*)1);

    assert(nv_map_set(map, NULL, (void*)2));
    assert(nv_map_get(map, NULL, &value));
    assert(value == (void*)2);

    nv_map_free(map);
}

static void test_map_put() {
    nv_map_t* map = nv_map_alloc(8, NULL);
    assert(map);

    assert(!nv_map_contains(map, NULL));

    /* returns true because it is inserting, not setting */
    assert(nv_map_put(map, NULL, (void*)1));

    void* value;
    assert(nv_map_get(map, NULL, &value));
    assert(value == (void*)1);

    /* returns false because it is setting, not inserting */
    assert(!nv_map_put(map, NULL, (void*)2));

    assert(nv_map_get(map, NULL, &value));
    assert(value == (void*)2);
}

int main(int argc, const char** argv) {
    test_map_set_and_insert();
    test_map_put();

    test_invalid_map_size();
    test_bad_insert();
    test_bad_set();

    for (size_t i = 1; i <= 256; i++) {
        test_string_keys(i);
        test_integer_keys(i);
    }

    return 0;
}
