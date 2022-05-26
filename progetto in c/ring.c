#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<omp.h>
#include<time.h>

int main(int argc, char **argv){
    int dim  = atoi(argv[1]);
    int nmax = 100000000;

// 1) topologia ring:

comm = MPI.COMM_WORLD       # communicator di default
rank = comm.Get_rank()      # rank del processore all'interno di comm.
size = comm.Get_size()

highests = []

ring = comm.Create_cart(dims = [size], periods =[True], reorder=False) 

left, right = ring.Shift(0, 1)

if rank == 0:
    numbers = np.random.randint(nmax, size=dim)

    #DEBUG:

    #numbers = [0,1,2,3,4,5,6,7,8,9,10,11,12,13]

    numbers = np.array_split(numbers, size)
else:
    numbers = []

t0 = time.time()

local_numbers = ring.scatter(numbers, root = 0)

local_max = max(local_numbers)

#x = ring.reduce(max, MPI.MAX, root = 0)

# all the odd numbers send to the left and right neighborhood

#Debug:
#left, right = ring.Shift(0, 2**(1-1))
#max_right = 9
#fine debug

#print(" Sono il processore ", rank, "sx: ", left, "dx: ", right)

for i in range(1, math.ceil(math.log2(size)+1)):
    left, right = ring.Shift(0, 2**(i-1))

    #print(" Sono il processore ", rank, "sx: ", left, "dx: ", right)

    if (rank % (2**i)) != 0:
        #print("Processore", rank, ".   Invio a ", left, " e mi tolgo, livello: ", i)
        ring.send(local_max, dest = left, tag = rank)   #   send to left local_max
        break
    
    else:
        #if((right != 0) or (rank == 0)):  
        if(rank + 2**(i-1) < size):
            #print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            max_right = ring.recv(source = right, tag = right)     #   recive from any max_right
            #print(" Sono il processore ", rank, "il mio destro e' ", right, " messaggio ricevuto.  livello: ", i) 
            if max_right > local_max:
                local_max = max_right

t1 = time.time()

if(rank == 0):
    print(local_max)
    print("Time: ", t1-t0)



#if rank == 0:
#    print(max(highests))