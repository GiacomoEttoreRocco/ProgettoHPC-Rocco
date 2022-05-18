# coding=utf-8
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
grid2d = MPI.Compute_dims(size, 2)
#print(grid2d[0])
mesh = comm.Create_cart(dims = grid2d, periods =[True, True], reorder=False) # creo un nuovo communicator

#coords = mesh.Get_coords(rank)
#print ("In mesh topology, Processor ",rank, " has coordinates ", coords)

if rank == 0:
    numbers = np.random.randint(nmax, size=dim)
    #DEBUG:
    #numbers = [99,187,2,3,4,5,60,7,8,9,10,11,1892,13,1000]
    numbers = np.array_split(numbers, size)
else:
    numbers = []

t0 = time.time()

local_numbers = mesh.scatter(numbers, root = 0)

local_max = max(local_numbers)

max_down = 0

coords = mesh.Get_coords(rank)

for i in range(1, math.ceil(math.log2(grid2d[0])+1)):
    up, down = mesh.Shift(0, 2**(i-1))
    #print(" Sono il processore", rank, ": ", mesh.Get_coords(rank), "up: ", up, ": ",mesh.Get_coords(up), "down: ", down, ": ", mesh.Get_coords(down), "livello: ", i)
    if (coords[0] % (2**i)) != 0:
        #print("Processore", rank, ".   Invio a ", up, " e mi tolgo, livello: ", i)
        mesh.send(local_max, dest = up, tag = rank)   #   send to left local_max
        break
    
    else:
        #if((down != 0) or (rank == 0)):  
        #if(rank + 2**(i-1) < grid2d[0]):
        if(coords[0] + 2**(i-1) < grid2d[0]):
            #print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            max_down = mesh.recv(source = down, tag = down)     #   recive from any max_down
            #print(" Sono il processore ", rank, "il mio down e' ", mesh.Get_coords(down), " messaggio ricevuto.  livello: ", i) 
            if max_down > local_max:
                local_max = max_down

mesh.Barrier()

for liv in range(1, grid2d[1]):
    left, right = mesh.Shift(1, 2**(liv-1))

    if(rank % (2**liv)) != 0:
        mesh.send(local_max, dest = left, tag = rank) 
        break

    else:
        #if((down != 0) or (rank == 0)):  
        #if(rank + 2**(i-1) < grid2d[0]):
        if(rank + 2**(liv-1) < grid2d[1]):
            #print("Processore ", rank," riceverei da right: ", right, " il mio left: ", left, ", livello: ", i)
            max_right = mesh.recv(source = right, tag = right)     #   recive from any max_down
            #print(" Sono il processore ", rank, "il mio right e' ", mesh.Get_coords(right), " messaggio ricevuto.  livello: ", liv) 
            if max_right > local_max:
                local_max = max_right

t1 = time.time()

if(rank == 0):
    print(local_max)
    print("Time: ", t1-t0)


#nup, ndown = mesh.Shift(0, 1) # il secondo paramentro è di quanto voglio spostarmi a destra. 

