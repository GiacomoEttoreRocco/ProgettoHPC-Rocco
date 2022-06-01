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
    int *numbers;
    numbers = (int*)malloc(sizeof(int)*dim);

    for(int i = 0; i<dim; i++){
        numbers[i] = i; 
    }

    double start = clock();
    int local_max = find_max(numbers, dim);
    double end = clock();

    printf("\n%d",local_max);
    double t = end-start;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("\nTime: %lf", time_taken); 
}


