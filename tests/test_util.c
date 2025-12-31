#include <nyoravim/util.h>

#include <assert.h>
#include <string.h>

static void test_strdup() {
    const char* test_str = "fooBarBAZ\n";

    char* duplicated = nv_strdup(test_str);
    assert(duplicated);
    assert(strcmp(test_str, duplicated) == 0);

    assert(nv_strdup(NULL) == NULL);
}

int main(int argc, const char** argv) {
    test_strdup();

    return 0;
}
