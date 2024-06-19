#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VNF_COUNT 5
#define SFC_COUNT 3
#define MAX_LENGTH 4

// Function to generate random reliability for VNFs
float generateRandomReliability() {
    return 0.99 + (rand() / (float)RAND_MAX) * (0.999 - 0.99);
}

// Function to generate random SFCs and write them to a file
void generateVNFs(const char *vnfFile) {
    FILE *vnfOutput = fopen(vnfFile, "w");

    if (vnfOutput == NULL) {
        printf("Error: Unable to open files for writing.\n");
        return;
    }

    // Generate and write VNF reliability
    srand(time(NULL));
    for (int i = 0; i < VNF_COUNT; i++) {
        float reliability = generateRandomReliability();
        fprintf(vnfOutput, "%d %f\n", i, reliability);
    }
    fclose(vnfOutput);
}

// Function to generate random SFCs and write them to a file
void generateSFCs(const char *sfcFile) {
    FILE *sfcOutput = fopen(sfcFile, "w");
    
    if (sfcOutput == NULL) {
        printf("Error: Unable to open files for writing.\n");
        return;
    }

    srand(time(NULL)); 

    // Generate and write SFCs
    for (int i = 0; i < SFC_COUNT; i++) {
        //int length = rand() % MAX_LENGTH + 1;
        int length = MAX_LENGTH;
        fprintf(sfcOutput, "%d ", length);

        int used[VNF_COUNT] = {0}; // Array to track used VNFs

        for (int j = 0; j < length; j++) {
            int vnf;
            do {
                vnf = rand() % VNF_COUNT; // Generate a random VNF
            } while (used[vnf]); // Check if the VNF has already been used

            used[vnf] = 1; // Mark the VNF as used
            fprintf(sfcOutput, "%d ", vnf);
        }
        fprintf(sfcOutput, "\n");
    }
    fclose(sfcOutput);
}



int main() {
    generateVNFs("VNFs.txt");
    generateSFCs("SFCs.txt");
    
    printf("SFCs and VNFs generated and written to files successfully.\n");
    return 0;
}
