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
test_update(struct vlist_tree * tree, struct vlist_node * node_new, struct vlist_node * node_old)
{
    (void)tree;

    if (node_new != NULL && node_old != NULL)
    {
        struct test_element_st *el_new = container_of(node_new, struct test_element_st, node);

        printf("ignoring and freeing new entry: %p\n", el_new);
        free(el_new);
    }
    else if (node_new != NULL)
    {
        struct test_element_st *el_new = container_of(node_new, struct test_element_st, node);

        printf("must be adding new node %p\n", el_new);
    }
    else if (node_old != NULL)
    {
        struct test_element_st *el_old = container_of(node_old, struct test_element_st, node);

        printf("must be removing the original entry: %p, so freeing it\n", el_old);
        free(el_old);
    }
}

static test_element_st *
add_node_to_test(test_st *t, char * name, int some_value)
{
    test_element_st *el = calloc(1, sizeof *el);

    el->name = strdup(name);
    el->some_value = some_value;

    vlist_add(&t->test_tree, &el->node, el->name);

    return el;
}

static void
print_tree(test_st *t, char const * message)
{
    test_element_st *el;
    size_t count = 0;

    printf("%s\n", message);
    fflush(stdout);
    vlist_for_each_element(&t->test_tree, el, node)
    {
        printf("tree contains: %s, value %d\n", el->name, el->some_value);
        count++;
    }
    if (count == 0)
    {
        printf("tree is empty\n");
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
    int * p;
    /*
     * Use this mode when new nodes should be ignored if a matching node already exists,
     * so the tree just contains the first node entered.
     * In this case the new node is simply free up.
     */
    vlist_init(&t->test_tree, avl_strcmp, test_update);
    t->test_tree.keep_old = true;

    test_element_st * el;
    test_element_st * el_new;
    test_element_st * lookup;

    el = add_node_to_test(t, "123", 123);
    p = &el->some_value;
    print_tree(t, "original values");

    printf("now update some info in the element\n");
    update_start(t);
    el_new = add_node_to_test(t, "123", 456);
    printf("%p should have been freed\n", el_new);
    update_complete(t);

    print_tree(t, "new values");

    lookup = vlist_find(&t->test_tree, "123", lookup, node);
    printf("lookup el: %p should equal original (%p), and value (%d) should also match the original (123)\n",
           p, &el->some_value, *p);

    /*
     * Deleting the node can be done the usual way, using either vlist_delete or by not updating the node between
     * vlist_update() and vlist_complete().
     * Note that although a new node isn't added if a matching once already exists, the version of the existing node
     * will be updated so it isn't removed when the update completes.
     */
    //vlist_delete(&t->test_tree.avl, &el->node.avl);
    update_start(t);
    update_complete(t);

    /* And now the tree should be empty. */
    print_tree(t, "expect the tree to be empty");

    return 0;
}
