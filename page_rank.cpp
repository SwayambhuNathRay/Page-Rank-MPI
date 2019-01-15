
#include <iostream>
#include <fstream>
#include <cstddef>
#include <string>
#include <sys/time.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
using namespace std;



int main()
{
    int num_of_row = 50000;
    int num_of_column = 150;
    int **arr = (int **)malloc(num_of_row * sizeof(int *));//defining the adjaceccy list to hold the nodes

    for (int i=0; i<num_of_row; i++)
        arr[i] = (int *)malloc(num_of_column * sizeof(int));
    for(int i = 0; i < num_of_row; i++)
        for(int j = 0; j < num_of_column; j++)
         			arr[i][j] = 0;
    for(int i = 0; i < 50000; i++)
        arr[i][0] = i;
    float *tentative_page_rank = (float*) malloc (50000*sizeof(float)); //array for storing tentative page ranks for each node
    for(int i = 0; i < 50000; i++)
        tentative_page_rank[i] = 0; // initializing the tentative page rank array for all the nodes
    ifstream inFile;
    inFile.open("input.txt"); //openning file to read from file
    int k = 0, source_prev = -1, source, l = 1, sink;
    
    
    /*the informations are read from a text file and used to populate an adjacency matrix*/
    
    if(inFile.is_open()) //populating the adjacency matrix
    {
        string line;
        while (! inFile.eof() )
        {
            getline(inFile,line);
            for(int j = 0; j < line.size(); j++)
            {
                if(line[j] == 'a')
                {
                    j+=2;
                    char temp[5];
                    for(int z = 0; z < 5; z++)
                        temp[z] = 'k';
                    int i = 0;
                    while(line[j]!=' ')
                    {
                        temp[i] = line[j];
                        i++;
                        j++;
                    }
                    source = atoi(temp);
                    tentative_page_rank[source] = (1.0/33000.0);
                    if(source_prev!=source)
                    {
                        l=1;
                    }
                    j++;
                    char keep[5];
                    for(int z = 0; z < 5; z++)
                        keep[z] = 'k';
                    i = 0;
                    while(line[j]!=' ')
                    {
                        keep[i] = line[j];
                        i++;
                        j++;
                    }
                    sink = atoi(keep);
                    tentative_page_rank[sink] = (1.0/33000.0);
                    arr[source][l] = sink;
                    l++;
                    source_prev = source;
                }
            }
        }
        inFile.close();
    }
    struct timeval start_serial,end_serial;
    gettimeofday(&start_serial,NULL);//starting counter
    int connections = 0;
    
    
    /*starting the iterations as described in the paper, for each iterations sending required weights to the required nodes and calculating the final weights of each node*/
    
    for(k = 0; k < 29; k++) //30 iterations for passing weights
    {
        float* weight_to_be_added = (float*) malloc(50000*sizeof(float));
        for(int i = 0; i < 50000; i++)
            weight_to_be_added[i] = 0;
        int *store_connections = (int*) malloc(50000*sizeof(int));
        for(int i = 0; i < 50000; i++) //calculating connections for each node
        {
            for(int j = 1; j < 150; j++)
            {
                if(arr[i][j] != 0)
                {
                    connections++;
                }
            }
            store_connections[arr[i][0]] = connections;
            connections = 0;
        }
        for(int i = 0; i < 50000; i++)
        {
            for(int j = 1; j < 150; j++)
            {
                if(arr[i][j]!=0)
                {
                    weight_to_be_added[arr[i][j]] = weight_to_be_added[arr[i][j]] + (float)tentative_page_rank[i]/(float)store_connections[i]; // calculating weights to be given to other nodes in each iterations
                }
            }
        }
        
        for(int i = 0; i < 50000; i++)
        {
            if(tentative_page_rank[i]!=0)
            {
                tentative_page_rank[i] = 0.15*(1.0/33000.0) + 0.85*(weight_to_be_added[i]); //calculating the resulatant weights for each node
            }
        }
        delete weight_to_be_added;
    }
    
    gettimeofday(&end_serial,NULL);
        printf("\nTime taken for calculation is %ld microseconds \n",(end_serial.tv_sec - start_serial.tv_sec)*1000000 + (end_serial.tv_usec - start_serial.tv_usec)); //printing time required
    return 0;
}