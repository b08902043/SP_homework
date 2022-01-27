#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define MAXLEN 128

int id;
int depth;
char lucky_num_string[MAXLEN];
char id_string[MAXLEN];
int lucky_num;
FILE *readchild[2], *writechild[2];
void forkChildren(int new_depth, int player_id[],int player_or_host)
{
    char depth_string[MAXLEN];
    char player[2][MAXLEN];
    //host or player
    if (player_or_host == 0){
        sprintf(depth_string, "%d", new_depth);
    }
    else{
        for (int i = 0; i < 2; i++){
            sprintf(player[i], "%d", player_id[i]);
        }
    }    
    // Fork children
    int write_to_child[2][2], read_from_child[2][2];
    // Build pipes
    for(int i = 0;i < 2;i ++){
        pipe(write_to_child[i]);
        pipe(read_from_child[i]);
    }
    // Fork first child
    if (fork() == 0)
    {
        close(write_to_child[0][1]);
        close(read_from_child[0][0]);
        // Redirect stdout & stdin
        dup2(write_to_child[0][0], STDIN_FILENO);
        close(write_to_child[0][0]);
        dup2(read_from_child[0][1], STDOUT_FILENO);
        close(read_from_child[0][1]);

        // Exec
        if (player_or_host == 0){
            execl("./host", "host", "-m",id_string,"-d",depth_string,"-l",lucky_num_string, NULL);
        }
        else{
            execl("./player", "player","-n", player[0], NULL);
        }   
    }
    //fork second child
    if (fork() == 0)
    {   
        close(write_to_child[1][1]);
        close(read_from_child[1][0]);
        dup2(write_to_child[1][0], STDIN_FILENO);
        close(write_to_child[1][0]);
        dup2(read_from_child[1][1], STDOUT_FILENO);
        close(read_from_child[1][1]);

        // Exec
        if (player_or_host == 0){
            execl("./host", "host", "-m",id_string,"-d",depth_string,"-l",lucky_num_string, NULL);
        }
        else{
            execl("./player", "player","-n", player[1], NULL);
        }   
    }
    //close parent write[0] and read[1]
    for(int i = 0;i < 2;i ++){
        close(write_to_child[i][0]);
        close(read_from_child[i][1]);
    }
    
    // Convert fd to filestream
    for (int i = 0; i < 2; i++)
    {
        readchild[i] = fdopen(read_from_child[i][0], "r");
        writechild[i] = fdopen(write_to_child[i][1], "w");
        setbuf(writechild[i], NULL);
    }
}

FILE *fifo_read;
FILE *fifo_write;

void set_fifo()
{
    char fifo_name[MAXLEN];
    sprintf(fifo_name, "fifo_%s.tmp", id_string);
    fifo_read = fopen(fifo_name, "r");
    fifo_write = fopen("fifo_0.tmp", "w");
}

typedef struct point
{
    int id, score;
} Player;
void Winner(int *winner, int *winner_guess)
{
    //player and their guess
    int player[2], guess[2];
    for (int i = 0; i < 2; i++){
        fscanf(readchild[i], "%d%d", &player[i], &guess[i]);
    }
    //find the distance of guess and lucky number
    if(depth == 2){
        guess[0] -= lucky_num;
        guess[1] -= lucky_num;
        guess[0] = (guess[0] < 0)? -guess[0]:guess[0];
        guess[1] = (guess[1] < 0)? -guess[1]:guess[1];
    }
    *winner = player[0], *winner_guess = guess[0];
    if (guess[1] < guess[0] || (guess[1] == guess[0] && player[1] < player[0]))
    {
        *winner = player[1];
        *winner_guess = guess[1];
    }
}

int cmp(const void *a, const void *b)
{
    return ((Player *)b)->score < ((Player *)a)->score;
}

int main(int argc, char *argv[])
{
    //argument convert to string & integer
    
    id = atoi(argv[2]);
    depth = atoi(argv[4]);
    lucky_num = atoi(argv[6]);
    sprintf(lucky_num_string,"%d",lucky_num);
    sprintf(id_string,"%d",id);
    if (depth == 0)
    {
        //printf("test\n");
        // Root host
        //set read fifo from gamesystem and write fifo to game system
        set_fifo();
        forkChildren(1, NULL,0);
        while (1)
        {
            // Get player ids
            int player_id[8] = {};
            for (int i = 0; i < 8; i++)
            {
                fscanf(fifo_read, "%d", &player_id[i]);
            }
            //send end message
            if (player_id[0] == -1)
            {
                fprintf(writechild[0], "-1 -1 -1 -1\n");
                fprintf(writechild[1], "-1 -1 -1 -1\n");
                break;
            }

            // pass player ids to child
            char to_child1[MAXLEN] = "", to_child2[MAXLEN] = "";
            sprintf(to_child1, "%d %d %d %d\n", player_id[0], player_id[1], player_id[2], player_id[3]);
            sprintf(to_child2, "%d %d %d %d\n", player_id[4], player_id[5], player_id[6], player_id[7]);
            fprintf(writechild[0], "%s", to_child1);
            fprintf(writechild[1], "%s", to_child2);
            //printf("%s%s",to_child1,to_child2);
            // Init player's point
            Player point[13];
            //initResult(result, player_id);
            for (int i = 0; i < 13; i++){
                point[i].id = i;
                point[i].score = -1;
            }
            for (int i = 0; i < 8; i++){
                point[player_id[i]].score = 0;
            }
            // Get result
            for (int i = 1; i <= 10; i++)
            {
                // Get the winner
                int winner, winner_guess;
                Winner(&winner, &winner_guess);
                // Add the score of the winner
                point[winner].score+=10;
            }

            //Print result
            //printResult(point);
            qsort(point, 13, sizeof(Player), cmp);
            // send score back to game system
            fprintf(fifo_write, "%s\n", id_string);
            for (int i = 0; i < 13; i++)
            {
                if (point[i].score != -1)
                {
                    fprintf(fifo_write, "%d %d\n", point[i].id, point[i].score);
                }
            }
            fflush(fifo_write);
        }
        
    }
    else if (depth == 1)
    {
        forkChildren(2, NULL, 0);

        while (1)
        {
            int player_id[4];
            for (int i = 0; i < 4; i++)
                scanf("%d", &player_id[i]);

            if (player_id[0] == -1)
            {
                fprintf(writechild[0], "-1 -1\n");
                fprintf(writechild[1], "-1 -1\n");
                break;
            }

            char to_child1[MAXLEN] = "", to_child2[MAXLEN] = "";
            sprintf(to_child1, "%d %d\n", player_id[0], player_id[1]);
            sprintf(to_child2, "%d %d\n", player_id[2], player_id[3]);
            fprintf(writechild[0], "%s", to_child1);
            fprintf(writechild[1], "%s", to_child2);

            for (int i = 1; i <= 10; i++)
            {
                int winner, winner_guess;
                Winner(&winner, &winner_guess);
                printf("%d %d\n", winner, winner_guess);
            }
            fflush(stdout);
        }
    }
    else
    {
        while (1)
        {
            int player_id[2];
            for (int i = 0; i < 2; i++)
                scanf("%d", &player_id[i]);

            if (player_id[0] == -1)
                break;

            forkChildren(-1, player_id, 1);

            for (int i = 1; i <= 10; i++)
            {
                int winner, winner_guess;
                Winner(&winner, &winner_guess);
                printf("%d %d\n", winner, winner_guess);
            }
            fflush(stdout);
            while (wait(NULL) > 0) {}
        }
    }
    while (wait(NULL) > 0) {}
    return 0;
}
