#pragma GCC push_options
#pragma GCC optimize ("O0")

#include<stdio.h>
#include<stdlib.h>
//#include<math.h>
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
    double NUMBER_OF_REPS = atoi(argv[2]);
    int *numbers;
    numbers = (int*)malloc(sizeof(int)*dim);

    for(int i = 0; i<dim; i++){
        numbers[i] = i;
    }

double start, end;
int local_max;
double mean_time, time;
start = clock();

for(int rep = 0; rep < NUMBER_OF_REPS; rep++){
    local_max = find_max(numbers, dim);
    numbers[0] = NUMBER_OF_REPS;
}

end = clock();

mean_time = ((double)(end-start))/(NUMBER_OF_REPS);
time = (double)mean_time/((double)(CLOCKS_PER_SEC));
printf("%lf\n", time);
}


#pragma GCC pop_options
