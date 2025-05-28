#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include "util_cd.h"
static void free_tree(struct dirnode* node); // for style issues 

// dam split la path in relatie cu / urile 
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

// "/home/user/docs" si "/home/user/pics" -> gasim un prefix comun (home, user)
static int find_root(char** srcparts, int srclen, char** dstparts, int dstlen){
    int i=0;
    //comparam fiecare parte din sursa si destinatie
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

// arbore de directoare construit folosind stiva
static struct dirnode* build_tree_with_stack(const char* path){
    DIR* dir = opendir(path);

    if(!dir){
        perror(path); //DEBUG
        return NULL;
    }

    // cream nodul pentru directorul curent
    struct dirnode* node = malloc(sizeof(struct dirnode));
    if(node == NULL){
        perror("malloc failed");
        closedir(dir);
        // deleteStack(&stackTop);
        return NULL;
    }

    // extragem doar numele directorului (partea dupa ultimul '/')
    const char* last_slash = strrchr(path, '/');
    strcpy(node->name, last_slash ? last_slash + 1 : path);
    node->childcnt = 0;
    node->children = NULL;

    Node* stackTop = NULL;

    // parcurgem toate entry urile din director
    const struct dirent* entry;
    while((entry = readdir(dir)) != NULL){
        // skip "." si ".."
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // calea completa a entry-ului
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        //verificam daca este un director
        struct stat st;
        if(stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)){
            // daca este director, il adaugam in stiva
            push(&stackTop, full_path);
        }
    }


    // parcurgem toate subdirectoarele din stiva
    while(!isEmpty(stackTop)){
        char* subpath = pop(&stackTop);

        //  realloc la vectorul de copii pentru a face loc noului subdirector
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


// construieste calea pe baza parcursului arborelui de directoare (un fel de DFS?)
static int find_path_full(struct dirnode* node, char** target_parts, int target_len, int depth, Node** pathStack){
    if(!node) return 0;   
    if(depth >= target_len) return 0;
    if(strcmp(node->name, target_parts[depth]) != 0) return 0;

    // adaugam nodul curent in stiva (calea partiala)
    push(pathStack, node->name);

    // daca am ajuns la ultima parte a destinatiei, am gasit calea
    if(depth == target_len - 1) return 1;

    // cautam in copii
    for(int i=0; i<node->childcnt; i++) 
        if(find_path_full(node->children[i], target_parts, target_len, depth + 1, pathStack))
            return 1; // daca am gasit calea, iesim

    // backtrack: daca nu am gasit calea, scoatem nodul curent din stiva
    free(pop(pathStack));
    return 0;
}




void util_cd(const char *srcpath, const char *dstpath, const char *output_file){
    char **srcparts = NULL, **dstparts = NULL;
    struct dirnode *root = NULL;
    Node *down = NULL, *rev = NULL;
    FILE *out = NULL;
    int ok=1;

    // impartim caile src si dst in parts
    int srclen=0, dstlen=0;
    srcparts = split(srcpath, &srclen);
    dstparts = split(dstpath, &dstlen);
    if(!srcparts || !dstparts) ok=0;

    // gasim prefixul comun intre cele doua
    int cmn = ok ? find_root(srcparts, srclen, dstparts, dstlen) : 0;
    
    char cmnpath[MAX_PATH_LEN] = "";
    // daca calea originala incepe cu /, prefixul comun trebuie sa inceapa la fel
    if(srcpath[0] == '/') strcat(cmnpath, "/");

    // construim calea prefixului comun
    for(int i=0; i<cmn; i++){
        if(i>0) strcat(cmnpath, "/");
        strcat(cmnpath, srcparts[i]);
    }
    // daca nu avem prefix comun, folosim calea curenta (.)
    if(cmn == 0 && srcpath[0] != '/') strcpy(cmnpath, ".");

    // construim un arbore de directoare incepand de la prefixul comun
    if(ok){
        root = build_tree_with_stack(cmnpath);
        if(!root) ok=0;
    }

    // gasim calea de la prefixul comun la destinatie in arbore
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

    //construim calea relativa de la src la dst
    if(ok){

        for(int i=cmn; i<srclen; i++) fprintf(out, "../");
        
        //inversam stiva pentru a avea ordinea corecta
        while(!isEmpty(down)){
            char *v = pop(&down);
            push(&rev, v);
            free(v);
        }

        // eliminam primul element (daca exista)
        if(!isEmpty(rev)) free(pop(&rev));
        
        // scriem calea la destinatie
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
