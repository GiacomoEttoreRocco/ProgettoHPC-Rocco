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

// 2) topologia mesh: 
    MPI_Init(&argc , &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD , &size);  
    int dims[3] = {0,0,0};
    MPI_Dims_create(size, 3, dims);
    MPI_Comm torus;
    int qperiods[2] = {1, 1, 1};
    MPI_Cart_create (MPI_COMM_WORLD, 2, dims, qperiods, 0, &torus);
    int rank;
    MPI_Comm_rank(torus, &rank);  
    int coords[2];
    MPI_Cart_coords(torus, rank , 2, coords); 
    printf("\nIn mesh topology, Processor %d has coordinates %d,%d,%d", rank, coords[0], coords[1], coords[2]);

int numbers[16];

if(rank == 0){
    //int *numbers = (int*)malloc(sizeof(int)*dim) // np.random.randint(nmax, size=dim)
    //DEBUG:
    for(int i = 0; i<16; i++){
        numbers[i] = i;
    }
    //---------------------
    //numbers = np.array_split(numbers, size)
}

//t0 = time.time()
double start = MPI_Wtime();

int *local_numbers = (int*)malloc(sizeof(int)*dim/size);
MPI_Scatter(numbers, dim/size, MPI_INT, local_numbers, dim/size, MPI_INT , 0, torus);

int local_max = find_max(local_numbers, dim/size);

int max_down = 0;
int max_right = 0;
int max_deep = 0;


//coords = mesh.Get_coords(rank)
int x = ceil(log2(dims[0])+1);
int up, down;

for(int i=1; i < x; i++){
    MPI_Cart_shift(torus, 1, pow(2,(i-1)), &up, &down);
    //up, down = mesh.Shift(0, 2**(i-1))
    // print(" Sono il processore", rank, ": ", mesh.Get_coords(rank), "up: ", up, ": ",mesh.Get_coords(up), "down: ", down, ": ", mesh.Get_coords(down), "livello: ", i)
    int y = pow(2,i);
    if (coords[0] % y != 0){
        // print("Processore", rank, ".   Invio a ", up, " e mi tolgo, livello: ", i)
        MPI_Send(&local_max , 1, MPI_INT , up , rank , torus);
        break;
    }else{
        int k = pow(2, i-1);
        if(coords[0] + k < dims[0]){
            //print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            MPI_Recv(&max_down, 1, MPI_INT, down, down, torus, MPI_STATUS_IGNORE);
            //max_down = mesh.recv(source = down, tag = down)     #   recive from any max_down
            //print(" Sono il processore ", rank, "il mio down e' ", mesh.Get_coords(down), " messaggio ricevuto.  livello: ", i) 
            if(max_down > local_max){
                local_max = max_down;
            }
        }
    }
}

MPI_Barrier(torus);
int left, right;

for(int liv = 1; liv < dims[1]; liv++){
    //left, right = mesh.Shift(1, 2**(liv-1))
    MPI_Cart_shift(torus, 2, pow(2,(liv-1)), &left, &right);
    int temp = pow(2,liv);
    if((rank % temp) != 0){
        //mesh.send(local_max, dest = left, tag = rank) 
        MPI_Send(&local_max , 1, MPI_INT , left , rank , torus);
        break;
    }
    else{
        int p = pow(2,(liv-1));
        if(rank + p < dims[1]){
            //max_right = mesh.recv(source = right, tag = right)
            MPI_Recv(&max_right, 1, MPI_INT, right, right, torus, MPI_STATUS_IGNORE);
            //print(" Sono il processore ", rank, "il mio right e' ", mesh.Get_coords(right), " messaggio ricevuto.  livello: ", liv) 
            if(max_right > local_max){
                local_max = max_right;
            }
        }     
    }
}

MPI_Barrier(torus);
int shallow, deep;

if((coords[1] == 0) & (coords[2] == 0)){
    for(int liv=1; liv<dims[0];liv++){
        //shallow, deep = torus.Shift(0, 2**(liv-1))
        MPI_Cart_shift(torus, 0, pow(2,(liv-1)), &shallow, &deep);
        //print(" Sono il processore", rank, ": ", torus.Get_coords(rank), "shallow: ", shallow, ": ", torus.Get_coords(shallow), "deep: ", deep, ": ", torus.Get_coords(deep), "livello: ", i)
        int k = pow(2,liv);
        if(coords[0] % k != 0){
            //print("sending, local_max:",local_max, ",          from.. ",rank, coords, "to..", shallow, torus.Get_coords(shallow))
            //torus.send(local_max, dest = shallow, tag = rank) 
            MPI_Send(&local_max , 1, MPI_INT , shallow , rank , torus);
            break;
        }
        else{
            //if((down != 0) or (rank == 0)):  
            //if(rank + 2**(i-1) < grid2d[0]):
            int u = pow(2, liv-1);
            if(coords[0] + u < dims[0]){
                //print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
                //max_deep = torus.recv(source = deep, tag = deep)     #   recive from any max_down
                MPI_Recv(&max_deep, 1, MPI_INT, deep, deep, torus, MPI_STATUS_IGNORE);
                //print("Sono il processore ", rank, "il mio right e' ", torus.Get_coords(right), " messaggio ricevuto. Livello: ", liv) 
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
