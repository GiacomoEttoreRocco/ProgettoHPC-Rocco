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
    int dim  = atoi(argv[1]);
    double NUMBER_OF_REPS = atoi(argv[2]);
// 2) topologia mesh: 
    MPI_Init(&argc , &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD , &size);  
    int dims[2] = {0,0};
    MPI_Dims_create(size, 2, dims);
    MPI_Comm mesh;
    int qperiods[2] = {1, 1};
    MPI_Cart_create (MPI_COMM_WORLD, 2, dims, qperiods, 0, &mesh);
    int rank;
    MPI_Comm_rank(mesh, &rank);  
    int coords[2];
    MPI_Cart_coords(mesh, rank , 2, coords); 

int *numbers;

if(rank == 0){
    numbers = (int*)malloc(sizeof(int)*dim);
    for(int i = 0; i<dim; i++){
        numbers[i] = i;
    }
}

int local_max;
double start, end, mean_time;
int max_down;
int max_right;
int up, down;

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

int *local_numbers = (int*)malloc(sizeof(int)*size_local[rank]);

//start = MPI_Wtime();


//for(int rep = 0; rep < NUMBER_OF_REPS; rep++){

    MPI_Scatterv(numbers, size_local, displ,  MPI_INT, local_numbers, size_local[rank], MPI_INT , 0, mesh);
    local_max = find_max(local_numbers, size_local[rank]);  //esecuzione

    int x = ceil(log2(dims[0])+1);

start = MPI_Wtime();

for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
    for(int i=1; i < x; i++){
        MPI_Cart_shift(mesh , 0, pow(2,(i-1)), &up, &down);
        int y = pow(2,i);
        if (coords[0] % y != 0){
            MPI_Send(&local_max , 1, MPI_INT , up , rank , mesh);
            break;
        }else{
            int k = pow(2, i-1);
            if(coords[0] + k < dims[0]){
                MPI_Recv(&max_down, 1, MPI_INT, down, down, mesh, MPI_STATUS_IGNORE);
                if(max_down > local_max){
                    local_max = max_down;
                }
            }
        }
    }

    MPI_Barrier(mesh);
    int left, right;
    if(rank < dims[1]){
        for(int liv = 1; liv < dims[1]; liv++){
            MPI_Cart_shift(mesh , 1, pow(2,(liv-1)), &left, &right);
            int temp = pow(2,liv);
            if(rank % temp != 0){
                MPI_Send(&local_max , 1, MPI_INT , left , rank , mesh);
                break;
            }
            else{
                int p = pow(2,(liv-1));
                if(rank + p < dims[1]){
                    MPI_Recv(&max_right, 1, MPI_INT, right, right, mesh, MPI_STATUS_IGNORE);
                    if(max_right > local_max){
                        local_max = max_right;
                    }
                }     
            }
        }
    }

    //end = MPI_Wtime();
}

end = MPI_Wtime();

//mean_time = (end-start)/NUMBER_OF_REPS;

mean_time = (end-start)/(NUMBER_OF_REPS/1000);

if(rank == 0){
    printf("%d: %lf \n", size, mean_time);
}
    
MPI_Finalize();
}
