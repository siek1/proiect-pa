#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<limits.h> 
#include<unistd.h> 

#define MAX_FILES 1024
#define MAX_INCLUDES 128

typedef struct{
    char *file;
    char *includes[MAX_INCLUDES];
    int includecnt;
} Node;

Node files[MAX_FILES];
int filecnt=0;

int showwrngs=0;

// vedem daca un fisier e sursa sau header
int is_src(const char *fname){
    return strstr(fname, ".c") || strstr(fname, ".h");
}

// vedem daca linia e include si extragem numele fisierului
int is_include(const char *line, char *out){
    if(sscanf(line, "#include \"%[^\"]\"", out) == 1) return 1;
    if(sscanf(line, "#include <%[^>]>", out) == 1) return 2;
    return 0;
}

// citim un fisier si extragem toate includes 
void read_file(const char *fpath){
    FILE *f = fopen(fpath, "r");
    if(!f) return;

    // calea absoluta a fisierului
    char path[PATH_MAX];
    if(!realpath(fpath, path)){
        fclose(f);
        return;
    }

    //salvam calea absoluta in structura
    files[filecnt].file = strdup(path);
    files[filecnt].includecnt = 0;

    char line[512], inc_raw[PATH_MAX], inc_full[PATH_MAX];

    // getdir of curr file
    char dir[PATH_MAX];
    strcpy(dir, path);
    char *last_slash = strrchr(dir, '/');
    if(last_slash) *last_slash = '\0';

    // citim fisierul linie cu linie
    while(fgets(line, sizeof(line), f)){
        // type 0 = nu e include
        // type 1 = include local
        // type 2 = include sistem

        int type = is_include(line, inc_raw);

        if(type == 1){
            size_t dir_len = strlen(dir);
            size_t inc_raw_len = strlen(inc_raw);
            if(dir_len + 1 + inc_raw_len < PATH_MAX){ //+1 for /
                strcpy(inc_full, dir);
                strcat(inc_full, "/");
                strcat(inc_full, inc_raw);
            } else {
                fprintf(stderr, "Warning: path > PATH_MAX.\n");
                continue;
            }

            char resolved_include[PATH_MAX];
            if(realpath(inc_full, resolved_include)){
                files[filecnt].includes[files[filecnt].includecnt++] = strdup(resolved_include);
            }
        } else if(type == 2){
            files[filecnt].includes[files[filecnt].includecnt++] = strdup(inc_raw);  // system include
        }
    }

    fclose(f);
    filecnt++;
}


// parcurgem un director si citim toate fisierele sursa 
void traverse_dir(const char *dirpath){
    DIR *d = opendir(dirpath);
    if(!d) return;

    struct dirent *entry;
    while((entry = readdir(d))){
        // skip la . si ..
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[PATH_MAX];
        
        size_t dirpathlen = strlen(dirpath);
        size_t dnamelen = strlen(entry->d_name);
        
        // verificam sa nu depasim PATH_MAX
        if(dirpathlen + 1 + dnamelen < PATH_MAX){ //+1 for /
            strcpy(full_path, dirpath);
            strcat(full_path, "/");
            strcat(full_path, entry->d_name);
        } else {
            fprintf(stderr, "Warning: path > PATH_MAX\n");
            continue;
        }

        // informatii despre fisier sau director
        struct stat st;
        if(stat(full_path, &st) == -1) continue;

        // daca e director il parcurgem recursiv
        if(S_ISDIR(st.st_mode))
            traverse_dir(full_path);
        // daca e sursa il citim
        else if(is_src(entry->d_name))
            read_file(full_path);
    }

    closedir(d);
}

int find_file_index(const char *path){
    char real[PATH_MAX];
    if(!realpath(path, real)) return -1;

    for(int i=0; i<filecnt; i++)
        if(strcmp(real, files[i].file) == 0) return i;

    return -1;
}

int visited[MAX_FILES], in_stack[MAX_FILES];

int dfs(int idx){
    visited[idx] = 1;
    in_stack[idx] = 1;

    for(int i=0; i<files[idx].includecnt; i++){
        const char *included = files[idx].includes[i];

        if(included[0] != '/') continue;

        int j = find_file_index(included);
        if(j != -1){
            if(!visited[j] && dfs(j)) return 1;
            else if(in_stack[j]) return 1;
        }
    }

    in_stack[idx] = 0;
    return 0;
}

void detect_problems(){
    
    memset(visited, 0, sizeof(visited));
    memset(in_stack, 0, sizeof(in_stack));

    for(int i=0; i<filecnt; i++){
        if(!visited[i]){
            if(dfs(i)){
                printf("ERROR loop detected\n");
                break; 
            }
        }
    }

    // duplicate includes
    if(showwrngs)
        for(int i=0; i<filecnt; i++){
            for(int j=i+1; j<filecnt; j++){
                for(int a=0; a<files[i].includecnt; a++){
                    for(int b=0; b<files[j].includecnt; b++){
                        if(strcmp(files[i].includes[a], files[j].includes[b]) == 0)
                            printf("WARNING double include: %s in %s %s\n", files[i].includes[a], files[i].file, files[j].file);
                    }
                }
            }
        }
}

int main(int argc, char **argv){
    if(argc < 2){
        fprintf(stderr, "%s directory --full\n", argv[0]);
        return 1;
    }

    if(argc == 3 && strcmp(argv[2], "--full") == 0)
        showwrngs = 1;


    char root[PATH_MAX];
    if(!realpath(argv[1], root)){
        fprintf(stderr, "Invalid path.\n");
        return 1;
    }

    traverse_dir(root);
    detect_problems();

    return 0;
}