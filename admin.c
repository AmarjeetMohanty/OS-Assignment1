#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/types.h>

// Shared memory name
#define SHM_NAME "/hotel_manager"

// Shared memory structure
typedef struct {
    int close_hotel;
} SharedData;

int main() {
    char input;
    SharedData *shared_data;

    // Generate a key
    key_t key = ftok("admin.c", 'R');
    if (key == -1) {
        perror("ftok failed\n");
        return 1;
    }

    // Create the shared memory segment
    int shm_id = shmget(key, sizeof(SharedData), 0644 | IPC_CREAT);
    if (shm_id == -1) {
        perror("shmget error");
        return 1;
    }

     // Attach to the shared memory
    shared_data = (SharedData *)shmat(shm_id, (void *)0, 0644);
    if (shared_data == (SharedData *)(-1)) {
        perror("shmat");
        return 1;
    }

     // Initialize the shared memory
    shared_data->close_hotel = 0;

    while(1){
        printf("Enter option");
        scanf("%c", &input);

        // conditional statement
       if(input == 'Y' || input == 'y'){
            shared_data->close_hotel = 1;
            printf("Hotel is closed\n");
            //detach from shared memory
            if(shmdt(shared_data) == -1){
                perror("shmdt");
                return 1;
            }
            
        }
        else if(input == 'N' || input == 'n'){
            shared_data->close_hotel = 0;
            printf("Hotel is open\n");
            // goto to the question again
            continue;
        }
        else{
            printf("Invalid input\n");
        }

        return 0;
    }



