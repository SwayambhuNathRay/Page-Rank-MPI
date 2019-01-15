#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "mpi.h"

int main()
{
	int ierr, rank, size;
            double time_trivial_start, time_trivial_end, time_algo_start, time_algo_end;
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Offset filesize;
	MPI_File fh;
	MPI_Barrier(MPI_COMM_WORLD);

	/* Each processor is reading the entire file and populating its own part of the adjacency matrix. Each processor
	has its own part of the adjacency matrix, like an 1-D mapping of the adjacency matrix.*/
            
	ierr = MPI_File_open (MPI_COMM_WORLD, "input.txt" ,MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
	MPI_File_get_size(fh, &filesize);
	char *array_own = (char*) malloc(filesize * sizeof(char));
	MPI_File_read_all_begin(fh, array_own, filesize, MPI_CHAR); //populating an array from the input file
	int i,j;
	MPI_File_close(&fh);
	int num_of_row = 50000/size;
	int num_of_column = 143;
	int **arr = (int **)malloc(num_of_row * sizeof(int *)); //initializing the adjacency matrix for each processor
    	for (i=0; i<num_of_row; i++)
         		arr[i] = (int *)malloc(num_of_column * sizeof(int));
         	for(i = 0; i < num_of_row; i++)
         		for(j = 0; j < num_of_column; j++)
         			arr[i][j] = 0;
         	static float array[50000];
         	int lower_node = rank*(50000/size);
         	int upper_node = (rank+1)*(50000/size); //defining the number of nodes for each processsor
         	float *tentative_page_rank = (float*) malloc(50000*sizeof(float));
         	float *weights_passed = (float*) malloc(50000*sizeof(float));
         	float *receive_weights = (float*) malloc(50000*sizeof(float));
         	memcpy(weights_passed, array, 50000*sizeof(float));
         	memcpy(tentative_page_rank, array, 50000*sizeof(float));
         	memcpy(receive_weights, array, 50000*sizeof(float));
         	if(rank == size - 1)
         		upper_node = 50000;
                  
         	int k = 0, l = 0, source_prev = -1, source, sink;
         	for(j = 0; j < num_of_row; j++)
         	{
         		arr[j][0] = lower_node+j;
         	}
         	
         	for(j = 0; j < filesize; j++) //reading from the array and forming the adjacency list
         	{
         		if(array_own[j] == 'a')
         		{
         			j+=2;
         			char temp[5];
         			int z;
         			for(z = 0; z < 5; z++)
         				temp[z] = 'k';
         			i = 0;
         			while(array_own[j]!=' ')
         			{
         				temp[i] = array_own[j];
         				i++;
         				j++;
         			}
         			source = atoi(temp);
         			tentative_page_rank[source] = (1.0/33000.0);
         			if(source >= lower_node && source < upper_node)
         			{
         				if(source_prev!=source)
         				{
         					k = source%num_of_row;
         					l = 1;
         				}
         			}
         			else
         			{
         				j++;
         				char keep[5];
         				for(z = 0; z < 5; z++)
         					keep[z] = 'k';
         				i = 0;
         				while(array_own[j]!=' ')
         				{
         					keep[i] = array_own[j];
         					i++;
         					j++;
         				}
         				sink = atoi(keep);
         				tentative_page_rank[sink] = (1.0/33000.0);
         				continue;
         			}
         			source_prev = source;
         			j++;
         			char keep[5];
         			for(z = 0; z < 5; z++)
         				keep[z] = 'k';
         			i = 0;
         			while(array_own[j]!=' ')
         			{
         				keep[i] = array_own[j];
         				i++;
         				j++;
         			}
         			sink = atoi(keep);
         			arr[k][l] = sink;
         			l++;
         		}
         	}


         	int num_of_nodes = num_of_row;
         	
         	int *store_connections = (int*) malloc(num_of_nodes*sizeof(int)); //array for storing the number of connections for a particular node
         	
         	int connections, weights;
         	MPI_Barrier(MPI_COMM_WORLD);
         	if(rank == 0)
                        time_algo_start = MPI_Wtime();
         	for(k = 0 ; k < num_of_nodes; k++)
         	{
         		connections = 0;
         		weights = 0;
         		for(l = 1; l < num_of_column; l++)
         		{
         			if(arr[k][l]!=0)
         				connections++;
         			else
         				break;
         		} 
         		store_connections[k] = connections;
         		float weight_to_be_sent = 0; 
         		/*for the first iteration the weights to be sent to other processors are calculated and stored in an array*/
         		if(connections!=0)
         		{
         			weight_to_be_sent = (float)tentative_page_rank[arr[k][0]]/(float)connections;
         			for(l = 1; l < num_of_column; l++)
         			{
         			if(tentative_page_rank[arr[k][0]]!=0)
         				weights_passed[arr[k][l]] = weight_to_be_sent + weights_passed[arr[k][l]];
         			else
         				break;
         			}
         		}
         		
         	}
         	MPI_Allreduce(weights_passed, receive_weights, 50000, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

         	/*An allreduce is performed summing over all the weights received by a particular node*/

         	for(k=0; k < num_of_nodes; k++) //calculating the tentative page rank after the first iteration
         	{
         		if(tentative_page_rank[arr[k][0]]!=0.0)
         		{
         			tentative_page_rank[arr[k][0]] = 0.15*(1.0/33000.0) + 0.85*(receive_weights[arr[k][0]]);
         		}
         	}
         	
         	
         	for(k=0; k < 28; k++) //doing the rest of the iterations
         	{
         	memcpy(weights_passed, array, 50000*sizeof(float));
         	memcpy(receive_weights, array, 50000*sizeof(float));
         		for(l = 0; l < num_of_nodes; l++)
         		{
         			float weight_to_be_sent = 0;
         			if(store_connections[l]!=0) 
         			{
         				weight_to_be_sent = (float)tentative_page_rank[arr[l][0]]/(float)store_connections[l];
         				for(j = 1; j < num_of_column; j++)
         				{
         				if(tentative_page_rank[arr[l][0]]!=0)
         					weights_passed[arr[l][j]] = weight_to_be_sent + weights_passed[arr[l][j]];
         				else
         					break;
         				}
         			}
         		}

         		/*the weight array is produced which holds the information of the amount of weights each node has to
         		send to the other nodes in its neighbourhood
		
		After that an all reduce is performed to sum up all the weights received for each node and then the
		tentative page ranks are calculated for each node.

         		*/

         		MPI_Allreduce(weights_passed, receive_weights, 50000, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
         		for(i=0; i < num_of_nodes; i++)
         		{
         			if(tentative_page_rank[arr[i][0]]!=0)
         			{
         				tentative_page_rank[arr[i][0]] = 0.15*(1.0/33000.0) + 0.85*(receive_weights[arr[i][0]]);
         			}
         		}
         	}

         	
         	MPI_Barrier(MPI_COMM_WORLD);
         	if(rank == 0)
             {
                           time_algo_end = MPI_Wtime();
                           printf("Time required  is %f seconds\n", time_algo_end - time_algo_start); //printing the time required
             }
         	for(j = 0; j < 50000; j++)
         	{
         		if(j < lower_node || j >= upper_node) 
         			tentative_page_rank[j] = 0;
         	}
         	MPI_Barrier(MPI_COMM_WORLD);
         	float *final_weights = (float*) malloc(50000*sizeof(float)); // array for final weights of each node
         	MPI_Reduce(tentative_page_rank, final_weights, 50000, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
         	/*at the end each processor has the entire tentative page rank array, from which the final rank of each node can 
         	be derived*/
	MPI_Finalize();
	return 0;
}