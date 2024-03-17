#include <stdio.h>
#include <stdlib.h> // for malloc, realloc and free
#define JSONCANVAS_IMPLEMENTATION
#include "jsoncanvas.h"

#include <Windows.h>

int main()
{
    jcanvas canvas;
    bool ok = jcanvas_init(&canvas);
    if (!ok) { jcanvas_destroy(&canvas); return -1; }

    // assings a UUID by default
    jcanvas_node* a = jcanvas_text_node(&canvas, "nodea", "# Node a\\nThis ```text``` is interpreted as _*markdown*_!");
    jcanvas_pos_node(a, -600, 0, 400, 400);
    jcanvas_node* b = jcanvas_text_node(&canvas, "nodeb", "# Node b\\nNodes can be connected by calling ```jcanvas_connect``` with two nodes as a paramter");
    jcanvas_pos_node(b, 0, 0, 400, 400);

    // automatically infers link side (e.g. left, right, top, bottom)
    jcanvas_edge* c = jcanvas_connect(&canvas, a, b);
    // but you can change the sides like this:
    // c->from_side = SIDE_BOTTOM; c->to_side = SIDE_TOP;

    a->color = jcanvas_yellow;
    c->color = jcanvas_orange;
    b->color = jcanvas_red;
    
    jcanvas_node* file_node = jcanvas_file_node(&canvas, "readme", "README.md");
    // subpaths can be set like this
    // jcanvas_set_subpath(file_node, "example_subpath");
    jcanvas_pos_node(file_node, -200, 600, 200, 400);

    jcanvas_edge* e1 = jcanvas_connect(&canvas, a, file_node);
    jcanvas_edge* e2 = jcanvas_connect(&canvas, b, file_node);
    e1->from_end = END_NONE; e1->to_end = END_ARROW;
    e2->from_end = END_ARROW; e2->to_end = END_ARROW;
    e1->color = jcanvas_cyan; e2->color = jcanvas_purple;

    str result = jcanvas_generate(&canvas);
    printf("%s", result.data);

    jcanvas_destroy(&canvas);
    return 0;
}