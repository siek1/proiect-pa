#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "argparser.h"
#include "util_ls.h"

int main(int argc, char** argv) {
    int argscnt=0;
    int showdirs=0;
    int hreadable=0;
    int recursive=0;


    if(strcmp(argv[1], "task1") == 0){
        char* input_fname = getInFile(argc, argv);
        if(input_fname == NULL) {
            fprintf(stderr, "input file not foudn\n");
            return 1;
        }

        char* output_fname=getOutFile(argc, argv);
        if(output_fname==NULL)
            output_fname = "output.out";

        FILE* in= fopen(input_fname, "r");
        if(in == NULL) {
            fprintf(stderr, "cannot open %s \n", input_fname);
            return 1;
        }

        struct argument* args = malloc(100 * sizeof(struct argument));
        parseInFile(in, args, &argscnt);
        fclose(in);

        setDefFlags(args, argscnt);
        procArgv(argc, argv, args, &argscnt);

        FILE* out = fopen(output_fname, "w");
        if(out == NULL){
            fprintf(stderr, "cannot open %s \n", output_fname);
            return 1;
        }
        writeOutFile(out, args, argscnt);
        fclose(out);

        for(int i=0; i<argscnt; i++){
            if(args[i].ids){
                for(int j= 0; j<args[i].idscnt; j++)
                    free(args[i].ids[j].id);
                free(args[i].ids);
            }
            if(args[i].value)
                free(args[i].value);
            
        }

        return 0;

    } else if(strcmp(argv[1], "task2") == 0) {
        struct argument* args=malloc(100 * sizeof(struct argument));

        //Diferit de functia de la task1
        generateArgsFromArgv(argc, argv, args, &argscnt);


        setDefFlags(args, argscnt);
        procArgv(argc, argv, args, &argscnt);
        char* output_fname=getOutFile(argc, argv);
        if(output_fname==NULL)
            output_fname = "output.out";
        
        for(int i=0; i<argscnt; i++){
            if(args[i].type == 'f'){
                // for(int j=0;j<args[i].ids; )
                for(int j=0; j<args[i].idscnt; j++) {
                    if(strcmp(args[i].ids[j].id, "d")==0 || strcmp(args[i].ids[j].id, "directory")==0)
                        showdirs = 1;
                    if(strcmp(args[i].ids[j].id, "h")==0 || strcmp(args[i].ids[j].id, "human-readable")==0)
                        hreadable = 1;
                    if(strcmp(args[i].ids[j].id, "r")==0 || strcmp(args[i].ids[j].id, "recursive")==0)
                        recursive = 1;
                }
            }
        }
        
        char* dir_param = argv[2];
        if(dir_param == NULL) dir_param = ".";
        
        FILE* out = fopen(output_fname, "w");
        if(out == NULL){
            fprintf(stderr, "cannot open %s \n", output_fname);
            return 1;
        }

        //DEBUG
        printf("hreadable = %d\n", hreadable);
        
        util_ls(dir_param, showdirs, hreadable, recursive, out);
        fclose(out);
        for(int i=0; i<argscnt; i++){
            if(args[i].ids){
                for(int j= 0; j<args[i].idscnt; j++)
                    free(args[i].ids[j].id);
                free(args[i].ids);
            }
            if(args[i].value)
                free(args[i].value);
            
        }
        return 0;
    }
    // printf("%s", "invalid task name (argv[1])");
    return 1;
}
