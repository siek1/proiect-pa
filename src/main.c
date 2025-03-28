#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct id_entry{
    char type;  // 'l' - long, 's' - short
    char* id; // șir de caractere sau un caracter identificator
};

struct argument{
    char type; // 'f' - flag, 'a' - argument, 'o' - opțiune
    void* value; // pointer la valoare
     // pentru 'a' și 'o' este pointer un șir de caractere
    // pentru 'f' este pointer la o valoare numerică, 0 sau 1
    struct id_entry* ids;
    int ids_count;
};

char* getInFile (int argc, char** argv){
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

char* getOutFile(int argc, char** argv ){
    for(int i=1; i<argc; i++) {
        if(strncmp(argv[i], "--out=", 6) == 0)
            return argv[i] + 6;
    }
    return NULL;
}

void parseInFile( FILE* input_file, struct argument* args, int* args_count){
    char line[100];
    while(fgets(line, sizeof(line), input_file)){
        // dam remove la new lines
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
                args[*args_count].type = 'a';
                args[*args_count].value = NULL;
                args[*args_count].ids = NULL;
                args[*args_count].ids_count = 0;
                (*args_count)++;
            } else {
                for(int i=1; i<count; i++){
                    if (strcmp(x[i], "task1") == 0) // sari peste task1
                        continue;
                    args[*args_count].type = 'a';
                    args[*args_count].value = strdup(x[i]);
                    args[*args_count].ids = NULL;
                    args[*args_count].ids_count = 0;
                    (*args_count)++;
                }
            }        
        } else {
            char type_char = x[count - 1][0];
        
            args[*args_count].type = type_char;
            args[*args_count].value = NULL;
            args[*args_count].ids_count = 0;
        
            //maxim 10 ids
            args[*args_count].ids = malloc(10 * sizeof(struct id_entry));
        
            for(int i=0; i<count-1; i++){
                if(strncmp(x[i], "--", 2) == 0){
                    args[*args_count].ids[args[*args_count].ids_count].type = 'l'; // forma lunga
                    args[*args_count].ids[args[*args_count].ids_count].id = strdup(x[i] + 2); // copiem de la 2 incolo
                    args[*args_count].ids_count++;
                } else if(strncmp(x[i], "-", 1) == 0){
                    args[*args_count].ids[args[*args_count].ids_count].type = 's'; //forma scurta
                    args[*args_count].ids[args[*args_count].ids_count].id = strdup(x[i] + 1); // copiem de la 1 incolo
                    args[*args_count].ids_count++;
                }
            }
        
            (*args_count)++;
        }
    }
}

void setDefFlags(struct argument* args, int args_count){
    for(int i=0; i<args_count; i++){
        if(args[i].type == 'f' && args[i].value == NULL){
            int* val = malloc(sizeof(int));
            *val = 1;
            args[i].value = val;
        }
    }
}

void procArgv(int argc, char** argv, struct argument* args, int* args_count){
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
        for (int j =0; j < *args_count; j++) {
            if (ok) break;

            for(int k=0; k<args[j].ids_count; k++){
                char complet[100];

                if(args[j].ids[k].type == 'l')
                    sprintf(complet, "--%s", args[j].ids[k].id);
                else
                    sprintf(complet, "-%s", args[j].ids[k].id);

                if(strcmp(cur, complet) == 0) {
                    if(args[j].type == 'f') {
                        if(!args[j].value)
                            args[j].value = malloc(sizeof(int));
                        *((int*)args[j].value) = 1;
                    } else if(args[j].type == 'o') {
                        if((i + 1) < argc && argv[i + 1][0] != '-'){
                            free(args[j].value);
                            args[j].value = strdup(argv[++i]);
                        } else {
                            free(args[j].value);
                            args[j].value = strdup("");
                        }
                    }
                    ok = 1;
                    break;
                } else if(strncmp(cur, complet, strlen(complet)) == 0 && cur[strlen(complet)] == '='){
                    char* val = cur + strlen(complet) + 1;
                    if(args[j].type == 'f') {
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

        // daca nu am gasit nimic atunci e argument
        if(!ok&&cur[0]!='-'){
            if (strcmp(cur, "task1") == 0) // sari peste task1
                continue;

            int found = 0;
            for(int j=0; j<*args_count; j++){
                if(args[j].type == 'a' && args[j].value == NULL){
                    args[j].value = strdup(cur);
                    found = 1;
                    break;
                }
            }

            if(!found){
                args[*args_count].type = 'a';
                args[*args_count].value = strdup(cur);
                args[*args_count].ids = NULL;
                args[*args_count].ids_count = 0;
                (*args_count)++;
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
        if(arg->ids_count > 0) {
            if(arg->ids[0].type == 'l')
                fprintf(out, "--%s", arg->ids[0].id);
            else
                fprintf(out, "-%s", arg->ids[0].id);
            for (int j=1; j <arg->ids_count; j++){
                if(arg->ids[j].type == 'l')
                    fprintf(out, " --%s", arg->ids[j].id);
                else
                    fprintf(out, " -%s", arg->ids[j].id);
            }

            int val;
            if(arg->value != NULL)
                val = *((int*)arg->value);
            else
                val = 0;

            fprintf(out, " f %d \n", val);
        }
    } else if(arg->type == 'o'){
        if(arg->ids_count > 0){
            if(arg->ids[0].type == 'l')
                fprintf(out, "--%s", arg->ids[0].id);
            else
                fprintf(out, "-%s", arg->ids[0].id);
            for(int j=1; j<arg->ids_count;j++){
                if(arg->ids[j].type == 'l')
                    fprintf(out, " --%s", arg->ids[j].id);
                else
                    fprintf(out, " -%s", arg->ids[j].id);
            }

            char* val;
            if(arg->value != NULL)
                val = (char*)arg->value;
            else
                val = "";

            fprintf(out, " o %s \n", val);
        }
    }
}


void writeOutFile(FILE* out, struct argument* args, int args_count){
    for(int i=0; i<args_count; i++){
        printArg(out, &args[i]);
    }
}

int main(int argc, char** argv){
    int args_count = 0;

    char* input_fname = getInFile(argc, argv);
    if(!input_fname){
        fprintf(stderr, "input file not foudn\n");
        return 1;
    }
    char* output_fname = getOutFile(argc, argv);
    if(output_fname == NULL)
        output_fname = "output.out";

    FILE* in = fopen(input_fname, "r");
    if(in == NULL){
        fprintf(stderr, "cannot open %s \n", input_fname);
        return 1;
    }

    struct argument* args = malloc(100 * sizeof(struct argument));
    parseInFile(in, args, &args_count);
    fclose(in);

    setDefFlags(args, args_count);
    procArgv(argc, argv, args, &args_count);

    FILE* out = fopen(output_fname, "w");
    if(out == NULL){
        fprintf(stderr, "cannot open %s \n", output_fname);
        return 1;
    }
    writeOutFile(out, args, args_count);
    fclose(out);


    //free stuff
    for(int i=0; i<args_count; i++) {
        if(args[i].ids){
            for(int j= 0; j<args[i].ids_count; j++){
                free(args[i].ids[j].id);
            }
            free(args[i].ids);
        }
        if(args[i].value){
            free(args[i].value);
        }
    }
    // free(args);
    return 0;
}
