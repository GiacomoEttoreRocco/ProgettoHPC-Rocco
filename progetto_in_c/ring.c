#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<mpi.h> 
#include<time.h>

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
    int dim = atoi(argv[1]);
    double NUMBER_OF_REPS = atoi(argv[2]);

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

    int* numbers; 

    if(rank == 0){
        numbers = (int*)malloc(sizeof(int)*dim);
        for(int i = 0; i<dim; i++){
            numbers[i] = i; //drand48()*dim;
        }
    }

int local_max;
double start, end, mean_time;
int max_right;
int left, right;

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

int *local_numbers = (int*)malloc(sizeof(int)*dim/size);

//start = MPI_Wtime();
//for(int rep = 0; rep < NUMBER_OF_REPS; rep++){

    MPI_Scatterv(numbers, size_local, displ,  MPI_INT, local_numbers, size_local[rank], MPI_INT , 0, ring);
    local_max = find_max(local_numbers, dim/size);
    int x = ceil(log2(size)+1);

start = MPI_Wtime();

for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
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
                    local_max = max_right;
                    }
                }
            }
    }
    
    //end = MPI_Wtime();
}
end = MPI_Wtime();
//mean_time = (end-start)/NUMBER_OF_REPS;
mean_time = (end-start);

    if(rank == 0){
        printf("%d: %lf\n", size, mean_time);
    }
    
    MPI_Finalize();
}
