#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<string.h>

#define MAX_TABLES 10
#define BUFF_SIZE 1024

typedef struct {
    int itemNumber;
    char itemName[500];
    } Order;
/* 
In this context, Order** orders; is being used to create a dynamic two-dimensional array. Here's how it works:

The first level of indirection (*) is used to create an array of pointers. Each of these pointers can be thought of as representing a customer at the table.

The second level of indirection (**) is used to make each of these pointers point to another array. Each of these second arrays represents the orders placed by a particular customer.

So, in essence, Order** orders; creates a table where each row represents a customer and each column in a row represents an order placed by that customer.

The Order structure would typically contain details about each order, such as the food item ordered, the quantity, and perhaps other details like special instructions for the order.

This structure allows for flexibility in the number of customers and the number of orders per customer, as these can be dynamically allocated and deallocated at runtime. This wouldn't be possible with a regular two-dimensional array, which requires the size to be known at compile time.
*/

typedef struct {
    int tableNumber;
    int numCustomer;
    Order** orders;
} Table;

#include <unistd.h>

void create_customer_process(int num_customers, int table_number) {
    // TODO: Create customer processes and pipes for IPC
    int i;
    int pipefd[num_customers][2]; // Array of pipes

    for (i = 0; i < num_customers; i++) {
        if (pipe(pipefd[i]) == -1) {
            perror("Pipe creation failed");
            exit(1);
        }

        int pid = fork();
        if (pid == 0) {
            // Child process
            printf("Customer %d is ordering\n", i);
            close(pipefd[i][0]); // Close the read end of the pipe
            // TODO: Write customer's order to the pipe
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
    }

    for (i = 0; i < num_customers; i++) {
        close(pipefd[i][1]); // Close the write end of the pipe
        wait(NULL);
    }
}

void read_menu_and_display() {
    // TODO: Read menu.txt and display the menu
    FILE* file = fopen("menu.txt", "r");
    char line[500];
    while(fgets(line, 500, file)){
        printf("%s", line);
    }
    fclose(file);
    
}

void take_orders_from_customers(int num_customers, int table_number) {
    // TODO: Take orders from customers
    /*
    1. Veg Burger 30 INR
    2. Chicken Burger 40 INR
    3. Ostrich Eggs 25 INR
    4. Egg Frankie 30 INR
    */
   // Customer keeps inputing the order index until he presses -1 keep taking order
    int order;
    int i;
    for(i = 0; i < num_customers; i++){
        // Enter the serial number(s) of  the item(s) to order from the emnu . Enter -1 when done
        printf("Enter the serial number(s) of  the item(s) to order from the nemu . Enter -1 when done\n");
        while(1){
            scanf("%d", &order);
            if(order == -1){
                break;
            }
            Table* tables[MAX_TABLES]; // Declare the "tables" variable
            
            tables[table_number]->orders[i] = (Order*)malloc(sizeof(Order));
            tables[table_number]->orders[i]->itemNumber = order;
            
        }
            
    }
        
}

void communicate_order_to_waiter(Table *table) {
    // TODO: Communicate the order to the waiter process using shared memory
    int shmid;
    key_t key = ftok("waiter.c", table->tableNumber);
    if (key == -1) {
        perror("Error in ftok");
        // Handle the error here
        return 1;
    }
    int* shmPtr;
    shmid = shmget(key, BUFF_SIZE, 0644|IPC_CREAT);
    if (shmid == -1) {
        perror("Error in shmget");
        // Handle the error here
        return 1;
    }
    shmPtr = shmat(shmid, (void*)0, 0);
    if (shmPtr == (int*)-1) {
        perror("Error in shmat");
        // Handle the error here
        return 1;
    }
    
    // Write the order to the shared memory
    int i;
    for (i = 0; i < table->numCustomer; i++) {
        sprintf(shmPtr, "%d", table->orders[i]->itemNumber);
        shmPtr += strlen(shmPtr) + 1;
    }
    // Write a -1 to indicate the end of the order
    sprintf(shmPtr, "-1");
    shmdt(shmPtr);
    return 0;
}

void receive_and_display_bill(Table *table) {
    // TODO: Receive the total bill amount from the waiter process and display it
    int shmid;
    key_t key = ftok("waiter.c", table->tableNumber);
    if (key == -1) {
        perror("Error in ftok");
        // Handle the error here
        return 1;
    }
    int* shmPtr;
    shmid = shmget(key, BUFF_SIZE, 0644|IPC_CREAT);
    if (shmid == -1) {
        perror("Error in shmget");
        // Handle the error here
        return 1;
    }
    shmPtr = shmat(shmid, (void*)0, 0);
    if (shmPtr == (int*)-1) {
        perror("Error in shmat");
        // Handle the error here
        return 1;
    }
    // Read the bill amount from the shared memory
    printf("The total bill amount is: %s\n", shmPtr);
    shmdt(shmPtr);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;

}



int main(){
    int numTables = 0;
    Table* tables[MAX_TABLES];
    while(numTables< MAX_TABLES){
        tables[numTables] = (Table*)malloc(sizeof(Table));
        tables[numTables]->tableNumber = numTables;
        tables[numTables]->numCustomer = 0;
        tables[numTables]->orders = (Order**)malloc(sizeof(Order*));
        numTables++;
        int num_customers;
        int table_number;
        printf("Enter the number of customers: ");
        scanf("%d", &num_customers);
        printf("Enter the table number: ");
        scanf("%d", &table_number);
        //update the table with the number of customers
        tables[table_number]->numCustomer = num_customers;
        create_customer_process(num_customers, table_number);
        read_menu_and_display();
        take_orders_from_customers(num_customers, table_number);
        communicate_order_to_waiter(tables[table_number]);
        receive_and_display_bill(tables[table_number]);
    }
    
}



