#include <libubox/vlist.h>
#include <libubox/avl-cmp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct test_element_st
{
    struct vlist_node node;
    char * name;
    int some_value;
} test_element_st;

typedef struct test_st
{
    struct vlist_tree test_tree;
} test_st;

static void
test_update(struct vlist_tree *tree, struct vlist_node *node_new,
            struct vlist_node *node_old)
{
    (void)tree;
    struct test_element_st *el_old = container_of(node_old, struct test_element_st, node);
    struct test_element_st *el_new = container_of(node_new, struct test_element_st, node);

    if (node_old != NULL && node_new != NULL)
    {
        printf("replacing old entry\n");
    }
    else if (node_new != NULL)
    {
        printf("adding new entry\n");
    }
    if (node_old != NULL)
    {
        printf("freeing old entry\n");
        free(el_old);
    }
}

static test_element_st *
add_node_to_test(test_st *t, char * name)
{
    test_element_st *el = calloc(1, sizeof *el);

    el->name = strdup(name);

    vlist_add(&t->test_tree, &el->node, el->name);

    return el;
}

static void
print_tree(test_st *t, char const * message)
{
    test_element_st *el;

    printf("%s\n", message);
    vlist_for_each_element(&t->test_tree, el, node)
    {
        printf("tree contains: %s, value %d\n", el->name, el->some_value);
    }
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

int main(void)
{
    test_st test;
    test_st * t = &test;

    /*
     * Use this mode when the new node can completely replace the old node.
     * No resource either outside the element (other than the tree ),
     * or within the element (e.g. struct uloop_timeout) should refer to the old element.
     *
     * In this mode the 'update' callback is expected to free the old entry in the tree.
     */
    vlist_init(&t->test_tree, avl_strcmp, test_update);
    t->test_tree.no_delete = false;
    t->test_tree.keep_old = false;

    test_element_st * el;
    el = add_node_to_test(t, "123");
    el->some_value = 1;
    print_tree(t, "original values");

    update_start(t);
    el = add_node_to_test(t, "123");
    el->some_value = 2;
    update_complete(t);

    print_tree(t, "new values. Expect some_value to be 2");

    return 0;
}
