#include <nyoravim/list.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static void test_push_back() {
    struct nv_list list;
    nv_list_init(&list);

    size_t initial_size = 5;
    for (size_t i = 0; i < initial_size; i++) {
        size_t value = i + 1;
        assert(nv_list_push_back(&list, (void*)value));
    }

    assert(list.head);
    assert(list.tail);

    size_t index = 0;
    struct nv_list_node* current = list.head;

    while (current) {
        size_t expected = index + 1;
        assert(current->value == (void*)expected);

        index++;
        current = current->next;
    }

    assert(index == initial_size);
    nv_list_clear(&list, NULL, NULL);
}

static void test_push_front() {
    struct nv_list list;
    nv_list_init(&list);

    size_t initial_size = 5;
    for (size_t i = 0; i < initial_size; i++) {
        size_t value = i + 1;
        assert(nv_list_push_front(&list, (void*)value));
    }

    assert(list.head);
    assert(list.tail);

    size_t reverse_index = 0;
    struct nv_list_node* current = list.tail;

    while (current) {
        size_t expected = reverse_index + 1;
        assert(current->value == (void*)expected);

        reverse_index++;
        current = current->prev;
    }

    assert(reverse_index == initial_size);
    nv_list_clear(&list, NULL, NULL);
}

static void test_remove_mid() {
    struct nv_list list;
    nv_list_init(&list);

    for (size_t i = 0; i < 3; i++) {
        assert(nv_list_push_back(&list, (void*)i));
    }

    assert(list.head);
    assert(list.head->value == (void*)0);

    struct nv_list_node* next = list.head->next;
    assert(next);
    assert(next->value == (void*)1);
    assert(nv_list_remove(&list, next));

    next = list.head->next;
    assert(next && next == list.tail);
    assert(next->prev && next->prev == list.head);
    assert(next->value == (void*)2);

    nv_list_clear(&list, NULL, NULL);
}

static void test_remove_head() {
    struct nv_list list;
    nv_list_init(&list);

    for (size_t i = 0; i < 3; i++) {
        assert(nv_list_push_back(&list, (void*)i));
    }

    assert(list.head);
    assert(!list.head->prev);

    struct nv_list_node* next = list.head->next;
    assert(next);

    assert(nv_list_remove(&list, list.head));
    assert(list.head == next);
    assert(!next->prev);

    nv_list_clear(&list, NULL, NULL);
}

static void test_remove_tail() {
    struct nv_list list;
    nv_list_init(&list);

    for (size_t i = 0; i < 3; i++) {
        assert(nv_list_push_back(&list, (void*)i));
    }

    assert(list.tail);
    assert(!list.tail->next);

    struct nv_list_node* prev = list.tail->prev;
    assert(prev);

    assert(nv_list_remove(&list, list.tail));
    assert(list.tail == prev);
    assert(!prev->next);

    nv_list_clear(&list, NULL, NULL);
}

static void test_insert_after_mid() {
    struct nv_list list;
    nv_list_init(&list);

    for (size_t i = 0; i < 4; i++) {
        assert(nv_list_push_back(&list, (void*)i));
    }

    assert(list.head);

    struct nv_list_node* next = list.head->next;
    assert(next);

    struct nv_list_node* after = next->next;
    assert(after);
    assert(after->prev == next);

    struct nv_list_node* inserted = nv_list_insert_after(&list, next, (void*)5);
    assert(inserted);

    assert(inserted->prev == next);
    assert(inserted->next == after);

    assert(next->next == inserted);
    assert(after->prev == inserted);

    nv_list_clear(&list, NULL, NULL);
}

static void test_insert_before_mid() {
    struct nv_list list;
    nv_list_init(&list);

    for (size_t i = 0; i < 4; i++) {
        assert(nv_list_push_back(&list, (void*)i));
    }

    assert(list.tail);

    struct nv_list_node* prev = list.tail->prev;
    assert(prev);

    struct nv_list_node* before = prev->prev;
    assert(before);
    assert(before->next == prev);

    struct nv_list_node* inserted = nv_list_insert_before(&list, prev, (void*)5);
    assert(inserted);

    assert(inserted->next == prev);
    assert(inserted->prev == before);

    assert(prev->prev == inserted);
    assert(before->next == inserted);

    nv_list_clear(&list, NULL, NULL);
}

static void test_insert_after_tail() {
    struct nv_list list;
    nv_list_init(&list);

    assert(nv_list_push_back(&list, (void*)0));

    struct nv_list_node* tail = list.tail;
    assert(tail);

    struct nv_list_node* inserted = nv_list_insert_after(&list, tail, (void*)1);
    assert(inserted);

    assert(list.tail == inserted);
    assert(tail->next == inserted);

    assert(inserted->prev == tail);
    assert(!inserted->next);

    nv_list_clear(&list, NULL, NULL);
}

static void test_insert_before_head() {
    struct nv_list list;
    nv_list_init(&list);

    assert(nv_list_push_back(&list, (void*)0));

    struct nv_list_node* head = list.head;
    assert(head);

    struct nv_list_node* inserted = nv_list_insert_before(&list, head, (void*)1);
    assert(inserted);

    assert(list.head == inserted);
    assert(head->prev == inserted);

    assert(inserted->next == head);
    assert(!inserted->prev);

    nv_list_clear(&list, NULL, NULL);
}

int main(int argc, const char** argv) {
    test_push_back();
    test_push_front();

    test_remove_mid();
    test_remove_head();
    test_remove_tail();

    test_insert_after_mid();
    test_insert_before_mid();

    test_insert_after_tail();
    test_insert_before_head();

    return 0;
}
