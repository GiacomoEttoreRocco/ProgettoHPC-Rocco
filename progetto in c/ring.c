#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<mpi.h> 
#include<time.h>

const int NUMBER_OF_REPS = 1;

int find_max(int* arr, int dim){
    int max = arr[0];
    for(int i = 1; i < dim; i++){
        if(max < arr[i]){
            max = arr[i];
        }
    }
    return max;
}

int main(int argc, char **argv){
    int dim  = atoi(argv[1]);

// 1) topologia ring:
    MPI_Init(&argc , &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD , &size);  
    int dims[1] = {0};
    MPI_Dims_create(size, 1, dims);
    MPI_Comm ring;
    int qperiods[1] = {1};
    MPI_Cart_create (MPI_COMM_WORLD, 1, dims, qperiods, 0, &ring);
    int rank;
    MPI_Comm_rank(ring, &rank);     
    int coords[1];
    MPI_Cart_coords(ring, rank , 1, coords); 
    //printf("\nIn ring topology, Processor %d has coordinates %d", rank, coords[0]);

    // left, right = ring.Shift(0, 1)

    int* numbers; 

    if(rank == 0){
        numbers = (int*)malloc(sizeof(int)*dim);
        for(int i = 0; i<dim; i++){
            numbers[i] = i; //drand48()*dim;
        }
    }

int local_max;
double start, end, sum_time, mean_time;
int max_right;

//t0 = time.time()
for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
//#############     INIZIO CALCOLO TEMPO     ###############
    start = MPI_Wtime();

    int *local_numbers = (int*)malloc(sizeof(int)*dim/size);
    // Compute displacement in case of n%size != 0
    //  ->  We're spreading the remainder to all processors until the remainder is gone
    int remainder = dim % size;
    int size_local[size], displ[size];
    int sum = 0;
    for (int i = 0; i < size; i++) {
        size_local[i] = dim / size;
        if (remainder > 0) {
            size_local[i]++;
            remainder--;
        }
        displ[i] = sum;
        sum += size_local[i];
    }
    MPI_Scatterv(numbers, size_local, displ,  MPI_INT, local_numbers, size_local[rank], MPI_INT , 0, ring);
    local_max = find_max(local_numbers, dim/size);
    //max_right;
    // all the odd numbers send to the left and right neighborhood

    int x = ceil(log2(size)+1);
    int left, right;
    for(int i =1; i<x; i++){
        MPI_Cart_shift(ring, 0, pow(2,(i-1)), &left, &right);
        int y = pow(2,i);
        if ((rank % y) != 0){
            MPI_Send(&local_max , 1, MPI_INT , left , rank , ring);
            break;
        }else{
            int k = pow(2, i-1);
            if(rank + k < size){
                MPI_Recv(&max_right, 1, MPI_INT, right, right, ring, MPI_STATUS_IGNORE);
                if (max_right > local_max){
                    //printf("%d", local_max);
                    local_max = max_right;
                    }
                }
            }
    }
    
    end = MPI_Wtime();
//#############     FINE CALCOLO TEMPO     ###############
    sum_time += end-start;
    printf("-");
}
mean_time = sum_time/NUMBER_OF_REPS;


    if(rank == 0){
        printf("\n%d",local_max);
        printf("\nTime: %lf", end-start);
    }
    
    MPI_Finalize();
}