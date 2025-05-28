#include "util_ls.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/types.h>  // for off_t
#include<sys/stat.h>
#include<unistd.h>


#define MAX 1024

struct fileinfo{
    char* path;
    off_t size;
};

struct directoryinfo{
    char* path;
    int depth;
    struct fileinfo* files;
    int filecnt;
};

static void showsize(off_t size, int hreadable, FILE* out){
    int n = 1024;
    if(!hreadable){
        fprintf(out, "%ld", size);
    } else {
        if(size >= n*n*n)
            fprintf(out, "%.0fG", size/((double)n*n*n));
        else if(size>= n*n)
            fprintf(out, "%.0fM", size/((double)n*n));
        else if(size >= n)
            fprintf(out, "%.0fK", size/(double)n);
        else
            fprintf(out, "%ld", size);
    }
}


static void listdir(const char* basedir,int depth, int showdirs, int hreadable, int recursive, FILE* out){
    DIR* dir=opendir(basedir);
    if(dir==NULL) return;

    const struct dirent* entry;
    // struct fileinfo* fileinfo;
    
    // struct fileinfo files[1024];
    // int filecnt = 0;
    
    struct directoryinfo dirinfo;
    dirinfo.path = strdup(basedir);
    dirinfo.depth = depth;
    dirinfo.files = malloc(1024 * sizeof(struct fileinfo));
    dirinfo.filecnt = 0;


    char* subdirs[1024];
    int subdircnt =0;
    // int depth = 0;
    
    entry=readdir(dir);
    while(entry != NULL){
        // if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            entry = readdir(dir);
            continue;
        }
        char fullpath[MAX];
        snprintf(fullpath, MAX, "%s/%s", basedir, entry->d_name);

        struct stat st;
        if(stat(fullpath, &st) != -1){
            if(S_ISDIR(st.st_mode)){
                subdirs[subdircnt++] = strdup(fullpath);
            } else if(S_ISREG(st.st_mode)){
                dirinfo.files[dirinfo.filecnt].path = strdup(entry->d_name);
                dirinfo.files[dirinfo.filecnt++].size = st.st_size;
            }
        }
        // }
        entry = readdir(dir);  
    }
    closedir(dir);

    // printeaza . si .. la inceput
    if(depth == 0){
        showsize(4096, hreadable, out);
        fprintf(out, " .\n");

        showsize(4096, hreadable, out);
        fprintf(out, " ..\n");
    }

    struct mrg{
        char* name;       
        off_t size;
        int isdir; // 1 director 0 fisier
        char* fullpath;   
    } merged[2048];    
    int mcnt = 0;
    
   // add subdirs to merged
    for(int i=0; i<subdircnt; i++){
        struct stat st;
        stat(subdirs[i], &st); 
        merged[mcnt].isdir =1;
        merged[mcnt].size = st.st_size;
        merged[mcnt].fullpath =subdirs[i];
        // ia ce vine dupa /
        merged[mcnt].name = strrchr(subdirs[i], '/');
        
        if(merged[mcnt].name)
            merged[mcnt].name++;  // skip /
        else
            merged[mcnt].name = subdirs[i];
        
        mcnt++;
    }

    // add files to merged
    for(int i=0; i<dirinfo.filecnt; i++){
        merged[mcnt].isdir = 0;
        merged[mcnt].size = dirinfo.files[i].size;
        merged[mcnt].fullpath = NULL;
        merged[mcnt].name = dirinfo.files[i].path;
        mcnt++;
    }

    // sort merged arr by naem
    for(int i=0; i<mcnt-1; i++){
        for(int j=i+1; j<mcnt; j++){
            if(strcmp(merged[i].name, merged[j].name) > 0){
                struct mrg aux = merged[i];
                merged[i] = merged[j];
                merged[j] = aux;
            }
        }
    }

    for(int i=0; i<mcnt; i++){
        
        // daca showdir dam skip la files
        if(showdirs && merged[i].isdir == 0)
            continue;

        for(int j=0; j<depth; j++)
            fprintf(out, "    ");
        showsize(merged[i].size, hreadable, out);
        fprintf(out, " %s\n", merged[i].name);
        
        // list dir recursively just after printing it
        if(recursive && merged[i].isdir){
            listdir(merged[i].fullpath, depth + 1, showdirs, hreadable, recursive, out);
            // free(merged[i].fullpath);
        }
    }

    //free
    for(int i=0; i<mcnt; i++){
        if (merged[i].isdir && merged[i].fullpath)
            free(merged[i].fullpath);
    }
 
    for(int i=0; i<dirinfo.filecnt; i++)
        free(dirinfo.files[i].path);

    free(dirinfo.files);
    free(dirinfo.path);
    // for(int i=0; i<subdircnt; i++)
    //     if(subdirs[i]) free(subdirs[i]);
    
}

void util_ls(const char* basedir, int showdirs, int hreadable, int recursive, FILE* out){
    listdir(basedir, 0, showdirs, hreadable, recursive, out);
}