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
            str path;
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
jcanvas_node* jcanvas_file_node_s(jcanvas* c, str id, str content);
jcanvas_node* jcanvas_file_node(jcanvas* c, char* id, char* content);
jcanvas_node* jcanvas_link_node_s(jcanvas* c, str id, str link);
jcanvas_node* jcanvas_link_node(jcanvas* c, char* id, char* link);
jcanvas_node* jcanvas_group_node_s(jcanvas* c, str id);
jcanvas_node* jcanvas_group_node(jcanvas* c, char* id);
void jcanvas_set_label_s(jcanvas_node* node, str label);
void jcanvas_set_label(jcanvas_node* node, char* label);
void jcanvas_set_background_image_s(jcanvas_node* node, str path);
void jcanvas_set_background_image(jcanvas_node* node, char* path);
void jcanvas_set_background_style(jcanvas_node* node, jcanvas_background_style style);
jcanvas_edge* jcanvas_connect(jcanvas* c, jcanvas_node* a, jcanvas_node* b);
void jcanvas_pos_node(jcanvas_node* node, int64_t x, int64_t y, int64_t width, int64_t height);
str jcanvas_generate(jcanvas* c);
void jcanvas_destroy(jcanvas* c);