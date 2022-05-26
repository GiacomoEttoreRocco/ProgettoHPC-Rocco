#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<omp.h>
#include<time.h>

int main(int argc, char **argv){
    int dim  = atoi(argv[1]);
    int nmax = 100000000;

// 2) topologia mesh: 
    MPI_Init(&argc , &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD , &size);  
    int dims[2];
    int MPI_Dims_create(size, 2, dims);
    MPI_Comm mesh;
    int qperiods[2] = {true, true};
    int MPI_Cart_create (MPI_COMM_WOLRD, 2, dims, qperiods, false, &mesh);
    int rank;
    MPI_Comm_rank(mesh, &rank);  
    int coords[2];
    MPI_Cart_coords(mesh, rank , 2, coords); 
    printf("In mesh topology, Processor %d has coordinates %d,%d", rank, coords[0], coords[1]);


int numbers[15];

if(rank == 0){
    //int *numbers = (int*)malloc(sizeof(int)*dim) // np.random.randint(nmax, size=dim)
    //DEBUG:
    for(int i = 0; i<15; i++){
        numbers[i] = i;
    }
    //---------------------
    //numbers = np.array_split(numbers, size)
}

//t0 = time.time()
double start = MPI_Wtime();

int *local_numbers = (int*)malloc(sizeof(int)*dim/size);
MPI_Scatter(numbers, dim/size, MPI_INT, local_numbers, dim/size, MPI_INT , 0, mesh);

int local_max = max(local_numbers);

int max_down = 0;
int max_right = 0;


//coords = mesh.Get_coords(rank)
int x = ceil(log2(dims[0])+1);
int up, down;

for(int i=1; i < x; i++){
    MPI_Cart_shift(mesh , 0, pow(2,(i-1)), &up, &down);
    //up, down = mesh.Shift(0, 2**(i-1))
    // print(" Sono il processore", rank, ": ", mesh.Get_coords(rank), "up: ", up, ": ",mesh.Get_coords(up), "down: ", down, ": ", mesh.Get_coords(down), "livello: ", i)
    int y = pow(2,i);
    if (coords[0] % y != 0){
        // print("Processore", rank, ".   Invio a ", up, " e mi tolgo, livello: ", i)
        MPI_Send( local_max , 1, MPI_INT , up , rank , mesh);
        break;
    }else{
        int k = pow(2, i-1);
        if(coords[0] + k < dims[0]){
            //print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            MPI_Recv(max_down, 1, MPI_INT, down, down, mesh, MPI_STATUS_IGNORE);
            //max_down = mesh.recv(source = down, tag = down)     #   recive from any max_down
            //print(" Sono il processore ", rank, "il mio down e' ", mesh.Get_coords(down), " messaggio ricevuto.  livello: ", i) 
            if(max_down > local_max){
                local_max = max_down;
            }
        }
    }
}

MPI_Barrier(mesh);
int left, right;

for(int liv = 1; liv < dims[1]; liv++){
    //left, right = mesh.Shift(1, 2**(liv-1))
    MPI_Cart_shift(mesh , 1, pow(2,(liv-1)), &left, &right);
    int temp = pow(2,liv);
    if((rank % temp) != 0){
        //mesh.send(local_max, dest = left, tag = rank) 
        MPI_Send(local_max , 1, MPI_INT , left , rank , mesh);
        break;
    }
    else{
        int p = pow(2,(liv-1));
        if(rank + p < dims[1]){
            //max_right = mesh.recv(source = right, tag = right)
            MPI_Recv(max_right, 1, MPI_INT, right, right, mesh, MPI_STATUS_IGNORE);
            //print(" Sono il processore ", rank, "il mio right e' ", mesh.Get_coords(right), " messaggio ricevuto.  livello: ", liv) 
            if(max_right > local_max){
                local_max = max_right;
            }
        }     
    }
}

double end = MPI_Wtime();


if(rank == 0){
    printf("%d",local_max);
    printf("\nTime: %lf", end-start);
}
    
MPI_Finalize();
}