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
    clock_val = local_clock;
    MPI_Recv(&clock_val, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
    printf("%d --Local Time %d, Other Time %d : receive tag _%d_ from rank _%d_\n", rank, local_clock, clock_val, status->MPI_TAG, status->MPI_SOURCE);
}

void send_message(int dest, int tag) {
    printf("%d --Local Time %d: send tag %d to rank %d\n", rank, local_clock, tag, dest);
    clock_val = local_clock;
    MPI_Send(&clock_val, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

/* returns 0 if the local node and its two neighbors have terminated */
int check_termination() {
    if (local_state == DONE && left_state == DONE && right_state == DONE) return 0;
    if (local_clock > 100) return 0;
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

    clock_val = local_clock;
    while(check_termination()) {
        printf("\n\n---------------- % d ------------------\n\n\n", rank);
        clock_val = local_clock;
        if ((meals_eaten < NB_MEALS) && (local_state == THINKING)) {
            busy_wait();
            local_state = HUNGRY;
            send_message(left, WANNA_CHOPSTICK);
            local_clock++;
        } else if ((meals_eaten < NB_MEALS) && (local_state == HUNGRY)) {
            if (left_chopstick == 1 && right_chopstick == 0) {
                send_message(right, WANNA_CHOPSTICK);
            } else {
                printf("%d state: left %d, right %d (1: what happened???)\n ", rank, left_chopstick, right_chopstick);
            }
        }
        
        receive_message(&status);
        if (local_clock < clock_val) {
            local_clock = clock_val;
        }
        local_clock++;
        
        if (status.MPI_TAG == DONE_EATING) {
            printf("%d CASE 1: DONE EATING\n", rank);
            if (status.MPI_SOURCE == left) {
                left_state = DONE;
            } else if (status.MPI_SOURCE == right) {
                right_state = DONE;
            } else {
                printf("2: what happened??? # %d", rank);
            }
        } else if (status.MPI_TAG == CHOPSTICK_YOURS) {
            printf("%d CASE 2: CHOPSTICK_YOURS\n", rank);
            if (status.MPI_SOURCE == left) {
                left_chopstick = 1;
            } else if (status.MPI_SOURCE == right) {
                right_chopstick = 1;
            } else {
                printf("3: what happened??? # %d", rank);
            }
            // when the response is sent
            // meal_times[meals_eaten] = clock_val;
            printf("# rank %d time %d\n", rank, local_clock);
            if (left_chopstick == 1 && right_chopstick == 1) {
                printf("%d s\n", rank);
                busy_wait();
                left_chopstick = 0;
                right_chopstick = 0;
                send_message(right, CHOPSTICK_YOURS);
                local_clock++;
                send_message(left, CHOPSTICK_YOURS);
                local_clock++;
                printf("$$$$$$$MEALS_EATEN: %d\n", meals_eaten);
                // meal_times[meals_eaten] = local_clock;
                meals_eaten++;
                if (meals_eaten < 3) {
                    local_state = THINKING;
                } else {
                    printf("%d finishes eating at time %d\n", rank, local_clock);
                    local_state = DONE;
                    send_message(left, DONE_EATING);
                    local_clock++;
                    send_message(right, DONE_EATING);
                    local_clock++;
                }
            } else if (left_chopstick == 1) {
                printf("%d only get left chopstick, and that's normal\n", rank);
            } else {
                printf("%d only get right chopstick. what happened??\n", rank);
            }
        } else {
            printf("%d CASE 3: REQUEST\n", rank);
            printf("%d last local timestamp is %d while the request timestamp is %d\n", rank, meal_times[meals_eaten], clock_val);
            if (status.MPI_SOURCE == left && left_chopstick == 0) {
                printf("%d get request from left\n", rank);
                send_message(left, CHOPSTICK_YOURS);
                local_clock++;
            } else if (status.MPI_SOURCE == right && right_chopstick == 0) {
                printf("%d get request from right\n", rank);
                send_message(right, CHOPSTICK_YOURS);
                local_clock++;
            } else if (status.MPI_SOURCE == left && left_chopstick == 1) {
                printf("%d fighting left\n", rank);
                int priority = max(rank, status.MPI_SOURCE);
                if (priority == status.MPI_SOURCE) {
                    printf("%d give up and release\n", rank);
                    left_chopstick = 0;
                    send_message(left, CHOPSTICK_YOURS);
                    local_clock++;
                    local_state = THINKING;
                } 
                // if (clock_val < meal_times[meals_eaten]) {
                //     printf("%d branch 1\n", rank);
                //     left_chopstick = 0;
                //     send_message(left, CHOPSTICK_YOURS);
                //     local_clock++;
                //     local_state = THINKING;
                // } else if (clock_val == meal_times[meals_eaten] && priority == status.MPI_SOURCE) {
                //     printf("%d branch 2\n", rank);
                //     left_chopstick = 0;
                //     send_message(left, CHOPSTICK_YOURS);
                //     local_clock++;
                //     local_state = THINKING;
                // } else {
                //     printf("%d branch 3\n", rank);
                // }
            }
        }
        printf("%d ##### state: %d, left: %d, right: %d\n", rank, local_state, left_chopstick, right_chopstick);
    }
    
    MPI_Finalize();
    return 0;
}
