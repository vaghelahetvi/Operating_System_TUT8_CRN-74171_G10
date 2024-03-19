#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MEMORY 1024

typedef struct {
    char name[256];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    bool suspended;
} proc;

// Define a structure for FIFO queue node
typedef struct Node {
    proc data;
    struct Node* next;
} Node;

// Define a structure for FIFO queue
typedef struct {
    Node* front;
    Node* rear;
} Queue;

// Function to create a new node
Node* createNode(proc newProc) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = newProc;
    node->next = NULL;
    return node;
}

// Function to create an empty queue
Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to add an element to the rear of the queue
void enqueue(Queue* queue, proc newProc) {
    Node* newNode = createNode(newProc);
    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
        return;
    }
    queue->rear->next = newNode;
    queue->rear = newNode;
}

// Function to remove an element from the front of the queue
proc dequeue(Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\n");
        exit(EXIT_FAILURE);
    }
    Node* temp = queue->front;
    proc data = temp->data;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return data;
}

// Function to check if the queue is empty
bool isEmpty(Queue* queue) {
    return queue->front == NULL;
}

// Function to destroy the queue and free memory
void destroyQueue(Queue* queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}

int main() {
    // Initialize the priority and secondary queues
    Queue* priorityQueue = createQueue();
    Queue* secondaryQueue = createQueue();

    // Initialize the avail_mem array
    int avail_mem[MEMORY] = {0};

    // Read processes from file and populate queues
    FILE* file = fopen("processes_q2.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        proc newProc;
        sscanf(line, "%[^,],%d,%d,%d", newProc.name, &newProc.priority, &newProc.memory, &newProc.runtime);
        newProc.pid = 0;
        newProc.address = 0;
        newProc.suspended = false;

        if (newProc.priority == 0) {
            enqueue(priorityQueue, newProc);
        } else {
            enqueue(secondaryQueue, newProc);
        }
    }
    fclose(file);

    // Execute processes from priority queue first
    while (!isEmpty(priorityQueue)) {
        proc process = dequeue(priorityQueue);
        printf("Executing process: %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n", process.name, process.priority, process.pid, process.memory, process.runtime);

        // Execute process using fork and exec
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            execl(process.name, process.name, NULL);
            // If execl fails
            perror("execl");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            process.pid = pid;
            // Mark memory as used
            for (int i = 0; i < process.memory; i++) {
                avail_mem[i] = 1;
            }
            // Wait for process to terminate
            waitpid(pid, NULL, 0);
            // Free memory
            for (int i = 0; i < process.memory; i++) {
                avail_mem[i] = 0;
            }
        }
    }

    // Execute processes from secondary queue
    while (!isEmpty(secondaryQueue)) {
        proc process = dequeue(secondaryQueue);
        printf("Executing process: %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n", process.name, process.priority, process.pid, process.memory, process.runtime);

        // Execute process using fork and exec if enough memory is available
        if (process.memory <= MEMORY) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                execl(process.name, process.name, NULL);
                // If execl fails
                perror("execl");
                exit(EXIT_FAILURE);
            } else {
                // Parent process
                process.pid = pid;
                // Mark memory as used
                for (int i = 0; i < process.memory; i++) {
                    avail_mem[i] = 1;
                }
                // Wait for process to terminate
                waitpid(pid, NULL, 0);
                // Free memory
                for (int i = 0; i < process.memory; i++) {
                    avail_mem[i] = 0;
                }
            }
        } else {
            // If memory is not available, push process back into secondary queue
            enqueue(secondaryQueue, process);
        }
    }

    // Destroy the queues and free memory
    destroyQueue(priorityQueue);
    destroyQueue(secondaryQueue);

    return 0;
}

