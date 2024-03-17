#pragma once
#include <stdint.h>
#include <stdbool.h>

// Define
#ifndef ALLOCATE
    #define ALLOCATE malloc
#endif
#ifndef REALLOC
    #define REALLOC realloc
#endif
#ifndef FREE
    #define FREE free
#endif

typedef struct {
    char* data;
    uint32_t len;
    uint32_t cap;
} str;

typedef str jcanvas_color;

typedef enum {
    STYLE_OVER,
    STYLE_RATIO,
    STYLE_REPEAT,
} jcanvas_background_style; 

typedef struct {
    str id;
    int64_t x, y, width, height;
    jcanvas_color color;

    enum jcanvas_node_type {
        NODE_TYPE_TEXT,
        NODE_TYPE_FILE,
        NODE_TYPE_LINK,
        NODE_TYPE_GROUP,
    } type;

    union {
        str text;
        struct {
            str file;
            str subpath;
        } file;
        str link;
        struct {
            str label;
            str background;
            jcanvas_background_style background_style;
        } group_node;
    } as;
} jcanvas_node;

typedef enum {
   SIDE_TOP,
   SIDE_RIGHT,
   SIDE_BOTTOM,
   SIDE_LEFT, 
} jcanvas_side;

typedef enum {
    END_NONE,
    END_ARROW,
} jcanvas_end;

typedef struct {
    str id;
    str from_node;
    jcanvas_side from_side;
    jcanvas_end from_end;
    jcanvas_side to_side;
    jcanvas_end to_end;
    str to_node;
    jcanvas_color color;
    str label;
} jcanvas_edge;

typedef struct map {
    struct map* left;
    struct map* right;
    struct map* parent;
    uint64_t hash;
    void* value;
    bool is_red;
} map;

typedef struct {
    map id_to_nodes;
    map id_to_edges;
    jcanvas_node* nodes;
    jcanvas_edge* edges;
    uint32_t node_count, edge_count;
    uint32_t node_cap, edge_cap;
    char* last_error;
} jcanvas;

bool jcanvas_init(jcanvas* result);
jcanvas_node* jcanvas_text_node_s(jcanvas* c, str id, str content);
jcanvas_node* jcanvas_text_node(jcanvas* c, char* id, char* content);
jcanvas_edge* jcanvas_connect(jcanvas* c, jcanvas_node* a, jcanvas_node* b);
void jcanvas_pos_node(jcanvas_node* node, int64_t x, int64_t y, int64_t width, int64_t height);
str jcanvas_generate(jcanvas* c);
void jcanvas_destroy(jcanvas* c);

#ifdef JSONCANVAS_IMPLEMENTATION


static const jcanvas_color jcanvas_red = {"1", 1};
static const jcanvas_color jcanvas_orange = {"2", 1};
static const jcanvas_color jcanvas_yellow = {"3", 1};
static const jcanvas_color jcanvas_green = {"4", 1};
static const jcanvas_color jcanvas_cyan = {"5", 1};
static const jcanvas_color jcanvas_purple = {"6", 1};

static const str _background_style_strings[] = {
    {"cover", 5},
    {"ratio", 5},
    {"repeat", 6},
};

static const str _side_strings[] = {
    {"top", 3},
    {"right", 5},
    {"bottom", 6},
    {"left", 4},
};

static const str _type_strings[] = {
    {"text", 4},
    {"file", 4},
    {"link", 4},
    {"group", 5},
};

static const str _end_strings[] = {
    {"none", 4},
    {"arrow", 5},
};

static bool ensure_capacity(uint32_t* cap, uint32_t new_cap, void** data, uint32_t size_of_type)
{
    if (new_cap <= *cap) return true;

    if (*cap == 0) *cap = new_cap;
    else {
        while (*cap <= new_cap) {
            *cap *= 1.5;
        }
    }
    void* result = REALLOC(*data, *cap * size_of_type);
    if (result == NULL) {
        return false;
    }
    *data = result;
    return true;
}

//#region helper_functions
str str_init(uint32_t starting_cap)
{
    str result;
    result.len = 0; result.cap = starting_cap;
    result.data = ALLOCATE(starting_cap);
    return result;
}

void copy_mem(char* from, char* to, uint32_t size)
{
    if (from == NULL || to == NULL) return;
    if (size == 0) return;
    for (int i = 0; i < size; i++) {
        to[i] = from[i];
    }
}

bool str_append_s(str* a, str b)
{
    if (b.len == 0) return true;
    bool ok = ensure_capacity(&a->cap, a->len + b.len, &a->data, 1);
    if (!ok) return false;
    copy_mem(b.data, &a->data[a->len], b.len);
    a->len += b.len;
    return true;
}

uint32_t str_len(char* string)
{
    char* c = string;
    uint32_t len = 0;
    while (*c != '\0') {
        c++;
        len++;
    }
    return len;
}

[[always_inline]] bool str_append(str* a, char* text, uint32_t len)
{
    if (len == 0) len = str_len(text);
    str str_to_append; str_to_append.data = text; str_to_append.len = len;   
    return str_append_s(a, str_to_append);
}

str str_concat(str a, str b)
{
    str result;
    result.len = a.len + b.len;
    result.data = ALLOCATE(result.len+1);
    copy_mem(a.data, result.data, a.len);
    copy_mem(b.data, result.data + a.len, b.len);
    result.data[result.len] = 0;
    return result;
}

[[always_inline]] str make_str_l(char* data, uint32_t len)
{
    return (str) {
        .data = data,
        .len = len
    };
}

[[always_inline]] str make_str(char* data)
{
    uint32_t len = str_len(data);
    return make_str_l(data, len);
}

uint64_t fnv1a(char* start, char* end)
{
    const uint64_t magic_prime = 0x00000100000001b3;
    uint64_t hash = 0xcbf29ce484222325;
    for (; start <= end; start++) {
        hash = (hash ^ *start) * magic_prime;
    }
    return hash;
}

static void rotate_right(map* x)
{
    map* y = x->left;
    x->left = y->right;
    if (y->right) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent) {
        if (x == (x->parent)->right) {
            x->parent->right = y;
        }
        else {
            x->parent->left = y;
        }
    }
    y->right = x;
    x->parent = y;
}

static void rotate_left(map* x)
{
    map* y = x->right;
    x->right = y->left;
    if (y->left) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent) {
        if (x == (x->parent)->left) {
            x->parent->left = y;
        }
        else {
            x->parent->right = y;
        }
    }
    y->left = x;
    x->parent = y;
}

static void rebalance_tree(map* cur)
{
    cur->is_red = true;
    while ( (cur->parent) && (cur->parent->is_red)) {
        if (cur->parent->parent == NULL) break;
        if (cur->parent == cur->parent->parent->left) {
            map* uncle = cur->parent->parent->right;
            if (uncle == NULL) break;
            if (uncle->is_red) {
                // case 1: switch colors
                cur->parent->is_red = false; uncle->is_red = false;
                cur->parent->parent->is_red = true;
                cur = cur->parent->parent;
            } else {
                if (cur == cur->parent->right) {
                    // case 2: move cur up and rotate
                    cur = cur->parent;
                    rotate_left(cur);
                }
                else {
                // case 3
                    cur->parent->is_red = false;
                    cur->parent->parent->is_red = true;
                    rotate_right(cur->parent->parent);
                }
            }
        } else {
            map* uncle = cur->parent->parent->left;
            if (uncle == NULL) break;
            if (uncle->is_red) {
                // case 1: switch colors
                cur->parent->is_red = false; uncle->is_red = false;
                cur->parent->parent->is_red = true;
                cur = cur->parent->parent;
            } else {
                if (cur == cur->parent->left) {
                    // case 2: move cur up and rotate
                    cur = cur->parent;
                    rotate_left(cur);
                }
                else {
                // case 3
                    cur->parent->is_red = false;
                    cur->parent->parent->is_red = true;
                    rotate_right(cur->parent->parent);
                }
            }
        }
    }
}

void* map_get(map* root, str key)
{
    if (root == NULL) return NULL;
    uint64_t hash = fnv1a(key.data, key.data + key.len);
    map* cur = root;
    while (true) {
        if (cur == NULL || cur->hash == hash) break;
        cur = hash < cur->hash ? cur->left : cur->right;
    }
    return cur ? cur->value : NULL;
}

void map_set(map* root, str key, void* value)
{
    uint64_t hash = fnv1a(key.data, key.data + key.len);
    map* cur = root;
    map** place_to_insert = NULL;
    while (true) {
        if (hash == cur->hash) {
            cur->value = value;
            return;
        } else if (cur->hash == 0) {
            // root
            cur->hash = hash; cur->value = value;
            return;
        }
        if (hash < cur->hash) {
            if (cur->left == NULL) {
                place_to_insert = &cur->left; break;
            }
            cur = cur->left;
        }
        else if (hash > cur->hash) {
            if (cur->right == NULL) {
                place_to_insert = &cur->right; break;
            }
            cur = cur->right;
        }
    }
    *place_to_insert = ALLOCATE(sizeof(map)); map* new = *place_to_insert;
    new->hash = hash; new->left = new->right = NULL; new->value = value; new->parent = cur;
    new->is_red = true;

    rebalance_tree(new);
}
//#endregion

bool jcanvas_init(jcanvas* result) 
{
    result->edge_cap = 0; result->edge_count = 0; result->node_count = 0; result->node_cap = 0;
    result->nodes = NULL; result->edges = NULL; result->id_to_edges = (map){0}; result->id_to_nodes = (map){0};
    bool ok;
    ok = ensure_capacity(&result->edge_cap, 10, &result->edges, sizeof(jcanvas_edge));
    if (!ok) { return false; }
    ok = ensure_capacity(&result->node_cap, 10, &result->nodes, sizeof(jcanvas_node));
    return ok;
}

jcanvas_node* make_node(jcanvas* c, str id)
{
    if (map_get(&c->id_to_nodes, id) != NULL) {
        c->last_error = "Node with that id already exists";
        return NULL;
    }

    bool ok = ensure_capacity(&c->node_cap, c->node_count+1, &c->nodes, sizeof(jcanvas_node));
    if (!ok) return NULL;
    jcanvas_node* result = &c->nodes[c->node_count++];
    result->id = id;
    map_set(&c->id_to_nodes, id, result);
    return result;
}

jcanvas_node* jcanvas_text_node_s(jcanvas* c, str id, str content)
{
    jcanvas_node* result = make_node(c, id);
    result->type = NODE_TYPE_TEXT;
    result->as.text = content;
    return result;
}

[[always_inline]] jcanvas_node* jcanvas_text_node(jcanvas* c, char* _id, char* _content) 
{
    str id = make_str(_id);
    str content = make_str(_content);
    return jcanvas_text_node_s(c, id, content);
}

void jcanvas_infer_edge_sides(jcanvas_edge* edge, jcanvas_node* a, jcanvas_node* b)
{
    jcanvas_side from, to;
    // infer sides
    if (a->x + a->width < b->x) {
        if (a->y > b->y + b->height) { // a is below b
            from = SIDE_TOP; to = SIDE_LEFT;
        } else if (a->y + a->height <= b->y) { // a is above b
            from = SIDE_BOTTOM; to = SIDE_LEFT;
        } else {
            from = SIDE_RIGHT; to = SIDE_LEFT; // a and b are rougly on the same height
        }
    }
    else if (a->x > b->x + b->width) { // if a is right of b
        if (a->y > b->y + b->height) { // a is below b
            from = SIDE_TOP; to = SIDE_RIGHT;
        } else if (a->y + a->height <= b->y) { // a is above b
            from = SIDE_BOTTOM; to = SIDE_RIGHT;
        } else {
            from = SIDE_LEFT; to = SIDE_RIGHT; // a and b are rougly on the same height
        }
    } 
    else { // a and b are roughly on the same x location
        if (a->y > b->y + b->height) { // a is below b
            from = SIDE_TOP; to = SIDE_BOTTOM;
        } else if (a->y - a->height >= b->y) { // a is above b
            from = SIDE_BOTTOM; to = SIDE_TOP;
        } else {
            // in this case the nodes overlap so we can put arbitrary stuff here
            from = SIDE_RIGHT; to = SIDE_LEFT; // a and b are rougly on the same height
        }
    }
    edge->from_side = from; edge->to_side = to;
}

jcanvas_edge* jcanvas_connect_base(jcanvas* c, str id_from, str id_to)
{
    bool ok = ensure_capacity(&c->edge_cap, c->edge_count+1, &c->edges, sizeof(jcanvas_edge));
    if (!ok) { 
        c->last_error = "Not enough memory!";
        return NULL;
    }

    str id = str_concat(id_from, id_to);
    if (map_get(&c->id_to_edges, id) != NULL) {
        c->last_error = "Failed to connect edges: Nodes are already connected!"; return NULL;
    }

    jcanvas_edge* result = &c->edges[c->edge_count++];
    result->from_node = id_from; result->to_node = id_to;
    result->id = id; result->label = (str){0};
    map_set(&c->id_to_edges, id, result);
    return result;
}

jcanvas_edge* jcanvas_connect(jcanvas* c, jcanvas_node* a, jcanvas_node* b)
{
    if (a == NULL || b == NULL) {
        c->last_error = "Can't connect NULL nodes!";
        return NULL;
    }
    jcanvas_edge* e = jcanvas_connect_base(c, a->id, b->id);
    jcanvas_infer_edge_sides(e, a, b);
    return e;
}

jcanvas_edge* jcanvas_connect_by_id(jcanvas* c, str id_from, str id_to)
{
    jcanvas_node* a = map_get(&c->id_to_nodes, id_from);
    jcanvas_node* b = map_get(&c->id_to_nodes, id_to);

    if (a == NULL) {
        c->last_error = "Can't connect nodes: node to connect from doesn't exist!"; return NULL;
    } 
    if (b == NULL) {
        c->last_error = "Can't connect nodes: node to connect to doesn't exist!"; return NULL;
    }
    jcanvas_edge* e = jcanvas_connect_base(c, id_from, id_to);
    jcanvas_infer_edge_sides(e, a, b);
    return e;
}

[[always_inline]] void jcanvas_pos_node(jcanvas_node* node, int64_t x, int64_t y, int64_t width, int64_t height)
{
    node->x = x; node->y = y; node->width = width; node->height = height;
}

void reverse_buf(char* buf, uint32_t len)
{
    char* prev = &buf[29+len];
    char* new = buf;
    for (int i = 0; i < len; i++) {
        *new++ = *prev--;
    }
}

int int_to_str(char* buf, int64_t i)
{
    if (i == 0) { 
        buf[0] = '0'; buf[1] = 0;
        return 1; 
    }
    uint32_t index = 30;
    bool is_negative = false;
    if (i < 0) {
        is_negative = true; 
        i *= -1;
    }
    while (i > 0) {
        char digit = i % 10;
        digit += '0';
        buf[index++] = digit;
        i /= 10;
    }
    if (is_negative) {
        buf[index++] = '-';
    }
    uint32_t len = index-30;
    reverse_buf(buf, len);
    buf[len] = 0;
    return len;
}

char buf[50];
void jcanvas_generate_node(str* result, jcanvas_node* node)
{
    str_append(result, "{\"id\":\"", 7); str_append_s(result, node->id); 
    str_append(result, "\",\"type\":\"", 10); str_append_s(result, _type_strings[node->type]);
    switch (node->type) {
        case NODE_TYPE_TEXT: {
            str_append(result, "\",\"text\":\"", 10); str_append_s(result, node->as.text);
        } break;
        default: return; // not implemented yet!
    }
    char len = int_to_str(buf, node->x);
    str_append(result, "\",\"x\":\"", 7); str_append(result, buf, len);
    len = int_to_str(buf, node->y);
    str_append(result, "\",\"y\":\"", 7); str_append(result, buf, len);
    len = int_to_str(buf, node->width);
    str_append(result, "\",\"width\":\"", 11); str_append(result, buf, len);
    len = int_to_str(buf, node->height);
    str_append(result, "\",\"height\":\"", 12); str_append(result, buf, len);
    str_append(result, "\",\"color\":\"", 11); str_append_s(result, node->color);
    str_append(result, "\"}", 2);
} 

void jcanvas_generate_edge(str* result, jcanvas_edge* edge)
{
    str_append(result, "{\"id\":\"", 7); str_append_s(result, edge->id);
    str_append(result, "\",\"fromNode\":\"", 14); str_append_s(result, edge->from_node);
    str_append(result, "\",\"fromSide\":\"", 14); str_append_s(result, _side_strings[edge->from_side]);
    str_append(result, "\",\"fromEnd\":\"", 13); str_append_s(result, _end_strings[edge->from_end]);
    str_append(result, "\",\"toNode\":\"", 12); str_append_s(result, edge->to_node);
    str_append(result, "\",\"toSide\":\"", 12); str_append_s(result, _side_strings[edge->to_side]);
    str_append(result, "\",\"toEnd\":\"", 11); str_append_s(result, _end_strings[edge->to_end]);
    str_append(result, "\",\"color\":\"", 11); str_append_s(result, edge->color);
    str_append(result, "\",\"label\":\"", 11); str_append_s(result, edge->label);
    str_append(result, "\"}", 2);
}

str jcanvas_generate(jcanvas* c)
{
    str result = str_init(100);
    str_append(&result, "{\"nodes\":[", 10);
    for (int i = 0; i < c->node_count; i++) {
        jcanvas_node* nd = &c->nodes[i];
        jcanvas_generate_node(&result, nd);
        str_append(&result, ",", 1);
    }
    result.len -= 1; // remove trailing comma
    str_append(&result, "],\"edges\":[", 11);
    for (int i = 0; i < c->edge_count; i++) {
        jcanvas_edge* edge = &c->edges[i];
        jcanvas_generate_edge(&result, edge);
    }
    str_append(&result, "]}\0", 3); // null terminator for printing
    return result;
}

// TODO: implement properly
void jcanvas_destroy(jcanvas* c)
{
    if (c->nodes) {
        FREE(c->nodes);
    }
    if (c->edges) {
        FREE(c->edges);
    }
}
#endif
