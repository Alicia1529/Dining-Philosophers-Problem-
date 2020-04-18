#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//************    TAGS
#define WANNA_CHOPSTICK 0		// chopstick request
#define CHOPSTICK_YOURS 1		// chopstick release
#define DONE_EATING     2		// termination announcement

//************    NODE STATES
#define THINKING 0   // will eat at some point but requesting chopsticks
#define HUNGRY   1   // waiting for chopsticks
#define DONE     2   // will not eat any more

//************   MEALS
#define NB_MEALS 3	// total nb of meals each philosopher must eat

//************   MPI VARIABLES
int NB;               // total nb of philosophers
int rank;             // unique philosopher identifier
int left, right;      // identifiers of the left and right neighbors

//************   TIMING
int local_clock = 0;                    // local clock
int clock_val;                          // timestamp
int meal_times[NB_MEALS];               // meal times

//************   LOCAL STATES
int local_state = THINKING;		// myself
int left_state  = THINKING;		// my left neighbor
int right_state = THINKING;		// my right neighbor

//************   CHOPSTICKS
int left_chopstick = 0;		// I don't hold the left chopstick
int right_chopstick = 0;	// I don't hold the right chopstick

//************   MEALS
int meals_eaten = 0;		// nb de meals eaten locally


//************   FUNCTIONS   ***************************
int max(int a, int b) {
    return (a>b?a:b);
}

/* function you can call to introduce random waiting times */
void busy_wait() {
    int i, w;
    w = rand()%10+1;
    for (i = 0; i < (100000*w) ; i++)
        ;//NOP loop
}

void receive_message(MPI_Status *status) {
    // TODO
}

void send_message(int dest, int tag) {
    // TODO
}

/* returns 0 if the local node and its two neighbors have terminated */
int check_termination() {
    // TODO
}

//************   THE PROGRAM   ***************************
int main(int argc, char* argv[]) {
    
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &NB);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    srand(time(NULL) + rank);
    left = (rank + NB - 1) % NB;
    right = (rank + 1) % NB;
    
    while(check_termination()) {
        
        if ((meals_eaten < NB_MEALS) && (local_state == THINKING)) {
            // TODO
        }
        
        receive_message(&status);
        
        if (status.MPI_TAG == DONE_EATING) {
            // TODO
        } else
            if (status.MPI_TAG == CHOPSTICK_YOURS) {
                // TODO
            } else {
                // TODO
            }
    }
    
    MPI_Finalize();
    return 0;
}
