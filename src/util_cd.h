#ifndef UTIL_CD_H
#define UTIL_CD_H

#define MAX_PATH_LEN 1024


typedef struct Node{
    char* val;
    struct Node* next;
} Node;

struct dirnode{
    char name[MAX_PATH_LEN];
    int childcnt;
    struct dirnode** children;
};


void util_cd(const char* srcpath, const char* dstpath, const char* output_file);

// struct dirnode* build_tree_with_stack(const char* path);
// void free_tree(struct dirnode* node);

// int isEmpty(Node* top);
// void push(Node** top, const char* val);
// char* pop(Node** top);
// void deleteStack(Node** top);

#endif 