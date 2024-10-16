#include <stdio.h>  //Its standard IO library 
#include <stdlib.h>  //Used for memory allocation
#include <pthread.h> //For Py threads, threading.
#include <string.h> //Used for string manipulation 
#include <time.h>  //Time related functions

#define MAX_FINGERS 10   // Its used for maximum number of finger table entries per node. 
#define MAX_DATA 100 // Used Maximum number of data items per node
#define REPLICATION_FACTOR 3 // Its used for Number of replicas for fault tolerance

pthread_mutex_t lock;   //Its for mutex locks for synchronization
//Its for structure for representation  with key pair value. 
typedef struct Data {
    int key;
    char value[256];
} Data;
// Used for structure to represent a node in chord ring. 
typedef struct Node {
    int id;
    Data data[MAX_DATA]; // Data storage
    int dataCount; // Number of data items stored
    struct Node* fingers[MAX_FINGERS]; // Finger table
    struct Node* successor;  //Used for successor node
    struct Node* predecessor;  //Used for predecessor node
} Node;
// Used for structure to represent a chord ring.
Node* createNode(int id); // Used as a Function to create a new node
void* join(void* arg);// Used as a Function to join a node to the ring
void* stabilize(void* arg);// Used as a Function to stabilize the ring
void notify(Node* n, Node* successor); // Used as a Function to notify the successor of a node
Node* findSuccessor(Node* n, int id); // Used as a Function to find the successor of a node
Node* findPredecessor(Node* n, int id);  // Used as a Function to find the predecessor of a node
Node* closestPrecedingFinger(Node* n, int id);  // Used as a Function to find the closest preceding finger of a node
void storeData(Node* n, int key, const char* value); // Used as a Function to store data in a node
char* retrieveData(Node* n, int key); // Used as a Function to retrieve data from a node
void replicateData(Node* n, int key, const char* value); // Used as a Function to replicate data to the successor
//It is used for printing the chord ring.
typedef struct {
    Node* node; // As a Node pointer
    Node* existingNode; // Used for existing node
} JoinArgs;  // Used for structure to represent arguments for join function
// It is used for printing the chord ring.
Node* createNode(int id) {  // Used as a Function to create a new node
    Node* newNode = (Node*)malloc(sizeof(Node)); //Used to  Allocate memory for the new node
    newNode->id = id;  // Used to set the ID of the new node
    newNode->successor = newNode; // Used to set the successor of the new node as itself
    newNode->predecessor = NULL; //Used to set the predecessor of the new node as NULL
    newNode->dataCount = 0;  // Used to set the data count of the new node as 0
    for (int i = 0; i < MAX_FINGERS; i++) {
        newNode->fingers[i] = newNode;
    }
    return newNode;
}

// Used to Parallelize the join operation (Condition 1)
void* join(void* arg) {  // Used as a Function to join a node to the ring
    JoinArgs* args = (JoinArgs*)arg;  // Used to cast the argument to the JoinArgs structure
    Node* n = args->node; // Used to get the node from the argument
    Node* existingNode = args->existingNode; // Used to get the existing node from the argument

    pthread_mutex_lock(&lock); // Used to acquire the lock for synchronization
    if (existingNode != NULL) {  // Used to check if the existing node is not NULL
        printf("Node %d is joining via existing node %d\n", n->id, existingNode->id); // Used to print the joining message
        n->predecessor = NULL; // Used to set the predecessor of the new node as NULL
        n->successor = findSuccessor(existingNode, n->id);
    } else { // Used to check if the existing node is NULL
        // Only node in the network
        printf("Node %d is the first node in the network\n", n->id);
        n->predecessor = n;
        n->successor = n; // Used to set the successor of the new node as itself
    }
    pthread_mutex_unlock(&lock);  // Used to release the lock for synchronization
    return NULL; // Used to return NULL
}

// Parallelize the stabilize operation (Condition 1)
void* stabilize(void* arg) {  // Used as a Function to stabilize the ring
    Node* n = (Node*)arg; // Used to get the node from the argument

    pthread_mutex_lock(&lock); // Used to acquire the lock for synchronization
    printf("Stabilizing node %d\n", n->id); // Used to print the stabilizing message
    Node* x = n->successor->predecessor; // Used to get the predecessor of the successor of the node
    if (x != NULL && x->id > n->id && x->id < n->successor->id) {
        n->successor = x;
    } // Used to update the successor of the node
    notify(n, n->successor);
    pthread_mutex_unlock(&lock);
    return NULL;  // Used to return NULL
}

void notify(Node* n, Node* successor) {  // Used as a Function to notify the successor of a node
    printf("Node %d is notifying its successor %d\n", n->id, successor->id);  // Used to print the notifying message
    if (successor->predecessor == NULL || (n->id > successor->predecessor->id && n->id < successor->id)) {
        successor->predecessor = n;  
    }  // Used to update the predecessor of the successor
}  // Used to notify the successor of a node

Node* findSuccessor(Node* n, int id) {  // Used as a Function to find the successor of a node
    Node* pred = findPredecessor(n, id); // Used to get the predecessor of the node
    return pred->successor;
} // Used to find the successor of a node

Node* findPredecessor(Node* n, int id) {
    Node* current = n;
    while (!(id > current->id && id <= current->successor->id)) {
        printf("findPredecessor: current = %d, successor = %d\n", current->id, current->successor->id);
        Node* next = closestPrecedingFinger(current, id);
        if (next == current) {
            break;
        } // Used to check if the next node is the current node
        current = next;
    }
    return current;
} // Used to find the predecessor of a node
// Used to find the closest preceding finger of a node
Node* closestPrecedingFinger(Node* n, int id) {
    for (int i = MAX_FINGERS - 1; i >= 0; i--) {
        if (n->fingers[i]->id > n->id && n->fingers[i]->id < id) {
            printf("closestPrecedingFinger: node %d found closer finger %d for id %d\n", n->id, n->fingers[i]->id, id);
            return n->fingers[i];
        }
    } // Used to find the closest preceding finger of a node
    return n;
}

// Store data and replicate for fault tolerance (Condition 3)
void storeData(Node* n, int key, const char* value) {
    pthread_mutex_lock(&lock);  // Used to acquire the lock for synchronization
    Node* targetNode = findSuccessor(n, key);
    if (targetNode->dataCount < MAX_DATA) {
        targetNode->data[targetNode->dataCount].key = key;
        strcpy(targetNode->data[targetNode->dataCount].value, value);
        targetNode->dataCount++;
        printf("Data stored: key = %d, value = %s, at node %d\n", key, value, targetNode->id);
        replicateData(targetNode, key, value); // Used to Replicate data for fault tolerance
    } else {
        printf("Data storage failed: No space available at node %d\n", targetNode->id);// Used to print the data storage failed message
    }
    pthread_mutex_unlock(&lock); // Used to release the lock for synchronization
}

char* retrieveData(Node* n, int key) { // Used as a Function to retrieve data from a node
    pthread_mutex_lock(&lock); // Used to acquire the lock for synchronization
    Node* targetNode = findSuccessor(n, key); // Used to get the successor of the node
    for (int i = 0; i < targetNode->dataCount; i++) {
        if (targetNode->data[i].key == key) {
            pthread_mutex_unlock(&lock);
            return targetNode->data[i].value;
        } // Used to retrieve the data from the node
    }
    pthread_mutex_unlock(&lock); // Used to release the lock for synchronization
    return NULL; // Used to return NULL
}

void replicateData(Node* n, int key, const char* value) {
    Node* current = n; // Used to get the current node
    for (int i = 0; i < REPLICATION_FACTOR; i++) {
        current = current->successor; // Used to get the successor of the current node
        if (current->dataCount < MAX_DATA) { // Used to check if the data count is less than the maximum data
            current->data[current->dataCount].key = key; // Used to store the data in the current node
            strcpy(current->data[current->dataCount].value, value);
            current->dataCount++; // Used to increment the data count
            printf("Data replicated: key = %d, value = %s, at node %d\n", key, value, current->id);
        } else {
            printf("Data replication failed: No space available at node %d\n", current->id);
        } // Used to print the data replication failed message
    }
}
    // Used to print the chord ring


void printNode(Node* n) {
    printf("Node %d: successor = %d, predecessor = %d\n",
           n->id, n->successor->id, n->predecessor ? n->predecessor->id : -1);
}

int main() {
    pthread_t threads[6];
    JoinArgs joinArgs[3];

    pthread_mutex_init(&lock, NULL);

    // Used to Create initial nodes
    Node* node1 = createNode(1);
    Node* node2 = createNode(2);
    Node* node3 = createNode(3);

    // Used to Prepare join arguments
    joinArgs[0].node = node1;
    joinArgs[0].existingNode = NULL;
    joinArgs[1].node = node2;
    joinArgs[1].existingNode = node1;
    joinArgs[2].node = node3;
    joinArgs[2].existingNode = node1;

    // Used to Measure time for join operation (Condition 2)
    clock_t start_time = clock();

    //  Used to Join nodes using threads (Condition 1)
    pthread_create(&threads[0], NULL, join, &joinArgs[0]);
    pthread_create(&threads[1], NULL, join, &joinArgs[1]);
    pthread_create(&threads[2], NULL, join, &joinArgs[2]);

    // Used to Wait for join threads to finish
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end_time = clock();
    double join_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Join operation time: %f seconds\n", join_time);

    // Used  Measure time for stabilization operation (Condition 2)
    start_time = clock();

    // Uses Stabilize nodes using threads (Condition 1)
    pthread_create(&threads[3], NULL, stabilize, node1);
    pthread_create(&threads[4], NULL, stabilize, node2);
    pthread_create(&threads[5], NULL, stabilize, node3);

    // Wait for stabilize threads to finish
    for (int i = 3; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }

    end_time = clock();
    double stabilize_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Stabilization operation time: %f seconds\n", stabilize_time);

    // Print node states 
    printNode(node1);
    printNode(node2);
    printNode(node3);

    // Store and retrieve data
    storeData(node1, 10, "Data for key 10");
    storeData(node2, 20, "Data for key 20");
    storeData(node3, 30, "Data for key 30");

    char* value = retrieveData(node1, 20);
    if (value) {
        printf("Retrieved data: key = 20, value = %s\n", value);
    } else {
        printf("Data not found for key = 20\n");
    }

    // Used for Free memory
    free(node1);
    free(node2);
    free(node3);

    pthread_mutex_destroy(&lock);

    return 0;
}
//Compile the code.


