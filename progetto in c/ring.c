#include<stdio.h>
#include<stdlib.h>
#include<math.h>
//#include<mpi.h> 
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
    int nmax = 100000000;

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
    MPI_Cart_coords(ring, rank , 2, coords); 
    printf("\nIn ring topology, Processor %d has coordinates %d", rank, coords[0]);

    // left, right = ring.Shift(0, 1)
    int numbers[16];

    if(rank == 0){
        //numbers = np.random.randint(nmax, size=dim)
        //DEBUG:
        //numbers = [0,1,2,3,4,5,6,7,8,9,10,11,12,13]
        //numbers = np.array_split(numbers, size)
        for(int i = 0; i<16; i++){
            numbers[i] = i;
        }  

    double start = MPI_Wtime();

    int *local_numbers = (int*)malloc(sizeof(int)*dim/size);
    MPI_Scatter(numbers, dim/size, MPI_INT, local_numbers, dim/size, MPI_INT , 0, ring);

    int local_max = find_max(local_numbers, dim/size);
    int max_right = 0;
    // all the odd numbers send to the left and right neighborhood

    int x = ceil(log2(size)+1);
    int left, right;
    for(int i =1; i<x; i++){
        MPI_Cart_shift(ring, 0, pow(2,(i-1)), &left, &right);
        //printf(" Sono il processore ", rank, "sx: ", left, "dx: ", right)
        int y = pow(2,i);
        if ((rank % y) != 0){
            //print("Processore", rank, ".   Invio a ", left, " e mi tolgo, livello: ", i)
            //ring.send(local_max, dest = left, tag = rank)   #   send to left local_max
            MPI_Send(&local_max , 1, MPI_INT , left , rank , ring);
            break;
        }else{
            int k = pow(2, i-1);
            if(rank + k < size){
                //print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
                //max_right = ring.recv(source = right, tag = right)     #   recive from any max_right
                MPI_Recv(&max_right, 1, MPI_INT, right, right, ring, MPI_STATUS_IGNORE);
                //print(" Sono il processore ", rank, "il mio destro e' ", right, " messaggio ricevuto.  livello: ", i) 
                if (max_right > local_max){
                    local_max = max_right;
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