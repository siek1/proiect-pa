#include "argparser.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

char* getInFile(int argc, char** argv){
    for(int i=1; i< argc; i++){
        if((strcmp(argv[i],"-f") == 0 || strcmp(argv[i], "--file") == 0))
            if(i+1<argc)
                return argv[i+1];
        else if(strncmp(argv[i], "-f=", 3) == 0)
            return argv[i] + 3;
        else if(strncmp(argv[i], "--file=", 7) == 0) 
            return argv[i] + 7;
    }
    return NULL;
}

char* getOutFile(int argc, char** argv){
    for(int i=1; i<argc; i++){
        if(strncmp(argv[i], "--out=", 6) == 0)
            return argv[i] + 6;
    }
    return NULL;
}

void parseInFile(FILE* input_file, struct argument* args, int* argscnt){

    //ADDED THIS FOR TASK 2
    if(input_file == NULL) return; 
    
    char line[100];
    while(fgets(line, sizeof(line), input_file)){
        line[strcspn(line, "\n")] = '\0';
        char* x[20];    
        int count = 0;
        char* p = strtok(line, " "); 
        while(p && count < 20){
            x[count++] = p; 
            p = strtok(NULL, " ");   
        }
        
        if(strcmp(x[0], "a") == 0){
            if(count == 1){
                args[*argscnt].type = 'a';
                args[*argscnt].value = NULL;
                args[*argscnt].ids = NULL;
                args[*argscnt].idscnt = 0;
                (*argscnt)++;
            } else{
                for(int i=1; i<count; i++){
                    if(strcmp(x[i], "task1") == 0)
                        continue;
                    args[*argscnt].type = 'a';
                    args[*argscnt].value = strdup(x[i]);
                    args[*argscnt].ids = NULL;
                    args[*argscnt].idscnt = 0;
                    (*argscnt)++;
                }
            }        
        } else{
            char type_char = x[count - 1][0];
            args[*argscnt].type = type_char;
            args[*argscnt].value = NULL;
            args[*argscnt].idscnt = 0;
            args[*argscnt].ids = malloc(10 * sizeof(struct id_entry));
            for(int i=0; i<count-1; i++){
                if(strncmp(x[i], "--", 2) == 0){
                    args[*argscnt].ids[args[*argscnt].idscnt].type = 'l';
                    args[*argscnt].ids[args[*argscnt].idscnt].id = strdup(x[i] + 2);
                    args[*argscnt].idscnt++;
                } else if(strncmp(x[i], "-", 1) == 0){
                    args[*argscnt].ids[args[*argscnt].idscnt].type = 's';
                    args[*argscnt].ids[args[*argscnt].idscnt].id = strdup(x[i] + 1);
                    args[*argscnt].idscnt++;
                }
            }
            (*argscnt)++;
        }
    }
}

void setDefFlags(struct argument* args, int argscnt){
    for(int i=0; i<argscnt; i++){
        if(args[i].type == 'f' && args[i].value == NULL){
            int* val = malloc(sizeof(int));
            *val = 1;
            args[i].value = val;
        }
    }
}

void procArgv(int argc, char** argv, struct argument* args, int* argscnt){
    for(int i=1; i<argc; i++){
        char* cur = argv[i];
        if((strcmp(cur, "-f")==0 || strcmp(cur, "--file")==0) && (i + 1)<argc){
            i++;
            continue;
        }
        if(strncmp(cur, "-f=", 3)==0 || strncmp(cur, "--file=", 7)==0)
            continue;
        if(strncmp(cur, "--out=", 6)==0)
            continue;

        int ok = 0;
        for(int j =0; j < *argscnt; j++){
            if(ok) break;
            for(int k=0; k<args[j].idscnt; k++){
                char complet[100];
                if(args[j].ids[k].type == 'l')
                    sprintf(complet, "--%s", args[j].ids[k].id);
                else
                    sprintf(complet, "-%s", args[j].ids[k].id);

                if(strcmp(cur, complet) == 0){
                    if(args[j].type == 'f'){
                        if(!args[j].value)
                            args[j].value = malloc(sizeof(int));
                        *((int*)args[j].value) = 1;
                    } else if(args[j].type == 'o'){
                        if((i + 1) < argc && argv[i + 1][0] != '-'){
                            free(args[j].value);
                            args[j].value = strdup(argv[++i]);
                        } else{
                            free(args[j].value);
                            args[j].value = strdup("");
                        }
                    }
                    ok = 1;
                    break;
                } else if(strncmp(cur, complet, strlen(complet)) == 0 && cur[strlen(complet)] == '='){
                    char* val = cur + strlen(complet) + 1;
                    if(args[j].type == 'f'){
                        if(!args[j].value)
                            args[j].value = malloc(sizeof(int));
                        *((int*)args[j].value) = 1;
                    } else if(args[j].type == 'o'){
                        free(args[j].value);
                        args[j].value = strdup(val);
                    }
                    ok = 1;
                    break;
                }
            }
        }

        if(!ok && cur[0]!='-'){
            if(strcmp(cur, "task1") == 0)
                continue;
            int found = 0;
            for(int j=0; j<*argscnt; j++){
                if(args[j].type == 'a' && args[j].value == NULL){
                    args[j].value = strdup(cur);
                    found = 1;
                    break;
                }
            }
            if(!found){
                args[*argscnt].type = 'a';
                args[*argscnt].value = strdup(cur);
                args[*argscnt].ids = NULL;
                args[*argscnt].idscnt = 0;
                (*argscnt)++;
            }
        }
    }
}

void printArg(FILE* out, const struct argument* arg){
    if(arg->type == 'a'){
        char* val = (char*)arg->value;
        if(val == NULL)
            fprintf(out, "a \n");
        else
            fprintf(out, "a %s \n", val);
    } else if(arg->type == 'f'){
        if(arg->idscnt > 0){
            if(arg->ids[0].type == 'l')
                fprintf(out, "--%s", arg->ids[0].id);
            else
                fprintf(out, "-%s", arg->ids[0].id);
            for(int j=1; j <arg->idscnt; j++){
                if(arg->ids[j].type == 'l')
                    fprintf(out, " --%s", arg->ids[j].id);
                else
                    fprintf(out, " -%s", arg->ids[j].id);
            }

            int val = arg->value ? *((int*)arg->value) : 0;
            fprintf(out, " f %d \n", val);
        }
    } else if(arg->type == 'o'){
        if(arg->idscnt > 0){
            if(arg->ids[0].type == 'l')
                fprintf(out, "--%s", arg->ids[0].id);
            else
                fprintf(out, "-%s", arg->ids[0].id);
            for(int j=1; j<arg->idscnt;j++){
                if(arg->ids[j].type == 'l')
                    fprintf(out, " --%s", arg->ids[j].id);
                else
                    fprintf(out, " -%s", arg->ids[j].id);
            }

            char* val = arg->value ? (char*)arg->value : "";
            fprintf(out, " o %s \n", val);
        }
    }
}

void writeOutFile(FILE* out, struct argument* args, int argscnt){
    for(int i=0; i<argscnt; i++){
        printArg(out, &args[i]);
    }
}


// ADDED THIS FOR TASK 2
void generateArgsFromArgv(int argc, char** argv, struct argument* args, int* argscnt){
    for(int i=1; i<argc; i++){
        if(argv[i][0] == '-'){
            if(strncmp(argv[i],"--", 2) == 0){
                args[*argscnt].type = 'f';
                args[*argscnt].value= NULL;
                args[*argscnt].idscnt = 1;
                args[*argscnt].ids =malloc(sizeof(struct id_entry));
                
                //am adaugat perror pentru ca am avut o problema cu valgrind
                if(args[*argscnt].ids==NULL){
                    perror("malloc");
                    exit(1);
                }
                args[*argscnt].ids[0].type = 'l';
                args[*argscnt].ids[0].id = strdup(argv[i]+2);
                if(args[*argscnt].ids[0].id == NULL){
                    perror("strdup");
                    exit(1);
                }
                (*argscnt)++;
            } else {
                // verificam si daca sunt mai multe flag uri combinate
                int flagcnt = strlen(argv[i]+1);   // lungimea daca avem cv de genul -rh ( fara - )
                args[*argscnt].type = 'f';
                args[*argscnt].value = NULL;
                args[*argscnt].idscnt = flagcnt;
                args[*argscnt].ids = malloc(flagcnt * sizeof(struct id_entry));
                if(args[*argscnt].ids== NULL){
                    perror("malloc");
                    exit(1);
                }
                for(int j=0; j<flagcnt; j++){
                    args[*argscnt].ids[j].type = 's';
                    char temp[2] = { argv[i][j + 1], '\0' };
                    args[*argscnt].ids[j].id = strdup(temp);
                    if(args[*argscnt].ids[j].id == NULL){
                        perror("strdup");
                        exit(EXIT_FAILURE);
                    }
                }
                (*argscnt)++;
            }
        }
    }
}
