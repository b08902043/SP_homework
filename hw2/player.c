#include <stdio.h>
#include <stdlib.h>
#define N_ROUND 10

int main(int argc, char *argv[])
{
    // Error handling
    if (argc != 3)
    {
        fprintf(stderr, "usage: ./player [player_id]\n");
        return 0;
    }
    // Parse arguments
    int player_id = atoi(argv[2]);
    for (int i = 1; i <= N_ROUND; i++)
    {
        int guess;
        // initialize random seed: 
        srand ((player_id +i) * 323);
        //generate guess between 1 and 1000: 
        guess = rand() % 1001;
        // Send the winner to leaf host
        printf("%d %d\n", player_id, guess);
    }
}
