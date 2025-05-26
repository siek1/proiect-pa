#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include "util_diff.h"

#define MAX_LEN 1024
#define COST_DELETE 1
#define COST_INSERT 1
#define COST_REPLACE 2

typedef struct{
    int cost;
    int i, j;
} PQNode;

typedef struct{
    PQNode *nodes;
    int size;
    int capacity;
} PriorityQueue;

PriorityQueue* createPriorityQueue(int capacity){
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if(pq == NULL) return NULL;
    pq->nodes = (PQNode*)malloc(capacity*sizeof(PQNode));
    pq->size=0;
    pq->capacity=capacity;
    return pq;
}

void swap(PQNode *a, PQNode *b){
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(PriorityQueue *pq, int index){
    while(index > 0 && pq->nodes[(index-1)/2].cost > pq->nodes[index].cost){
        swap(&pq->nodes[(index-1)/2], &pq->nodes[index]);
        index=(index-1)/2;
    }
}

void heapifyDown(PriorityQueue *pq, int index){
    int smallest = index;
    int left = 2*index+1;
    int right = 2*index+2;

    if(left<pq->size && pq->nodes[left].cost<pq->nodes[smallest].cost)
        smallest=left;
    if(right<pq->size && pq->nodes[right].cost<pq->nodes[smallest].cost)
        smallest=right;

    if(smallest != index){
        swap(&pq->nodes[index], &pq->nodes[smallest]);
        heapifyDown(pq, smallest);
    }
}

int isEmptyPQ(PriorityQueue* pq){
    return pq->size == 0;
}


void enQueue(PriorityQueue *pq, PQNode node){
    if(pq->size == pq->capacity) return;
    pq->nodes[pq->size]=node;
    pq->size++;
    heapifyUp(pq, pq->size-1);
}


PQNode deQueue(PriorityQueue *pq){
    if(isEmptyPQ(pq))
        return (PQNode){INT_MAX, -1, -1};
    
    PQNode min = pq->nodes[0];
    pq->size--;
    if(pq->size>0){
        pq->nodes[0] = pq->nodes[pq->size];
        heapifyDown(pq, 0);
    }
    return min;
}


void deletePriorityQueue(PriorityQueue* pq){
    if(pq != NULL){
        free(pq->nodes);
        free(pq);
    }
}

void util_diff(const char *file1, const char *file2, const char *outfile){
    FILE *f1 = fopen(file1, "r");
    FILE *f2 = fopen(file2, "r");
    FILE *out = fopen(outfile, "w");

    if(!f1 || !f2 || !out){
        fprintf(stderr, "eroare la fisiere\n");
        if(f1) fclose(f1);
        if(f2) fclose(f2);
        if(out) fclose(out);
        return;
    }

    char line1[MAX_LEN], line2[MAX_LEN];
    while(1){
        char *read1 = fgets(line1, MAX_LEN, f1);
        char *read2 = fgets(line2, MAX_LEN, f2);

        if(!read1 && !read2) break;

        if(!read1) line1[0] = '\0';
        if(!read2) line2[0] = '\0';

        line1[strcspn(line1, "\n")] = '\0';
        line2[strcspn(line2, "\n")] = '\0';

        int n = strlen(line1);  // ref
        int m = strlen(line2);  // compare

        // matrix
        int **cost = malloc((m+1) * sizeof(int*));
        bool **visited = malloc((m+1) * sizeof(bool*));
        for(int i=0; i<=m; i++){
            cost[i] = malloc((n+1)*sizeof(int));
            visited[i] = malloc((n+1) * sizeof(bool));
            for(int j=0; j<=n; j++){
                cost[i][j] = INT_MAX;
                visited[i][j] = false;
            }
        }
        cost[0][0] = 0;

        PriorityQueue *pq = createPriorityQueue((m+1)*(n+1));
        enQueue(pq, (PQNode){0, 0, 0});

        // Dijkstra
        while(!isEmptyPQ(pq)){
            PQNode current = deQueue(pq);
            int i=current.i;
            int j=current.j;

            if(current.cost == INT_MAX || visited[i][j]) continue;
            visited[i][j] = true;

            if(i==m && j==n) break;

            
            // REPLACE/MATCH - diagonala
            if(i<m && j<n && !visited[i+1][j+1]){
                int new_cost;
                if(line2[i] == line1[j]){
                    new_cost = cost[i][j]; // MATCH (cost 0)
                } else {
                    new_cost = cost[i][j] + COST_REPLACE;
                }
                if(new_cost < cost[i+1][j+1]){
                    cost[i+1][j+1] = new_cost;
                    enQueue(pq, (PQNode){new_cost, i+1, j+1});
                }
            }

            // DELETE - dreapta
            if(j<n && !visited[i][j+1]){
                int new_cost = cost[i][j] + COST_DELETE;
                if(new_cost < cost[i][j+1]){
                    cost[i][j + 1] = new_cost;
                    enQueue(pq, (PQNode){new_cost, i, j+1});
                }
            }

            // INSERT - jos
            if(i<m && !visited[i+1][j]){
                int new_cost = cost[i][j] + COST_INSERT;
                if(new_cost<cost[i+1][j]){
                    cost[i+1][j] = new_cost;
                    enQueue(pq, (PQNode){new_cost, i+1, j});
                }
            }
        }

        // path reconstruction
        int i=m, j=n;
        int del=0, ins=0, rep=0;

        while(i>0 || j>0){
            // MATCH
            if(i>0 && j>0 && line2[i-1] == line1[j-1]){
                i--; j--;
            }
            // REPLACE
            else if(i>0 && j>0 && cost[i][j] == cost[i-1][j-1] + COST_REPLACE){
                i--; j--;
                rep++;
            }
            // DELETE
            else if(j>0 && cost[i][j] == cost[i][j-1] + COST_DELETE){
                j--;
                del++;
            }
            // INSERT
            else if(i>0 && cost[i][j] == cost[i-1][j] + COST_INSERT){
                i--;
                ins++;
            }
        }

        if(del==0 && ins==0 && rep==0 && line1[0]=='\0' && line2[0]=='\0')
            continue;

        fprintf(out, "%dD %dI %dR \n", del, ins, rep);

        // free
        for(int i=0; i<=m; i++){
            free(cost[i]);
            free(visited[i]);
        }
        free(cost);
        free(visited);
        deletePriorityQueue(pq);
    }
    fclose(f1);
    fclose(f2);
    fclose(out);
}