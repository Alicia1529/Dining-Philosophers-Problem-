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
        ;//NOP lpoo
}

void receive_message(MPI_Status *status) {
    MPI_Recv(&clock_val, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
    local_clock = max(local_clock, clock_val);
    local_clock++;
    
}

void send_message(int dest, int tag) {
    MPI_Send(&local_clock, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    local_clock++;
}

/* returns 0 if the local node and its two neighbors have terminated */
int check_termination() {
    if (local_state == DONE && left_state == DONE && right_state == DONE) return 0;
    return 1;
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

    send_message(left, WANNA_CHOPSTICK);
    send_message(right, WANNA_CHOPSTICK);

    while(check_termination()) {
        if ((meals_eaten < NB_MEALS) && (local_state == THINKING)) {
            busy_wait();
            local_clock++;
            local_state = HUNGRY;
        }
        
        receive_message(&status);
        int source = status.MPI_SOURCE;

        // FIRST: handling messages from other nodes
        if (status.MPI_TAG == DONE_EATING) {// 1.1: other node finishes eating
            // printf("%d CASE 1.1: NODE %d -- DONE EATING\n", rank, source);
            if (source == left && left_state != DONE) { // in case of 2 nodes
                left_state = DONE;
                // left_chopstick = 1;
            } else {
                right_state = DONE;
                // right_chopstick = 1;
            }
        } else if (status.MPI_TAG == CHOPSTICK_YOURS) {
            // printf("%d CASE 1.2: NODE %d -- CHOPSTICK_YOURS\n", rank, source);
            if (source == left && left_chopstick == 0) { // in case of 2 nodes
                left_chopstick = 1;
            } else {
                right_chopstick = 1;
            }
        } else {
            // printf("%d CASE 1.3: NODE %d -- REQUESTING\n", rank, source);
            if (local_state == DONE_EATING) { // if the local node is done, then give whatever others want
                send_message(source, CHOPSTICK_YOURS);
            } else { // competing with others for the chopsticks
                int priority = max(rank, source);
                if (clock_val < local_clock) {
                    send_message(source, CHOPSTICK_YOURS);
                } else if (clock_val == local_clock && priority == source) {
                    send_message(source, CHOPSTICK_YOURS);
                } else if (left_chopstick == 0) {
                    left_chopstick = 1;
                } else {
                    right_chopstick = 1;
                }
            }
        }

        // SECOND: discussing own cases
        if (left_chopstick == 1 && right_chopstick == 1 && meals_eaten < NB_MEALS) { // 2.1: if meals_eaten < NB_MEALS and have both chopsticks, start eating
            // eating
            busy_wait(); 
            local_clock++;
            // release the local occupied resources
            left_chopstick = 0;
            right_chopstick = 0;
            // record the eating time
            meal_times[meals_eaten] = local_clock;
            meals_eaten++;
            if (meals_eaten < NB_MEALS) {
                // printf("%d 🌸CASE 2.1 FINISHES EATING DINNER %d at time %d\n", rank, meals_eaten, local_clock);
                local_state = THINKING;
                send_message(right, CHOPSTICK_YOURS);
                send_message(left, CHOPSTICK_YOURS);
            } else {
                // printf("%d 🌸🌸🌸🌸🌸 CASE 2.2 FINISHES EATING ALL THE DINNERS at time %d\n", rank, local_clock);
                local_state = DONE;
                send_message(left, DONE_EATING);
                send_message(right, DONE_EATING);
            }
        } else if (left_chopstick == 0 && meals_eaten < NB_MEALS) { // 2.2: if meals_eaten < NB_MEALS and does not have left chopstick, start requesting left
            // printf("%d CASE 2.3 REQUESTING FROM THE LEFT at time %d\n", rank, local_clock);
            send_message(left, WANNA_CHOPSTICK);
        } else if (right_chopstick == 0 && meals_eaten < NB_MEALS){ // 2.3: if meals_eaten < NB_MEALS and does not have right chopstick, start requesting right
            // printf("%d CASE 2.4 REQUESTING FROM THE RIGHT at time %d\n", rank, local_clock);
            send_message(right, WANNA_CHOPSTICK);
        }
        // 2.4: already finish eating.
    }

    MPI_Send(&meal_times, NB_MEALS, MPI_INT, 0, 99, MPI_COMM_WORLD);
    if (rank == 0) {
        int memo[NB_MEALS];
        for (int i = 0; i < NB; i++) {
            MPI_Recv(&memo, NB_MEALS, MPI_INT, i, 99, MPI_COMM_WORLD, &status);
            for (int j = 0; j < NB_MEALS; j++) {
                printf("Philosopher %d eats meal %d at time %d\n", status.MPI_SOURCE, j, memo[j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}
