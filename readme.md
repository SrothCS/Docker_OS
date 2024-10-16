The project is used to implement  a distributed  hash table using our chord protocal. 
The coding will include the features for joining network, used for stabilizing network,
and replicating the data for fault tolerance. 

The code had Node join, Network stabalization,data storage and data replication. 

Our main.c file will contain all the logic as well as getting the data 

Below are the functions used in our code and their implementation. 

Node* createNode(int id): Its used Creates a new node with a given ID.
void* join(void* arg): It Handles the joining of a new node to the network.
void* stabilize(void* arg): It  Stabilizes the network by updating successor and predecessor pointers.
void notify(Node* n, Node* successor):It  Notifies the successor node of the current node's existence.
Node* findSuccessor(Node* n, int id): It Finds the successor of a given ID.
Node* findPredecessor(Node* n, int id): It Finds the predecessor of a given ID.
Node* closestPrecedingFinger(Node* n, int id): It  Finds the closest preceding finger for a given ID.
void storeData(Node* n, int key, const char* value): It Stores data in the network and replicates it for fault tolerance.
char* retrieveData(Node* n, int key): It Retrieves data by key from the network.
void replicateData(Node* n, int key, const char* value):It  Replicates data for fault tolerance.
void printNode(Node* n): Its used to Prints the state of a node.


For coding output, i deployed the code on docker but in my local system 
If we run the command, 
docker run -it name of the folder
in my name, i named my folder  final_project
so,
if i run docker run -it final_project
i will be getting  this as our Output: 

Node 1 is the first node in the network
Node 2 is joining via existing node 1
findPredecessor: current = 1, successor = 1
Node 3 is joining via existing node 1
findPredecessor: current = 1, successor = 1
Join operation time: 0.000397 seconds
Stabilizing node 1
Node 1 is notifying its successor 1
Stabilizing node 2
Node 2 is notifying its successor 1
Stabilizing node 3
Node 3 is notifying its successor 1
Stabilization operation time: 0.000056 seconds
Node 1: successor = 1, predecessor = 1
Node 2: successor = 1, predecessor = -1
Node 3: successor = 1, predecessor = -1
findPredecessor: current = 1, successor = 1
Data stored: key = 10, value = Data for key 10, at node 1
Data replicated: key = 10, value = Data for key 10, at node 1
Data replicated: key = 10, value = Data for key 10, at node 1
Data replicated: key = 10, value = Data for key 10, at node 1
findPredecessor: current = 2, successor = 1
Data stored: key = 20, value = Data for key 20, at node 1
Data replicated: key = 20, value = Data for key 20, at node 1
Data replicated: key = 20, value = Data for key 20, at node 1
Data replicated: key = 20, value = Data for key 20, at node 1
findPredecessor: current = 3, successor = 1
Data stored: key = 30, value = Data for key 30, at node 1
Data replicated: key = 30, value = Data for key 30, at node 1
Data replicated: key = 30, value = Data for key 30, at node 1
Data replicated: key = 30, value = Data for key 30, at node 1
findPredecessor: current = 1, successor = 1
Retrieved data: key = 20, value = Data for key 20