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
    int *all_maxs; 

    if(rank == 0){
        numbers = (int*)malloc(sizeof(int)*dim);
        for(int i = 0; i<dim; i++){
            numbers[i] = i; //drand48()*dim;
        }
        all_maxs = (int*)malloc(sizeof(int)*size);
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
int max;

//start = MPI_Wtime();
//for(int rep = 0; rep < NUMBER_OF_REPS; rep++){

    MPI_Scatterv(numbers, size_local, displ,  MPI_INT, local_numbers, size_local[rank], MPI_INT , 0, ring);
    local_max = find_max(local_numbers, dim/size);
    //printf("rank %d max: %d\n", rank, local_max);

start = MPI_Wtime();
for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
    MPI_Gather(&local_max, 1, MPI_INT, all_maxs, 1, MPI_INT, 0, ring);

    if(rank == 0){
        max = find_max(all_maxs, size);
    }
    //printf("here ..."); 
    //end = MPI_Wtime();
    
    }

end = MPI_Wtime();
//mean_time = (end-start)/NUMBER_OF_REPS;
//mean_time = (end-start)/(NUMBER_OF_REPS/1000);
mean_time = ((end-start)*1000)/NUMBER_OF_REPS;
    if(rank == 0){
        printf("%d: %lf in ms\n", size, mean_time);
    }
    
    MPI_Finalize();

}
