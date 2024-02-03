#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char* argv[])
{
    int choice; // variable to store user's menu choice
    int vector_size; // variable to store the size of the vector
    int rank, size; // variables to store the rank and size of the current process
    float* vector; // pointer to the vector
    float number; // variable to store a single number from the vector
    float sum = 0; // variable to store the sum of numbers in the vector
    float average;  // variable to store the average of numbers in the vector
    int higher = 0, lower = 0, equal = 0; // variables to store the count of numbers higher, lower, and equal to the average
    int total_higher, total_lower, total_equal; // variables to store the total count of numbers higher, lower, and equal to the average
    float total_sum, total_dispersion;// variables to store the total sum and total dispersion


 // Initialize MPI and get the rank and size of the current process
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    do {
    	 // If the current process is rank 0, display the menu and get the user's choice
        if(rank == 0) {
            printf("Menu:\n");
            printf("1. Run program\n");
            printf("2. Exit\n");
            printf("Enter choice: ");
            scanf("%d", &choice);
        }

        MPI_Bcast(&choice, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if(choice == 1) 
        {
        	// If the current process is rank 0, get the size of the vector and the numbers in the vector
            if(rank == 0) 
            {
                printf("Enter the size of the vector: ");
                scanf("%d", &vector_size);

                // Allocate memory for the vector
                vector = (float*)malloc(vector_size * sizeof(float));

                for(int i = 0; i < vector_size; i++) 
                {
                    printf("Enter the numbers %d: ", i+1);
                    scanf("%f", &vector[i]);
                }
            }

             // Broadcast the size of the vector to all processes
            MPI_Bcast(&vector_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // If the current process is not rank 0, allocate memory for the vector
            if(rank != 0) 
            {
                vector = (float*)malloc(vector_size * sizeof(float));
            }

            // Broadcast the vector to all processes
            MPI_Bcast(vector, vector_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
           
            // Initialize the sum, higher, lower and equal to 0
            sum = 0;
            higher = 0;
            lower = 0;
            equal = 0;

             // Calculate the start and end indices for the current process
            int start = rank * (vector_size / size);
            int end = start + (vector_size / size);
            
             // Calculate the sum for the current process
            for(int i = start; i < end; i++) 
            {
                sum += vector[i];
            }
            
             // If the current process is rank 0, receive the sum from other processes and calculate the total sum
            if(rank == 0) 
            {
                total_sum = sum;
                for(int i = 1; i < size; i++) 
                {
                    float recv_sum;
                    MPI_Recv(&recv_sum, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    total_sum += recv_sum;
                }

                // Calculate the average
                average = total_sum/vector_size;
            } else 
            
            {
            	 // Send the sum from the current process to rank 0
                MPI_Send(&sum, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
            }
            
             // Broadcast the average to all processes
            MPI_Bcast(&average, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
            
            
             float dispersion = 0;
            for(int i = start; i < end; i++) 
            {
                if(vector[i] > average) 
                {
                    higher++;
                } else if(vector[i] < average) 
                
                {
                    lower++;
                } else if(vector[i] == average) 
                
                {
                    equal++;
                }

                //Calculate the dispersion for the current process
                dispersion += pow(vector[i]-average,2);
            }
            //Divide the dispersion by the size of the vector
            dispersion = dispersion/(vector_size);
            
              
            //Send the dispersion, higher, lower and equal count for the current process to rank 0
            MPI_Send(&dispersion, 1, MPI_FLOAT, 0, 4, MPI_COMM_WORLD);
            MPI_Send(&higher, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&lower, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(&equal, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
            if(rank == 0) 
            {
            	// Initialize the total dispersion, total higher, total lower and total equal to 0
                total_dispersion = 0;
                total_higher = 0;
                total_lower = 0;
                total_equal = 0;

                 // Receive the dispersion, number of elements higher than average, lower than average, and equal to average from all other processes
                for(int i = 0; i < size; i++) 
                {
                    float recv_disp;
                    int recv_higher, recv_lower, recv_equal;

                    MPI_Recv(&recv_disp, 1, MPI_FLOAT, i, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(&recv_higher, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(&recv_lower, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(&recv_equal, 1, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    // Sum the values to get the total values
                    total_dispersion += recv_disp;
                    total_higher += recv_higher;
                    total_lower += recv_lower;
                    total_equal += recv_equal;
                }
                
                // Print the results
                printf("The average value of the vector X is: %f\n", average);
                printf("The total dispersion of the vector X is: %f\n", total_dispersion);
                printf("Numbers whith higher values than average: %d\n", total_higher);
                printf("Number of values lower than average: %d\n", total_lower);
                printf("Number of values equal to average: %d\n", total_equal);

                free(vector);
            }
        }
        
    } while(choice != 2);
    

    MPI_Finalize();

    return 0;
}
