#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#define NAMELEN 20
FILE *infile;
char * map, * nextmap, *tmp;
int N;
int M;
int Round;
int finished;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t v = PTHREAD_COND_INITIALIZER;
typedef struct paras Paras;
struct paras{
    long long start,end;
};
char  live[4];


void Readfile(char *infilename){
    infile = fopen(infilename,"r");
    int para[3];
    char c[10];
    for(int i = 0;i < 3;i ++){
        fscanf(infile,"%s",&c);
        para[i] = atoi(c);
    }
    N = para[0];
    M = para[1];
    Round = para[2];
    return;
}
void InitMap(char *map){
    char temp;
    char buf[M+10];
    fgets(buf,M+10,infile);
    for(int i = 0;i < N;i ++){
        fgets(buf,M+10,infile);
        for(int j = 0;j < M ;j ++){
            //temp = fgetc(infile);
            /*
            while(temp == '\n'){
                temp = fgetc(infile);
            }
            */
            map[M*i+j] = buf[j];
        }
    }
    return;
}
void UpdataStatus(int liveneigbor,int i,int j){
    if(liveneigbor >= 4){
        nextmap[M*i+j] = '.';
    }
    else if(liveneigbor == 3){
        nextmap[M*i+j] = 'O';
    }
    else if(liveneigbor == 2){
        nextmap[M*i+j] = map[M*i+j];
    }
    else{
        nextmap[M*i+j] = '.';
    }
    return;
}
void *CountLives(void *p){
    Paras *par = (Paras *)p;
    
    for(int count = par->start;count < par->end;count ++){
        int i = count/M;
        int j = count%M;
        int liveneigbor = 0;
        int up,down,left,right;
        int starti = (i-1 >= 0)? i-1:0;
        int endi = (i+1 < N)? i+1: N-1;
        int startj = (j-1 >= 0)? j-1:0;
        int endj = (j+1 < M)? j+1:M-1; 
        
        for(int k = starti;k <= endi;k ++){
            for(int l = startj;l <= endj;l ++){

                if(k != i || l != j){
                    if(map[k*M+l] == 'O'){
                        liveneigbor ++;
                    }
                }

            }
        }
        UpdataStatus(liveneigbor,i,j);
    }
    pthread_exit(NULL);
}
void processCountLive(int start,int end){
    for(int count = start;count < end;count ++){
        int i = count/M;
        int j = count%M;
        int liveneigbor = 0;
        int up,down,left,right;
        up = (i-1 >= 0)? i-1:i;
        down = (i+1 < N)? i+1:i;
        left = (j-1 >= 0)? j-1:j;
        right = (j+1 < M)? j+1:j;
        for(int k = up;k <= down;k ++){
            for(int l = left;l <= right;l ++){
                if(k != i || l != j){
                    if(map[k*M+l] == 'O'){
                        liveneigbor ++;
                    }
                }
            }
        }
        UpdataStatus(liveneigbor,i,j);
    }
    return;
}
void PrintCurrMap(char *map,char *outfilename){
    FILE *outfile;
    outfile = fopen(outfilename,"w");
    for(int i = 0;i < N;i ++){
        for(int j = 0;j < M ;j ++){
            fprintf(outfile,"%c",map[M*i+j]);
        }
        if(i != N-1){
            fprintf(outfile,"\n");
        }
        
    }
}
int main(int argc, char *argv[]){
    //Parse auguments
    char *threadOrprocess = argv[1];
    int num = atoi(argv[2]);
    char infilename[NAMELEN];
    char outfilename[NAMELEN];
    strcpy(infilename,argv[3]);
    strcpy(outfilename,argv[4]);
    int para[3];
    Readfile(infilename);
    
    //Init map
    //if process type, use mmap
    if(threadOrprocess[1] == 'p'){
        
        map = (char *)mmap(NULL,sizeof(char)*N*M,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
        nextmap = (char *)mmap(NULL,sizeof(char)*N*M,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    }
    else{
        //if thread type
        map = (char*)malloc(N*M*sizeof(char));
        nextmap = (char*)malloc(N*M*sizeof(char));
    }
    
    InitMap(map);
    
    long long total = N*M;
    long long blocks = total/num;
    pthread_t threads[num];
    Paras *p = (Paras *)malloc(sizeof(Paras)*num);
    //PrintCurrMap(map);
    //epochs
    for(int r = 0;r < Round;r ++){
        int remain = total % num;
        long long count = 0;
        //if thread mode
        if(threadOrprocess[1] == 't'){
            
            finished = 0;
            for(int i = 0;i < num;i ++){
                p[i].start = count;
                count = (remain > 0)? count+blocks+1:count+blocks;
                p[i].end = count;
                remain --;
                pthread_create(&threads[i], NULL, &CountLives, (void *)&p[i]);
            }
            for(int i = 0;i < num;i ++){
                pthread_join(threads[i], NULL);

            }
            
        }
        else{
            //if process mode
            for(int i = 0;i < num;i ++){
                int start = count;
                count = (remain > 0)? count+blocks+1:count+blocks;
                int end = count;
                remain --;
                int ret = fork();
                if(ret == 0){
                    //child process
                    processCountLive(start,end);
                    _exit(0);
                }
            }
            //parent wait
            for(int i = 0;i < num;i ++){
                wait(NULL);
            }
        }
        tmp = map;
        map = nextmap;
        nextmap = tmp;
    }
    
    PrintCurrMap(map,outfilename);
    return 0;
}