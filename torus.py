from turtle import up
from mpi4py import MPI
import numpy as np
import time
import math as math
import sys

dim  = int(sys.argv[1])
nmax = 100000000

# 2) topologia mesh: 

comm = MPI.COMM_WORLD # communicator di default
size = comm.Get_size()

#ase = math.floor(math.sqrt(size))
#height = math.floor(math.sqrt(size))
#while( size - (base*height) >= height):
#    base = base +1

rank = comm.Get_rank() # rank del processore all'interno di comm.
#mesh = comm.Create_cart(dims = [base, height], periods =[True, True], reorder=False) # creo un nuovo communicator
grid3d = MPI.Compute_dims(size, 3)
#print(grid2d[0])
torus = comm.Create_cart(dims = grid3d, periods =[True, True, True], reorder=False) # creo un nuovo communicator

#coords = mesh.Get_coords(rank)
#print ("In mesh topology, Processor ",rank, " has coordinates ", coords)

if rank == 0:
    numbers = np.random.randint(nmax, size=dim)
    #DEBUG:
    #numbers = [0,180,2,3,4,5,600,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29]
    numbers = np.array_split(numbers, size)
else:
    numbers = []

t0 = time.time()

local_numbers = torus.scatter(numbers, root = 0)

local_max = max(local_numbers)

max_down = 0

coords = torus.Get_coords(rank)

for i in range(1, math.ceil(math.log2(grid3d[1])+1)):
    up, down = torus.Shift(1, 2**(i-1))
    #print(" Sono il processore", rank, ": ", torus.Get_coords(rank), "up: ", up, ": ", torus.Get_coords(up), "down: ", down, ": ", torus.Get_coords(down), "livello: ", i)
    if (coords[1] % (2**i)) != 0:
        #print("Processore", rank, ".   Invio a ", up, " e mi tolgo, livello: ", i)
        torus.send(local_max, dest = up, tag = rank)   #   send to left local_max
        break
    
    else:
        #if((down != 0) or (rank == 0)):  
        #if(rank + 2**(i-1) < grid2d[0]):
        if(coords[1] + 2**(i-1) < grid3d[1]):
            #print("Processore ", rank," riceverei da down: ", down, " il mio up: ", up, ", livello: ", i)
            max_down = torus.recv(source = down, tag = down)     #   recive from any max_down
            #print(" Sono il processore ", rank, "il mio down e' ", mesh.Get_coords(down), " messaggio ricevuto.  livello: ", i) 
            if max_down > local_max:
                local_max = max_down

torus.Barrier()

#max_right = 0 # <- debug

for liv in range(1, grid3d[2]):
    left, right = torus.Shift(2, 2**(liv-1))
    #print(" Sono il processore", rank, ": ", torus.Get_coords(rank), "left: ", left, ": ", torus.Get_coords(left), "right: ", right, ": ", torus.Get_coords(right), "livello: ", i)
    if(rank % (2**liv)) != 0:
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

t1 = time.time()

if(rank == 0):
    print(local_max)
    print("Time: ", t1-t0)


#nup, ndown = mesh.Shift(0, 1) # il secondo paramentro è di quanto voglio spostarmi a destra. 

