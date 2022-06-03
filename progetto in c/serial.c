#include<stdio.h>
#include<stdlib.h>
#include<math.h>
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
    int NUMBER_OF_REPS = atoi(argv[2]);

    int *numbers;
    numbers = (int*)malloc(sizeof(int)*dim);

    for(int i = 0; i<dim; i++){
        numbers[i] = i; 
    }

double start, end, sum_time, mean_time;
int local_max;
for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
//#############     INIZIO CALCOLO TEMPO     ###############
    start = clock();
    local_max = find_max(numbers, dim);
    end = clock();
}
//#############     FINE CALCOLO TEMPO     ###############
sum_time += end-start;
printf("-");

mean_time = sum_time;
    printf("\n%d",local_max);
    //double t = end-start;
    double time_taken = mean_time/CLOCKS_PER_SEC;
    printf("\nTime for %d times: %lf", NUMBER_OF_REPS, time_taken); 
}


