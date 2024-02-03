#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv)
{
	int my_rank; // variable to store the rank of the current process
	int component; // variable for a specific element in the array
	int a,amount,num_proc,num; // variables for number of processes, total number of integers, number of integers for each process, and number of processes
	int tag1=50,tag2=60,tag3=70,tag4=80; // variables for tags used in MPI_Send and MPI_Recv functions
	int target,source; // variables for target and source processes
	//tag1 -> send,recv amount
	//tag2 -> send,recv elements to/from process/es
	//tag3 -> send,recv status of series and defective element
	//tag4 -> send,recv component
	
	MPI_Status status; // variable for status of MPI_Recv function
	int x; // variable for loop index
	int data[200]; // array to store all integers
    int data_loc[200]; // array to store the portion of integers for each process
    int sorted,edge; // variable for if the array is sorted, and variable for the last element of the previous process

	//the component that breaks the assention

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // find the rank of the current process
    MPI_Comm_size(MPI_COMM_WORLD, &a); // find the total number of processes

	if (my_rank == 0) // master process
	{
		printf("The amount of numbers:\n"); // input total number of integers
		scanf("%d", &amount);

		printf("The amount of %d numbers:\n",amount); // input integers
		for (x=0; x<amount; x++)
			scanf("%d", &data[x]);
        for (target = 1; target < a; target++)
            MPI_Send(&amount, 1, MPI_INT, target, tag1, MPI_COMM_WORLD); // send total number of integers to all other processes
       
	    num_proc = amount/a;  x=num_proc; // calculate number of integers for each process
        
        for (target = 1; target < a; target++)
        {
            MPI_Send(&data[x], num_proc, MPI_INT, target, tag2, MPI_COMM_WORLD);  // send portion of integers to each process
            x+=num_proc;
        }
        for (x=0; x<num; x++)
            data_loc[x]=data[x]; // store portion of integers for master process
	}
	 
	 else // other processes
    
	{
        MPI_Recv(&amount, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status); // receive total number of integers
        num_proc = amount/a;  // calculate number of integers for each process
        MPI_Recv(&data_loc[0], num_proc, MPI_INT, 0, tag2, MPI_COMM_WORLD, &status); // receive portion of integers for this process
        MPI_Send(&data_loc[0],1,MPI_INT,my_rank-1,tag4,MPI_COMM_WORLD); // send the last element of this process's portion to the previous process
		
		if (my_rank<a-1)
		{
			MPI_Recv(&edge,1, MPI_INT, my_rank+1, tag4, MPI_COMM_WORLD, &status); // receive the last element of the next process's portion
			
		}
		  
    }
    sorted=1;
	for (x=0 ; x<num_proc-1 ; x++)
	{
		if(data_loc[x] > data_loc[x+1] ) // check if the portion of the array is sorted
		{
			sorted=0; //here is the component=data_loc[x];
		}
		
		
		if((x+1==num_proc-1) && data_loc[x+1]>edge && my_rank!=a-1) // check if the last element of this process's portion is greater than the last element of the previous process's portion
		{
			sorted=0; //here is the component=data_loc[x+1];
		}
	
    }
	
	if (my_rank != 0) 
        MPI_Send(&sorted, 1, MPI_INT, 0, tag3, MPI_COMM_WORLD); // send a message to the master process indicating if the portion of the array is sorted
	
	else 
	{
		if (sorted==0)
		{
			printf("\n\n\n(NO) the assending order dissolved during the process: %d \nDefective element : %d\n" , my_rank,component);
			
		}
		// If the current process's portion of the array is not sorted, print a message indicating the process number and the defective element
		for (source=1 ; source<a ; source++) 
		{
			MPI_Recv(&sorted,1,MPI_INT,source,tag3,MPI_COMM_WORLD,&status); // receive message from other processes indicating if their portion of the array is sorted
			
			if ( sorted == 0 )
			{
				printf("\n\n\n(NO)  the assending order dissolved during the process: %d \nDefective element : %d\n" , source,component);
				// If any other process's portion of the array is not sorted, print a message indicating the process number and the defective element
			}
				
		}	
	}

    MPI_Finalize();
	
	return 0;
}
