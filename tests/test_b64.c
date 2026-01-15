#include <nyoravim/base64.h>
#include <nyoravim/mem.h>
#include <nyoravim/list.h>

#include <assert.h>
#include <string.h>

struct test_pair {
    const char* plain;
    const char* encoded;
};

static void free_pair(void* user, void* pair) { nv_free(pair); }

static void append_test_pair(struct nv_list* pairs, const char* plain, const char* encoded) {
    struct test_pair* pair = nv_alloc(sizeof(struct test_pair));

    pair->plain = plain;
    pair->encoded = encoded;

    nv_list_push_back(pairs, pair);
}

static void test_encode(const struct nv_list* pairs) {
    for (struct nv_list_node* node = pairs->head; node != NULL; node = node->next) {
        const struct test_pair* pair = node->value;

        size_t len = strlen(pair->plain);
        char* encoded = nv_base64_encode(pair->plain, len);

        assert(encoded);
        assert(strcmp(encoded, pair->encoded) == 0);

        nv_free(encoded);
    }
}

static void test_decode(const struct nv_list* pairs) {
    for (struct nv_list_node* node = pairs->head; node != NULL; node = node->next) {
        const struct test_pair* pair = node->value;

        size_t size = nv_base64_decode(pair->encoded, NULL);
        assert(strlen(pair->plain) == 0 || size != 0);

        char* block = nv_alloc(size + 1);
        block[size] = '\0';

        nv_base64_decode(pair->encoded, block);
        assert(strcmp(block, pair->plain) == 0);

        nv_free(block);
    }
}

static void test_invalid() {
    size_t size = nv_base64_decode("SGVs bG8gV29ybGQ=", NULL);
    assert(size == 0);

    size = nv_base64_decode("ab!@#c", NULL);
    assert(size == 0);
}

int main(int argc, const char** argv) {
    struct nv_list pairs;
    nv_list_init(&pairs);

    append_test_pair(&pairs, "", "");
    append_test_pair(&pairs, "f", "Zg==");
    append_test_pair(&pairs, "fo", "Zm8=");
    append_test_pair(&pairs, "foo", "Zm9v");
    append_test_pair(&pairs, "foob", "Zm9vYg==");
    append_test_pair(&pairs, "fooba", "Zm9vYmE=");
    append_test_pair(&pairs, "foobar", "Zm9vYmFy");

    test_encode(&pairs);
    test_decode(&pairs);
    test_invalid();

    nv_list_clear(&pairs, free_pair, NULL);
    return 0;
}
