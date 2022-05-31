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

// 2) topologia mesh: 
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
//    printf("%d, ", local_numbers[i]);
//}
//------------------------------------------------

int max_down = 0;

//coords = torus.Get_coords(rank)
int up, down;
for(int i=1; i<ceil(log2(dims[1])+1); i++){
    //up, down = torus.Shift(1, 2**(i-1))
    MPI_Cart_shift(torus , 0, pow(2,(i-1)), &up, &down);
    //print(" Sono il processore", rank, ": ", torus.Get_coords(rank), "up: ", up, ": ", torus.Get_coords(up), "down: ", down, ": ", torus.Get_coords(down), "livello: ", i)
    if(coords[1] %(int) (pow(2,i)) != 0){
        //print("Processore", rank, ".   Invio a ", up, " e mi tolgo, livello: ", i)
        //torus.send(local_max, dest = up, tag = rank)   #   send to left local_max
        MPI_Send(&local_max, 1, MPI_INT , up , rank , torus);
        break;
    }
    else{
        //if((down != 0) or (rank == 0)):  
        //if(rank + 2**(i-1) < grid2d[0]):
        if(coords[1] + (int)(pow(2,(i-1)) < dims[1])){
            //print("Processore ", rank," riceverei da down: ", down, " il mio up: ", up, ", livello: ", i)
            //max_down = torus.recv(source = down, tag = down)     //   recive from any max_down
            MPI_Recv(&max_down, 1, MPI_INT, down, down, torus, MPI_STATUS_IGNORE);
            //print(" Sono il processore ", rank, "il mio down e' ", mesh.Get_coords(down), " messaggio ricevuto.  livello: ", i) 
            if(max_down > local_max){
                local_max = max_down;
            }
        }
    }       
}
    
MPI_Barrier(torus);

//int max_right = 0 # <- debug
int left, right;
for(int liv=1; liv < dims[2]; liv++){
    //left, right = torus.Shift(2, 2**(liv-1))
    MPI_Cart_shift(torus , 0, pow(2,(liv-1)), &left, &right);
    //print(" Sono il processore", rank, ": ", torus.Get_coords(rank), "left: ", left, ": ", torus.Get_coords(left), "right: ", right, ": ", torus.Get_coords(right), "livello: ", i)
    if(rank % (int)pow(2,liv)) != 0:
        #print("sending, local_max:",local_max, ",          from.. ",rank, coords, "to..", left, torus.Get_coords(left))
        torus.send(local_max, dest = left, tag = rank) 
        break

    else:
        #if((down != 0) or (rank == 0)):  
        #if(rank + 2**(i-1) < grid2d[0]):
        if(coords[2] + 2**(liv-1) < grid3d[2]):
            #print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            max_right = torus.recv(source = right, tag = right)     #   recive from any max_down
            #print("Sono il processore ", rank, "il mio right e' ", torus.Get_coords(right), " messaggio ricevuto. Livello: ", liv) 
            if max_right > local_max:
                local_max = max_right
}
    

torus.Barrier()

if (coords[1] == 0) & (coords[2] == 0):
    for liv in range(1, grid3d[0]):
        shallow, deep = torus.Shift(0, 2**(liv-1))
        #print(" Sono il processore", rank, ": ", torus.Get_coords(rank), "shallow: ", shallow, ": ", torus.Get_coords(shallow), "deep: ", deep, ": ", torus.Get_coords(deep), "livello: ", i)
        if coords[0] % (2**liv) != 0:
            #print("sending, local_max:",local_max, ",          from.. ",rank, coords, "to..", shallow, torus.Get_coords(shallow))
            torus.send(local_max, dest = shallow, tag = rank) 
            break

        else:
            #if((down != 0) or (rank == 0)):  
            #if(rank + 2**(i-1) < grid2d[0]):
            if(coords[0] + 2**(liv-1) < grid3d[0]):
                #print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
                max_deep = torus.recv(source = deep, tag = deep)     #   recive from any max_down
                #print("Sono il processore ", rank, "il mio right e' ", torus.Get_coords(right), " messaggio ricevuto. Livello: ", liv) 
                if max_deep > local_max:
                    local_max = max_deep




double end = MPI_Wtime();
    if(rank == 0){
        printf("\n%d",local_max);
        printf("\nTime: %lf", end-start);
    }
    
    MPI_Finalize();
}
