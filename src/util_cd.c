#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include "util_cd.h"
static void free_tree(struct dirnode* node); // for style issues 


static char** split(const char* path, int* count){
    char* pathcpy = strdup(path);
    char** parts = malloc(100 * sizeof(char*));
    if (parts == NULL) {
        perror("malloc failed");
        exit(1);
    }
    const char* p = strtok(pathcpy, "/");
    int i=0;
    while(p){
        parts[i++] = strdup(p);
        p = strtok(NULL, "/");
    }
    *count=i;
    free(pathcpy);
    return parts;
}

static int find_root(char** srcparts, int srclen, char** dstparts, int dstlen){
    int i=0;
    while(i<srclen && i<dstlen && strcmp(srcparts[i], dstparts[i]) == 0)
        i++;
    return i; 
}

static int isEmpty(const Node* top){
    return top==NULL;
}

static void push(Node** top, const char* val){
    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("malloc failed");
        return;
    }
    newNode->val = strdup(val);
    newNode->next = *top;
    *top = newNode;
}

static char* pop(Node** top){
    if(isEmpty(*top)) return NULL;
    Node* temp = *top;
    char* val = strdup(temp->val);
    *top = (*top)->next;
    free(temp->val);
    free(temp);
    return val;
}

static void deleteStack(Node** top){
    while((*top) != NULL){  // echivalent cu !isEmpty(*top)
        Node* temp = *top;
        *top = (*top)->next;
        free(temp);
    }
}



static struct dirnode* build_tree_with_stack(const char* path){
    DIR* dir = opendir(path);

    if(!dir){
        perror(path); //DEBUG
        return NULL;
    }

    struct dirnode* node = malloc(sizeof(struct dirnode));
    if(node == NULL){
        perror("malloc failed");
        closedir(dir);
        // deleteStack(&stackTop);
        return NULL;
    }
    const char* last_slash = strrchr(path, '/');
    strcpy(node->name, last_slash ? last_slash + 1 : path);
    node->childcnt = 0;
    node->children = NULL;

    Node* stackTop = NULL;

    const struct dirent* entry;
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if(stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)){
            push(&stackTop, full_path);
        }
    }

    while(!isEmpty(stackTop)){
        char* subpath = pop(&stackTop);
        void* tmp = realloc(node->children, (node->childcnt + 1) * sizeof(struct dirnode*));
        if(tmp == NULL){
            perror("realloc failed");
            free_tree(node);
            deleteStack(&stackTop);
            closedir(dir);
            return NULL;
        }
        node->children = tmp;
        node->children[node->childcnt++] = build_tree_with_stack(subpath);
        free(subpath);
    }

    deleteStack(&stackTop);
    closedir(dir);
    return node;
}

static void free_tree(struct dirnode* node){
    for(int i=0; i<node->childcnt; i++)
        free_tree(node->children[i]);
        // free(node->children[i]);
    free(node->children);
    free(node);
}

static int find_path_full(struct dirnode* node, char** target_parts, int target_len, int depth, Node** pathStack){
    if(!node) return 0;   
    if(depth >= target_len) return 0;
    if(strcmp(node->name, target_parts[depth]) != 0) return 0;

    push(pathStack, node->name);

    if(depth == target_len - 1) return 1;

    for(int i=0; i<node->childcnt; i++) 
        if(find_path_full(node->children[i], target_parts, target_len, depth + 1, pathStack))
            return 1;

    free(pop(pathStack));
    return 0;
}


void util_cd(const char *srcpath, const char *dstpath, const char *output_file){
    char **srcparts = NULL, **dstparts = NULL;
    struct dirnode *root = NULL;
    Node *down = NULL, *rev = NULL;
    FILE *out = NULL;
    int ok=1;

    int srclen=0, dstlen=0;
    srcparts = split(srcpath, &srclen);
    dstparts = split(dstpath, &dstlen);
    if(!srcparts || !dstparts) ok=0;

    int cmn = ok ? find_root(srcparts, srclen, dstparts, dstlen) : 0;

    char cmnpath[MAX_PATH_LEN] = "";
    if(srcpath[0] == '/') strcat(cmnpath, "/");
    for(int i=0; i<cmn; i++){
        if(i>0) strcat(cmnpath, "/");
        strcat(cmnpath, srcparts[i]);
    }
    if(cmn == 0 && srcpath[0] != '/') strcpy(cmnpath, ".");

    if(ok){
        root = build_tree_with_stack(cmnpath);
        if(!root) ok=0;
    }

    if(ok){
        int start = cmn > 0 ? cmn - 1 : 0;
        if(!find_path_full(root, &dstparts[start], dstlen-start, 0, &down))
            ok = 0;
    }

    if(ok){
        out = fopen(output_file, "w");
        if(!out){
            perror("fopen");
            ok = 0;
        }
    }

    if(ok){
        for(int i=cmn; i<srclen; i++) fprintf(out, "../");
        
        while(!isEmpty(down)){
            char *v = pop(&down);
            push(&rev, v);
            free(v);
        }

        if(!isEmpty(rev)) free(pop(&rev));
        
        while(!isEmpty(rev)){
            char *v = pop(&rev);
            fprintf(out, "%s/", v);
            free(v);
        }
    }

    if(out) fclose(out);
    deleteStack(&down);
    deleteStack(&rev);
    if(root) free_tree(root);
    if(srcparts){
        for(int i=0; i<srclen; i++) free(srcparts[i]);
        free(srcparts);
    }
    if(dstparts){
        for(int i=0; i<dstlen; i++) free(dstparts[i]);
        free(dstparts);
    }
    if(!ok) fprintf(stderr, "util_cd - eroare la calcul relative path\n");
    
}
