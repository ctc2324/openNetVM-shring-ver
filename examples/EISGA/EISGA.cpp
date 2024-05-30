#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Define constants for maximum length, VNF count, top VNF count, SFC count, maximum sharing count, and maximum nodes
#define MAX_LENGTH 3
#define SFC_COUNT 3
#define MAX_SHARING_COUNT 3
#define MAX_NODES 1000
#define VNF_COUNT 5
#define PI 1.570796325

int TOP_VNF_COUNT = 3;
// Define the structure for Virtual Network Function (VNF)
typedef struct {
    int id;
    float reliability;
    int priority;
    int sharedCount;
} VNF;

// Define the linked list node structure for Service Function Chain (SFC)
typedef struct SFCNode {
    int vnfID;
    struct SFCNode* next[100];
    float reliability;
    int cen;
    int head;
} SFCNode;



// Array to store all SFC nodes
SFCNode* allNodes[MAX_NODES];
int nodeCount = 0;
VNF vnfSet[VNF_COUNT];
struct SFCNode* removeNode[MAX_NODES];
int removecnt = 0;
int recursive_time = 0;

float shrinkNode(SFCNode* source, SFCNode* destination,float reliability,int shrinkcnt,struct SFCNode* shrinked[]);
float deleteNode(SFCNode* source, SFCNode* destination, float reliability,int shrinkcnt,struct SFCNode* shrinked[]);
float nodeDeleting(int source,int v,int Matrix[MAX_NODES][MAX_NODES]) ;
float nodeShrinking(int source,int v,int Matrix[MAX_NODES][MAX_NODES]) ;
// Function to read VNF reliability from file
void readVNFs(const char *filename, VNF vnfSet[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open file for reading.\n");
        return;
    }

    int vnfCount;
    for (int i = 0; i < VNF_COUNT; i++) {
        fscanf(file, "%d %f", &vnfSet[i].id, &vnfSet[i].reliability);
    }

    fclose(file);
}


// Function to find the top VNFs based on reliability
void findTopVNFs(VNF vnfSet[], int topCount) {
    for (int i = 0; i < VNF_COUNT - 1; i++) {
        for (int j = 0; j < VNF_COUNT - i - 1; j++) {
            if (vnfSet[j].reliability < vnfSet[j + 1].reliability) {
                VNF temp = vnfSet[j];
                vnfSet[j] = vnfSet[j + 1];
                vnfSet[j + 1] = temp;
            }
        }
    }

    printf("Top %d VNFs by reliability:\n", topCount);
    for (int i = 0; i < topCount; i++) {
        printf("VNF %d (Reliability: %lf)\n", vnfSet[i].id, vnfSet[i].reliability);
    }
}

// Function to initialize VNFs
void initVNF(VNF vnfSet[]) {
    for (int i = 0; i < VNF_COUNT; i++) {
        switch (i) {
            case 0:
            case 1:
                vnfSet[i].priority = 0;
                break;
            case 2:
                vnfSet[i].priority = 1; 
                break;
            case 4:
                vnfSet[i].priority = 2;
                break;
            case 3:
                vnfSet[i].priority = -1;
                break;
        }
    }
}

// Function to swap two elements
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Partition function for quicksort
int partition(VNF vnfSet[], int *sfc, int low, int high) {
    int pivotIndex = sfc[high];
    int pivot = vnfSet[pivotIndex].priority;
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (vnfSet[sfc[j]].priority < pivot) {
            i++;
            swap(&sfc[i], &sfc[j]);
        }
    }
    swap(&sfc[i + 1], &sfc[high]);
    return i + 1;
}

// Quicksort function to sort SFC
void quickSort(VNF vnfSet[], int *sfc, int low, int high) {
    if (low < high) {
        int pi = partition(vnfSet, sfc, low, high);

        quickSort(vnfSet, sfc, low, pi - 1);
        quickSort(vnfSet, sfc, pi + 1, high);
    }
}

// Function to sort SFC by priority
void sortVNFBySFC(VNF vnfSet[], int *sfc, int sfcLength) {
    quickSort(vnfSet, sfc, 0, sfcLength - 1);
}


// Function to initialize Service Function Graph (SFG)
void initSFG() {
    SFCNode* SourceNode = (SFCNode*)malloc(sizeof(SFCNode));
    SFCNode* DestinationNode = (SFCNode*)malloc(sizeof(SFCNode));
    SourceNode->vnfID = -1;
    DestinationNode->vnfID = -1;
    SourceNode->reliability = 1;
    DestinationNode->reliability = 1;
    int SFCCnt = 0;
    for(int i = 0;i < 100;i++){
        SourceNode->next[i] = NULL;
        DestinationNode->next[i] = NULL;
    }
    for (int i = 0; i < nodeCount; i++) {
        if(i == 0){
            SourceNode->next[SFCCnt] = allNodes[i];
            SFCCnt++;
        }
        if(allNodes[i]->next[0] == NULL){
            allNodes[i]->next[0] = DestinationNode;
            if(i + 1 < nodeCount){
                SourceNode->next[SFCCnt] = allNodes[i + 1];
                SFCCnt++;
                SourceNode->next[SFCCnt] = NULL;
            }
        }
    }
    DestinationNode->next[0] = NULL;
    allNodes[nodeCount++] = SourceNode;
    allNodes[nodeCount++] = DestinationNode;
}

// Function to create a new SFC node
SFCNode* createSFCNode(int vnfID,bool head) {
    SFCNode* newNode = (SFCNode*)malloc(sizeof(SFCNode));
    newNode->vnfID = vnfID;
    newNode->reliability = vnfSet[vnfID].reliability;
    if(head){
        newNode->head = 1;
    }
    else{
        newNode->head = 0; 
    }

    for (int i = 0; i < 100; i++) {
        newNode->next[i] = NULL;
    }
    allNodes[nodeCount++] = newNode;
    return newNode;
}


// Function to print an SFC
void printSFC(SFCNode* head) {
    SFCNode* current = head;
    while (current != NULL) {
        printf("%p: vnfID: %d, next: ", (void*)current, current->vnfID);
        for (int i = 0; i < MAX_NODES && current->next[i] != NULL; i++) {
            printf("%p", (void*)current->next[i]);
            if (current->next[i + 1] != NULL) {
                printf(", ");
            }
        }
        printf("\n");
        current = current->next[0];
    }
}


// Function to insert a new SFC node at the end
void insertSFCNode(SFCNode** head, int vnfID) {
    if (*head == NULL) {
        *head = createSFCNode(vnfID,true);
    } else {
        SFCNode* current = *head;
        while (current->next[0] != NULL) {
            current = current->next[0];
        }
        SFCNode* newNode = createSFCNode(vnfID,false);
        current->next[0] = newNode;
    }
}

// Function to read SFCs from file and insert nodes
void readSFCs(const char *filename, VNF vnfSet[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open file for reading.\n");
        return;
    }

    int sfcCount;

    for (int i = 0; i < SFC_COUNT; i++) {
        int length;
        fscanf(file, "%d", &length);

        int sfc[MAX_LENGTH]; // Array to store SFC nodes
        for (int j = 0; j < length; j++) {
            fscanf(file, "%d", &sfc[j]);
        }

        
        // Sort the SFC nodes by priority
        sortVNFBySFC(vnfSet, sfc, length);

        
        // Insert the sorted SFC nodes into the linked list
        SFCNode* sfcHead = NULL;
        for (int j = 0; j < length; j++) {
            insertSFCNode(&sfcHead, sfc[j]);
        }

        printf("SFC%d: \n", i);
        printSFC(sfcHead);
    }

    fclose(file);
}


//Function to delete unuse node
void removeSharedNode(int index) {
    if (index >= nodeCount) {
        printf("Error: Index out of bounds.\n");
        return;
    }
    free(allNodes[index]);

    for (int i = index; i < nodeCount - 1; i++) {
        allNodes[i] = allNodes[i + 1];
    }
    
    nodeCount--;
}

// Function to remove duplicate next pointers in allNodes
void removeDuplicateNext() {
    for (int i = 0; i < nodeCount; i++) {
        SFCNode* current = allNodes[i];
        if (current == NULL) continue;
        
        for (int j = 0; current->next[j] ; j++) {
            SFCNode* nextNode = current->next[j];
            if (nextNode == NULL) break;
            
            for (int k = j + 1; current->next[k] ; k++) {
                if (current->next[k] == nextNode) {
                    // Shift pointers to fill the gap
                    int l = k;
                    for (l = k; current->next[l + 1] ; l++) {
                        current->next[l] = current->next[l + 1];
                    }
                    current->next[l] = NULL;
                    k--;
                }
            }
        }
    }
}

// Function to remove duplicate next pointers in allNodes
void removeUnuseNode() {
    for (int i = 0; i < nodeCount; i++) {
        SFCNode* current = allNodes[i];
        if (current == NULL) continue;
        
        for (int j = 0; j < removecnt ; j++) {
            SFCNode* currentRemove = removeNode[j];
            if (currentRemove == NULL) break;
            
            for (int k = 0; current->next[k] ; k++) {
                if (current->next[k] == currentRemove) {
                    int l = k + 1;
                    for (l = k; current->next[l + 1] ; l++) {
                        current->next[l] = current->next[l + 1];
                    }
                    current->next[l] = NULL;
                    break;
                }
            }
        }
    }
}


void updateNextPointers(int top) {
    SFCNode* sharingNode = NULL;
    int shareCnt = 0;
    bool first = false;
    
    
    for (int i = 0; i < nodeCount; i++) {
        if (allNodes[i]->vnfID == top) {
            if(first){
                sharingNode->next[shareCnt] = allNodes[i]->next[0];
                removeNode[removecnt] = allNodes[i];
                removecnt++;
                removeSharedNode(i);
                shareCnt++;
                i--;
                sharingNode->reliability = sharingNode->reliability * sharingNode->reliability;
            }
            else{
                sharingNode = allNodes[i];
                first = true;
                shareCnt++;
            }
        } 
        else if (first && allNodes[i]->next[0]) {
            for (int j = 0;j < MAX_NODES && allNodes[i]->next[j] != NULL; j++) {
                if ((allNodes[i]->next[j])->vnfID == sharingNode->vnfID) {
                    allNodes[i]->next[j] = sharingNode;
                }
                
            }
        }
        if(shareCnt >= MAX_SHARING_COUNT){
            first = false;
            shareCnt = 0;
        }
    }
    if(shareCnt > 0){
        sharingNode->next[shareCnt++] = NULL;
    }
    
}

// Function to print node information from allNodes array
void printNodeInfo() {
    printf("Node Information:\n");
    for (int i = 0; i < nodeCount; i++) {
        printf("Node %d-Address: %p, vnfID: %d,reliability: %f ,next: ", i, (void*)allNodes[i], allNodes[i]->vnfID, allNodes[i]->reliability);
        for (int j = 0; j < MAX_NODES && allNodes[i]->next[j]; j++) {
            printf("%p", (void*)allNodes[i]->next[j]);
            if (allNodes[i]->next[j + 1] != NULL) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

int adjacencyMatrix[MAX_NODES][MAX_NODES];
// Function to generate adjacency matrix for allNodes
void generateAdjacencyMatrix() {
    printf("Adjacency Matrix:\n");

    // Create and initialize adjacency matrix
    for (int i = 0; i < nodeCount; i++) {
        for (int j = 0; j < nodeCount; j++) {
            adjacencyMatrix[i][j] = 0;
        }
    }

    // Populate adjacency matrix based on node connections
    for (int i = 0; i < nodeCount; i++) {
        SFCNode* currentNode = allNodes[i];
        if (currentNode != NULL) {
            for (int j = 0; currentNode->next[j] != NULL && j < MAX_NODES; j++) {
                SFCNode* neighborNode = currentNode->next[j];
                // Find index of neighborNode in allNodes
                int neighborIndex = -1;
                for (int k = 0; k < nodeCount; k++) {
                    if (allNodes[k] == neighborNode) {
                        neighborIndex = k;
                        break;
                    }
                }
                if (neighborIndex != -1) {
                    adjacencyMatrix[i][neighborIndex] = 1;
                }
            }
        }
    }

    // Print adjacency matrix
    for (int i = 0; i < nodeCount; i++) {
        for (int j = 0; j < nodeCount; j++) {
            printf("%d ", adjacencyMatrix[i][j]);
        }
        printf("\n");
    }
}

void printMatrix(int Matrix[MAX_NODES][MAX_NODES]){
    // Print adjacency matrix
    for (int i = 0; i < nodeCount; i++) {
        for (int j = 0; j < nodeCount; j++) {
            printf("%d ", Matrix[i][j]);
        }
        printf("\n");
    }
}

// Function to simulate SFC transmission and calculate reliability using Monte Carlo simulation
float calculateReliability(int simulationCount) {
    int successfulTransmissions = 0;

    for (int i = 0; i < simulationCount; i++) {
        // Generate a random SFC
        SFCNode *sourceNode  = allNodes[nodeCount - 2];
        SFCNode *destinationNode  = allNodes[nodeCount - 1];


        // Generate a random number between 0 and 1
        SFCNode *currentNode = sourceNode;
        bool transmissionSuccessful = true;
        
        while (currentNode != destinationNode) {
            // Choose a random next node from the available options
            int cnt = 0;
            while(currentNode->next[cnt]){
                cnt++;
            }
            int nextIndex = rand() % cnt;
            if (currentNode->next[nextIndex] != NULL) {
                currentNode = currentNode->next[nextIndex];
                // Check the reliability of the current node
                float randomValue = (float)rand() / RAND_MAX;
                if (randomValue >= currentNode->reliability) {
                    // Transmission failed at this node
                    transmissionSuccessful = false;
                    break;
                }
            } else {
                // No next node available, transmission failed
                transmissionSuccessful = false;
                break;
            }
        }

        // Check if the transmission was successful until the destination node
        if (transmissionSuccessful) {
            successfulTransmissions++;
        }
        
        
    }

    // Calculate reliability as the ratio of successful transmissions to total simulations
    float reliability = (float)successfulTransmissions / simulationCount;
    return reliability;
}

void Setcentrality(){
    int cnt = 0;
    int cen = 0;
    for(int i = 0;i < nodeCount - 2;i++){
        for(int j = 0;j < nodeCount;j++){
            if(adjacencyMatrix[i][j] == 1)cen++;
            if(adjacencyMatrix[j][i] == 1)cen++;
        }
        allNodes[i]->cen = cen;
        cen = 0;
    }
    return;
}

void Backup_node(int saving){
    int backupcnt = 0;
    int backup_node[nodeCount];
    int i = 0;
    for(int i = 0;i < nodeCount - 2;i++){
        if(allNodes[i]->next[1]){
            backup_node[backupcnt] = i;
            backupcnt++;
            allNodes[i]->reliability = 1 - ((1 - allNodes[i]->reliability) * (1 - allNodes[i]->reliability));
        }
    }

    while(backupcnt < saving){
        float maxrel = 0;
        float minrel = 1;
        float maxcen = 0;
        int maxcenIndex = 0;
        for(int i = 0;i < nodeCount - 2;i++){
            if(allNodes[i]->reliability < minrel){
                minrel = allNodes[i]->reliability;
            }
            else if(allNodes[i]->reliability > maxrel){
                maxrel = allNodes[i]->reliability;
            }
        }
        float maxmin = maxrel - minrel;
        for(int i = 0;i < nodeCount - 2;i++){
            float cen;
            cen = allNodes[i]->cen * cos(PI * ((allNodes[i]->reliability - minrel) / maxmin));
            if(cen > maxcen){
                maxcen = cen;
                maxcenIndex = i;
            }
        }
        backup_node[backupcnt] = maxcenIndex;
        backupcnt++;
        allNodes[maxcenIndex]->reliability = 1 - ((1 - vnfSet[allNodes[maxcenIndex]->vnfID].reliability) * (1 - allNodes[maxcenIndex]->reliability));
    }
}

int findnext(int NFcnt[],SFCNode* next){
    for(int i = 0;i < nodeCount;i++){
        if(next == allNodes[i]){
            return i;
        }
    }
    return 0;
}


int main() {
    srand(time(NULL));
    clock_t start, end;
    double cpu_time_used;
    
    // Read VNF reliability from file
    readVNFs("VNFs.txt", vnfSet);
   
    initVNF(vnfSet);
    // Print VNF reliability
    printf("VNF Reliability:\n");
    for(int i = 0; i < VNF_COUNT; i++){
        printf("VNF%d : %f\n",i,vnfSet[i].reliability);
    }
    
    start = clock();

    // Read and process SFCs from file
    readSFCs("SFCs.txt", vnfSet);
    //SFCnoderelibility(vnfSet);
    findTopVNFs(vnfSet, TOP_VNF_COUNT);
    initSFG();
    int originNode = nodeCount;

    // Print node information before sharing
    printf("Before sharing:\n");
    printNodeInfo();
    

    // Print node information after sharing
    for(int i = 0;i < TOP_VNF_COUNT && TOP_VNF_COUNT < VNF_COUNT ;i++){
        printf("After %d node sharing:\n",i+1);
        updateNextPointers(vnfSet[i].id);
        printNodeInfo();
        printf("Node count is %d, save %d Node by instance sharing\n",nodeCount,originNode - nodeCount);
        if(originNode - nodeCount == 0){
            TOP_VNF_COUNT++;
        }
    }
    removeDuplicateNext();
    removeUnuseNode();
    printNodeInfo();

    int n = 2;
    int NFcnt[nodeCount];
    for(int i = 0;i < nodeCount;i++){
        if(allNodes[i]->head == 1){
            NFcnt[i] = 1;
        }
        else{
            NFcnt[i] = n;
            n++;
        }
        printf("%d\n",NFcnt[i]);
    }

    int nextIndex = -1;

    FILE *network_file = fopen("network.txt", "w");
    for (int i = 0; i < nodeCount; i++) {
        fprintf(network_file,"%d,%d,%d,",NFcnt[i],allNodes[i]->vnfID,allNodes[i]->head);
        for (int j = 0; j < MAX_NODES && allNodes[i]->next[j]; j++) {
            nextIndex = findnext(NFcnt,allNodes[i]->next[j]);
            fprintf(network_file,"%d", NFcnt[nextIndex]);
            if (allNodes[i]->next[j + 1] != NULL) {
                fprintf(network_file,",");
            }
        }
        fprintf(network_file,"\n");
    }
    
    fclose(network_file);
    generateAdjacencyMatrix();

    // Calculate reliability using Monte Carlo simulation
    int simulationCount = 10000000; // Adjust the simulation count as needed
    float reliability = calculateReliability(simulationCount);
    printf("Estimated reliability using Monte Carlo simulation: %f\n", reliability);

    Setcentrality();
    Backup_node(originNode - nodeCount);

    // Calculate reliability using Monte Carlo simulation
    float backup_reliability = calculateReliability(simulationCount);
    printf("Estimated backup reliability using Monte Carlo simulation: %f\n", backup_reliability);
    
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("CPU time used: %lf seconds\n", cpu_time_used);

    // Write CPU time to a CSV file
    FILE *csv_file = fopen("EISGA_cpu_time.csv", "a");
    if (csv_file != NULL) {
        //fprintf(csv_file, "%lf,%f\n", cpu_time_used,reliability);
        fprintf(csv_file, "%lf,%lf,%d,%lf\n", cpu_time_used,reliability,originNode - nodeCount,backup_reliability);
        fclose(csv_file);
    } else {
        printf("Error: Unable to open CSV file for writing.\n");
    }

    
    return 0;
}

