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
    //printf("\nIn mesh topology, Processor %d has coordinates %d,%d", rank, coords[0], coords[1]);

int *numbers;

if(rank == 0){
    numbers = (int*)malloc(sizeof(int)*dim);
    for(int i = 0; i<dim; i++){
        //numbers[i] = drand48()*dim;
        numbers[i] = i;
    }
}

int local_max;
double start, end, sum_time, mean_time;
int max_down;
int max_right;

for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
//#############     INIZIO CALCOLO TEMPO     ###############
    start = MPI_Wtime();

    int *local_numbers; //= (int*)malloc(sizeof(int)*dim/size);

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
    local_numbers = (int*)malloc(sizeof(int)*size_local[rank]);

    MPI_Scatterv(numbers, size_local, displ,  MPI_INT, local_numbers, size_local[rank], MPI_INT , 0, mesh);

    //for(int i = 0; i < size_local[rank]; i++){
    //    printf("%d , ",local_numbers[i]);
    //}

    local_max = find_max(local_numbers, size_local[rank]);

    int x = ceil(log2(dims[0])+1);
    int up, down;

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
            //left, right = mesh.Shift(1, 2**(liv-1))
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