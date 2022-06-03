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

    MPI_Init(&argc , &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD , &size);  
    int dims[3] = {0,0,0};
    MPI_Dims_create(size, 3, dims);
    MPI_Comm torus;
    int qperiods[3] = {1, 1, 1};
    MPI_Cart_create (MPI_COMM_WORLD, 3, dims, qperiods, 0, &torus);
    int rank;
    MPI_Comm_rank(torus, &rank);  
    int coords[3];
    MPI_Cart_coords(torus, rank , 3, coords); 
    //printf("\nIn torus topology, Processor %d has coordinates %d,%d,%d", rank, coords[0], coords[1], coords[2]);

int* numbers;

if(rank == 0){
    numbers = (int*)malloc(sizeof(int)*dim);
    for(int i = 0; i< dim; i++){
        numbers[i] = i; //drand48()*dim;
    }
}

//t0 = time.time()
double start = MPI_Wtime();

int *local_numbers; // = (int*)malloc(sizeof(int)*);
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

MPI_Scatterv(numbers, size_local, displ, MPI_INT, local_numbers, size_local[rank], MPI_INT , 0, torus);

int local_max = find_max(local_numbers, size_local[rank]);

//for(int i = 0; i< size_local[rank]; i++){
//    printf("%d", local_numbers[i]);
//}

//printf("Processore %d, (%d,%d,%d) \n", rank, coords[0], coords[1], coords[2]);

int up, down, max_down;

int x1 = ceil(log2(dims[0])+1);

for(int i =1; i<x1; i++){
    MPI_Cart_shift(torus, 0, pow(2,(i-1)), &up, &down);
    int y = pow(2,i);
    if ((coords[0]% y) != 0){
        printf("FASE UP-DOWN:  Processore %d, invio %d a %d, e mi tolgo, livello: %d\n", rank, local_max, up, i);
        MPI_Send(&local_max , 1, MPI_INT , up , rank , torus);
        break;
    }else{
        int k = pow(2, i-1);
        if((coords[0] + k) < dims[0]){
            //print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            //max_right = ring.recv(source = right, tag = right)     #   recive from any max_right
            MPI_Recv(&max_down, 1, MPI_INT, down, down, torus, MPI_STATUS_IGNORE);
            //printf("Processore %d, ricevuto %d", rank, max_down);
            if (max_down > local_max){
                local_max = max_down;
                }
            }
        }
}

//if(rank == 0 || rank == 1){
//    printf("LOCAL MAX: %d\n", local_max);
//}

MPI_Barrier(torus);
//----------------------------------------------------

int x = ceil(log2(dims[1])+1);

int left, right, max_right;

if(rank < dims[1]*dims[2]){
    for(int i=1; i < x; i++){
        MPI_Cart_shift(torus , 1, pow(2,(i-1)), &left, &right);
        int y = pow(2,i);
        if (coords[1] % y != 0){
            printf("FASE LEFT-RIGHT:  Processore %d, invio %d a %d, e mi tolgo, livello: %d\n", rank, local_max, left, i);
            MPI_Send(&local_max , 1, MPI_INT , left , rank , torus);
            break;
        }else{
            int k = pow(2, i-1);
            if(coords[1] + k < dims[1]){
                MPI_Recv(&max_right, 1, MPI_INT, right, right, torus, MPI_STATUS_IGNORE);
                if(max_right > local_max){
                    local_max = max_right;
                }
            }
        }
    }
}

MPI_Barrier(torus);

int shallow, deep, max_deep;

if(rank < dims[2]){
    for(int liv = 1; liv < dims[2]; liv++){
        MPI_Cart_shift(torus , 2, pow(2,(liv-1)), &shallow, &deep);
        int temp = pow(2,liv);
        if(rank % temp != 0){
            printf("FASE SHALLOW-DEEP:  Processore %d, invio %d a %d, e mi tolgo, livello: %d\n", rank, local_max, shallow, deep);
            MPI_Send(&local_max , 1, MPI_INT , shallow, rank , torus);
            break;
        }
        else{
            int p = pow(2,(liv-1));
            if(rank + p < dims[2]){
                MPI_Recv(&max_deep, 1, MPI_INT, deep, deep, torus, MPI_STATUS_IGNORE);
                if(max_deep > local_max){
                    local_max = max_deep;
                }
            }     
        }
    }
}

double end = MPI_Wtime();
    if(rank == 0){
        printf("\n%d",local_max);
        printf("\nTime: %lf", end-start);
    }
    
    MPI_Finalize();
}
