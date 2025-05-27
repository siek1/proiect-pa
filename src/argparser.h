#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include<stdio.h>

struct id_entry {
    char type;  // 'l' - long, 's' - short
    char* id; // șir de caractere sau un caracter identificator
};

struct argument {
    char type; // 'f' - flag, 'a' - argument, 'o' - opțiune
    void* value; // pointer la valoare
    struct id_entry* ids;
    int idscnt;
};

char* getInFile(int argc, char** argv);
char* getOutFile(int argc, char** argv);
void parseInFile(FILE* input_file, struct argument* args, int* argscnt);
void setDefFlags(struct argument* args, int argscnt);
void procArgv(int argc, char** argv, struct argument* args, int* argscnt);
void writeOutFile(FILE* out, struct argument* args, int argscnt);
// void printArg(FILE* out, const struct argument* arg);
void generateArgsFromArgv(int argc, char** argv, struct argument* args, int* argscnt);

#endif // ARG_PARSER_H
