#include <stdio.h>
#include <stdlib.h> // for malloc, realloc and free
#define JSONCANVAS_IMPLEMENTATION
#include "jsoncanvas.h"

int main()
{
    jcanvas canvas;
    bool ok = jcanvas_init(&canvas);
    if (!ok) { jcanvas_destroy(&canvas); return -1; }
    
    // assings a UUID by default
    jcanvas_node* a = jcanvas_text_node(&canvas, "nodea", "# Node a\nThis ```text``` is interpreted as _*markdown*_!");
    jcanvas_pos_node(a, 0, 0, 100, 100);
    jcanvas_node* b = jcanvas_text_node(&canvas, "nodeb", "# Node b\nNodes can be connected by calling ```jcanvas_connect``` with two nodes as a paramter");
    jcanvas_pos_node(b, 300, 100, 100, 100);

    // automatically infers link side (e.g. left, right, top, bottom)
    jcanvas_edge* c = jcanvas_connect(&canvas, a, b);
    // but you can change the sides like this:
    // c->from_side = SIDE_BOTTOM; c->to_side = SIDE_TOP;

    a->color = jcanvas_yellow;
    c->color = jcanvas_orange;
    b->color = jcanvas_red;

    c->from_end = END_NONE; c->to_end = END_ARROW;

    str result = jcanvas_generate(&canvas);
    printf("%s", result.data);
    return 0;
}