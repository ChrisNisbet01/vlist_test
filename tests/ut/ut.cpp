#include "CppUTest/TestHarness.h"

extern "C"
{
#include <libubox/vlist.h>
#include <libubox/avl-cmp.h>

#include <stdlib.h>
#include <string.h>
}

typedef struct test_element_st
{
    struct vlist_node node;
    char * name;
    int some_value;
} test_element_st;

typedef struct test_st
{
    struct vlist_tree test_tree;
    size_t deleted_count;
} test_st;

static void
regular_vlist_update_callback(struct vlist_tree *tree, struct vlist_node *node_new, struct vlist_node *node_old)
{
    (void)node_new;
    struct test_st * t = container_of(tree, struct test_st, test_tree);
    struct test_element_st *el_old = container_of(node_old, struct test_element_st, node);

#if defined REAL_WORLD_CODE
    struct test_element_st *el_new = container_of(node_new, struct test_element_st, node);

    if (el_old != nullptr && el_new != nullptr)
    {
        /* Compare the old and the new to see if anything has changed. */
    }
    else if (el_new != nullptr)
    {
        /* This element is new to the tree. */
    }
#endif

    if (node_old != nullptr)
    {
        /* This node is either being deleted or replaced, so free it. */
        free(el_old);
        t->deleted_count++;
    }
}

static test_element_st *
add_node_to_test(test_st *t, char const * name)
{
    test_element_st *el = (test_element_st *)calloc(1, sizeof *el);

    el->name = strdup(name);

    vlist_add(&t->test_tree, &el->node, el->name);

    return el;
}

static void
update_start(test_st *t)
{
    vlist_update(&t->test_tree);
}

static void
update_complete(test_st *t)
{
    vlist_flush(&t->test_tree);
}

TEST_GROUP(vlist)
{
    test_st test;
    test_st * t;

    void setup() override
    {
        t = &test;
        memset(t, 0, sizeof(*t));
    }

    void teardown() override
    {
    }

};


TEST(vlist, inserted_nodes_are_found)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el_123;
    test_element_st * el_456;

    el_123 = add_node_to_test(t, "123");
    el_123->some_value = 1;
    el_456 = add_node_to_test(t, "456");
    el_456->some_value = 2;

    test_element_st * el_found;

    el_found = vlist_find(&t->test_tree, "123", el_found, node);
    CHECK_EQUAL(el_123, el_found);

    el_found = vlist_find(&t->test_tree, "456", el_found, node);
    CHECK_EQUAL(el_456, el_found);
}

TEST(vlist, unknown_nodes_are_not_found_in_populated_tree)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el;

    el = add_node_to_test(t, "123");
    el->some_value = 1;

    el = vlist_find(&t->test_tree, "789", el, node);
    CHECK_TRUE(el == nullptr);
}

TEST(vlist, unknown_nodes_are_not_found_in_unpopulated_tree)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el;

    el = vlist_find(&t->test_tree, "789", el, node);
    CHECK_TRUE(el == nullptr);
}

TEST(vlist, nodes_with_version_updated_are_retained)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el_123;
    test_element_st * el_456;

    el_123 = add_node_to_test(t, "123");
    el_123->some_value = 1;
    el_456 = add_node_to_test(t, "456");
    el_456->some_value = 2;

    test_element_st  * el_update;

    update_start(t);
    vlist_for_each_element(&t->test_tree, el_update, node)
    {
        el_update->node.version = t->test_tree.version;
    }
    update_complete(t);

    test_element_st * el_found;

    el_found = vlist_find(&t->test_tree, "123", el_found, node);
    CHECK_EQUAL(el_123, el_found);

    el_found = vlist_find(&t->test_tree, "456", el_found, node);
    CHECK_EQUAL(el_456, el_found);
}

TEST(vlist, nodes_that_arent_readded_during_update_are_removed)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el;

    el = add_node_to_test(t, "123");
    el->some_value = 1;
    el = add_node_to_test(t, "456");
    el->some_value = 2;

    update_start(t);
    update_complete(t);

    el = vlist_find(&t->test_tree, "123", el, node);
    CHECK_TRUE(el == nullptr);

    el = vlist_find(&t->test_tree, "456", el, node);
    CHECK_TRUE(el == nullptr);
    CHECK_EQUAL(2, t->deleted_count);
}

TEST(vlist, updated_nodes_replace_existing_nodes)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el;
    char const * node_name = "123";
    int expected_value = 1;
    el = add_node_to_test(t, node_name);
    el->some_value = expected_value - 1;

    update_start(t);
    el = add_node_to_test(t, node_name);
    el->some_value = expected_value;
    update_complete(t);

    test_element_st  * el_found;

    el_found = vlist_find(&t->test_tree, "123", el_found, node);
    CHECK_EQUAL(el, el_found);
    CHECK_EQUAL(expected_value, el_found->some_value);
    CHECK_EQUAL(1, t->deleted_count);
}

TEST(vlist, deleted_nodes_are_removed)
{
    vlist_init(&t->test_tree, avl_strcmp, regular_vlist_update_callback);

    test_element_st * el;
    char const * node_name = "123";
    el = add_node_to_test(t, node_name);

    vlist_delete(&t->test_tree, &el->node);
    el = vlist_find(&t->test_tree, "123", el, node);
    CHECK_TRUE(el == nullptr);
    CHECK_EQUAL(1, t->deleted_count);
}

static void
test_keep_update(struct vlist_tree *tree, struct vlist_node *node_new, struct vlist_node *node_old)
{
    (void)tree;
    (void)node_new;
    (void)node_old;

    /* Do nothing. */
}

TEST(vlist, no_delete_mode_deleted_nodes_arent_deleted_by_vlist_delete)
{
    vlist_init(&t->test_tree, avl_strcmp, test_keep_update);
    t->test_tree.no_delete = true;

    test_element_st * el;
    char const * node_name = "123";
    el = add_node_to_test(t, node_name);

    vlist_delete(&t->test_tree, &el->node);

    test_element_st * el_found;

    /* It's up to the user to remove and delete the node from the tree, so a vlist_find should find the element. */
    el_found = vlist_find(&t->test_tree, "123", el, node);
    CHECK_TRUE(el_found != nullptr);

    avl_delete(&t->test_tree.avl, &el_found->node.avl);
    free(el_found);
}

TEST(vlist, no_delete_mode_deleted_nodes_arent_deleted_by_vlist_flush)
{
    vlist_init(&t->test_tree, avl_strcmp, test_keep_update);
    t->test_tree.no_delete = true;

    test_element_st * el;
    char const * node_name = "123";
    el = add_node_to_test(t, node_name);

    update_start(t);
    update_complete(t);

    test_element_st * el_found;

    /* It's up to the user to remove and delete the node from the tree, so a vlist_find should find the element. */
    el_found = vlist_find(&t->test_tree, "123", el, node);
    CHECK_EQUAL(el, el_found);

    avl_delete(&t->test_tree.avl, &el_found->node.avl);
    free(el_found);
}

TEST(vlist, no_delete_mode_new_nodes_arent_added)
{
    vlist_init(&t->test_tree, avl_strcmp, test_keep_update);
    t->test_tree.no_delete = true;

    test_element_st * el_original;
    char const * node_name = "123";
    el_original = add_node_to_test(t, node_name);

    update_start(t);
    test_element_st * el_new;

    el_new = add_node_to_test(t, node_name);
    update_complete(t);

    test_element_st * el_found;

    el_found = vlist_find(&t->test_tree, node_name, el_found, node);
    CHECK_EQUAL(el_found, el_original);

    /* Users are responsible for deleting the node that wasn't added to the tree. */
    free(el_new);
}

TEST(vlist, keep_old_mode_old_nodes_are_deleted_on_flush)
{
    vlist_init(&t->test_tree, avl_strcmp, test_keep_update);
    t->test_tree.keep_old = true;

    char const * node_name = "123";
    add_node_to_test(t, node_name);

    update_start(t);
    update_complete(t);

    /* The update callback is expected to free the node during flush (i.e. node_new will be NULL). */
    test_element_st * el_found = vlist_find(&t->test_tree, node_name, el_found, node);
    CHECK_EQUAL(nullptr, el_found);

}

TEST(vlist, keep_old_mode_old_nodes_are_deleted_on_vlist_delete)
{
    vlist_init(&t->test_tree, avl_strcmp, test_keep_update);
    t->test_tree.keep_old = true;

    test_element_st * el_original;
    char const * node_name = "123";
    el_original = add_node_to_test(t, node_name);

    vlist_delete(&t->test_tree, &el_original->node);

    test_element_st * el_found = vlist_find(&t->test_tree, node_name, el_found, node);
    CHECK_EQUAL(nullptr, el_found);

}

TEST(vlist, keep_old_mode_old_nodes_are_kept_on_add)
{
    vlist_init(&t->test_tree, avl_strcmp, test_keep_update);
    t->test_tree.keep_old = true;

    test_element_st * el_original;
    char const * node_name = "123";
    el_original = add_node_to_test(t, node_name);

    /*
     * In this case the user is expected to free the new node.
     * As it won't be added to the tree the user should not call vlist_delete() on the new node, and
     * is expected to free it himself (i.e. outside the update callback).
     */
    update_start(t);
    test_element_st * el_new = add_node_to_test(t, node_name);
    update_complete(t);

    test_element_st * el_found = vlist_find(&t->test_tree, node_name, el_found, node);
    CHECK_EQUAL(el_original, el_found);

    free(el_new);
}
