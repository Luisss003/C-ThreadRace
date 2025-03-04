/*
 * File sortThreads.c
 * Luis Saenz (khe699)
 *
 * SortThreads.c is a program that takes in an integer from user, and then
 * creates a double array of random double values of that size. After this,
 * calls on sortThread_avg to sort the array with single thread. Then will
 * split array in two, sort those threads with sortThread_avg, and then
 * merge them with merging_avg before outputting the time it took for both
 * methods.
*/
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

//Struct to hold characteristics of array, such as size, address, and average
typedef struct {
  double *array;
  int length;
  double average;
} ThreadArray;

/*
 * Function: sortThread_avg(void *);
 * Usage: pthread_create(&tid, NULL, sortThread_avg, &tmpArr);
 * ------------------------------------------
 * This function called via pthreads will take an void ptr 
 * which is interpreted as a threadArray struct. It will then
 * sort it, and calculate the average. Both values are returned
 * via the struct.
*/
void *sortThread_avg(void *);

/*
 * Function: merging_Avg(void *);
 * Usage: pthread_create(&tid4, NULL, merging_Avg, taArray);
 * -----------------------------------------------
 * This function called via pthreads will take a void ptr
 * which is interpreted as an array of ThreadArray structs.
 * It will then take the structs in the array, merge them.
 * and calcualte the overage all average.
*/
void *merging_Avg(void *);

int main(int argc, char *argv[]) {
  pthread_t tid, tid2, tid3, tid4;//Create thread ID for each thread to be made
  int n = atoi(argv[1]); //Get first arguement from user
  int nHalf = n / 2; //Get half of the user size
  struct timespec ts_begin, ts_end; //Create time measurement variables
  int i, j;
  double elapsed;


  if(argc != 2) {
    printf("Usage Error: ./sortThreads numOfElements");
    exit(1);
  }

  if(n % 2 != 0){
    printf("Error: arguement must be even\n");
    exit(1);
  }
  double *A = (double *)malloc(n * sizeof(double)); //Create array A (original)
  double *B = (double *)malloc(n * sizeof(double)); //Create array copy
  
  //Allocate space for the first and second half arrays for 2 thread example
  double *AfirstHalf = (double *)malloc(nHalf * sizeof(double)); 
  double *AsecondHalf = (double *)malloc(nHalf * sizeof(double));

  //Fill primary array with random doubles
  for (i = 0; i < n; i++) {
    double random = ((double)rand() / RAND_MAX) * (100 - 1) + 1;
    A[i] = random;
  }
  
  //Copy values from array A to B
  for (i = 0; i < n; i++) {
    B[i] = A[i];
  }

  //Fill out both half arrays
  for (i = 0; i < nHalf; i++) {
    AfirstHalf[i] = A[i];
    AsecondHalf[i] = A[i + nHalf];
  }

  //Create struct to store the attributes of the array to be passed
  ThreadArray tmpArr = {B, n, 0.0}, ret;
  ThreadArray *retArr;

  //Single thread sort
  //Start clock
  clock_gettime(CLOCK_MONOTONIC, &ts_begin);
  //Start thread process sending the temp array to be sorted and avg computed
  pthread_create(&tid, NULL, sortThread_avg, &tmpArr);
  pthread_join(tid, (void **)&retArr); //Catch return value 
  clock_gettime(CLOCK_MONOTONIC, &ts_end); //End clock

  //Calculate time required from single thread
  elapsed = ts_end.tv_sec - ts_begin.tv_sec;
  elapsed += (ts_end.tv_nsec - ts_begin.tv_nsec) / 1000000000.0;

  //Save return value locally
  ret = *retArr;
  printf("Sorting in 1 thread is done in %lf ms\n", elapsed * 1000.0);
  printf("Average: %lf\nFirst 10 elements in array\n", ret.average);
  for (i = 0; i < 10; i++) {
    printf("%d: %lf\n", i, ret.array[i]);
  }

  printf("\n");

  //Two thread sort
  //Create both structs for each of the half arrays
  ThreadArray tmpArr2 = {AfirstHalf, nHalf, 0.0};
  ThreadArray tmpArr3 = {AsecondHalf, nHalf, 0.0};

  ThreadArray *retArr2, *retArr3;
  //Allocate space taArray, which is an array of two ThreadArray structs
  ThreadArray *taArray = (ThreadArray *)malloc(2 * sizeof(ThreadArray));
  ThreadArray *retArray;

  //Start timer
  clock_gettime(CLOCK_MONOTONIC, &ts_begin);

  //Call threads to process each array half
  pthread_create(&tid2, NULL, sortThread_avg, &tmpArr2);
  pthread_create(&tid3, NULL, sortThread_avg, &tmpArr3);

  //Wait and catch the return values from the two array halves
  pthread_join(tid2, (void **)&retArr2);
  pthread_join(tid3, (void **)&retArr3);

  //allocate space for first half of result array and copy onto local struct
  taArray[0].array = (double *)malloc(retArr2->length * sizeof(double));
  memcpy(taArray[0].array, retArr2->array, retArr2->length * sizeof(double));
  taArray[0].length = retArr2->length; //copy first halves length
  taArray[0].average = retArr2->average; //and average calcualted

  //Do the same for the second half of the array
  taArray[1].array = (double *)malloc(retArr3->length * sizeof(double));
  memcpy(taArray[1].array, retArr3->array, retArr3->length * sizeof(double));
  taArray[1].length = retArr3->length;
  taArray[1].average = retArr3->average;
 
  //Free temporary structures
  free(retArr2);
  free(retArr3);

  //Merge the two arrays by passing the array of structs
  pthread_create(&tid4, NULL, merging_Avg, taArray);
  pthread_join(tid4, (void **)&retArray); //Catch structure of merged array

  clock_gettime(CLOCK_MONOTONIC, &ts_end); //End timer

  //Caculate time taken by two threads
  elapsed = ts_end.tv_sec - ts_begin.tv_sec;
  elapsed += (ts_end.tv_nsec - ts_begin.tv_nsec) / 1000000000.0;

  printf("Sorting by 2 threads is done in %lf ms\n", elapsed * 1000.0);
  printf("Average: %lf\nFirst 10 elements in array:\n", retArray->average);
 
  //Print first 10 elements of array
  for (i = 0; i < 10; i++) {
    printf("%d: %lf\n", i, retArray->array[i]);
  }

  //Free allocated memory
  free(taArray[0].array);
  free(taArray[1].array);
  free(taArray);
  free(retArray);

  return 0;
}

/*
 * Function: sortThread_avg(void *);
 * Usage: pthread_create(&tid, NULL, sortThread_avg, &tmpArr);
 * ------------------------------------------
 * This function called via pthreads will take an void ptr
 * which is interpreted as a threadArray struct. It will then
 * sort it, and calculate the average. Both values are returned
 * via the struct.
*/
void *sortThread_avg(void *arg) {
  ThreadArray *localcpy = (ThreadArray *)arg;
  int n = localcpy->length;
  int i, j;
  double *arr = localcpy->array;

  ThreadArray *myptrret = (ThreadArray *)malloc(sizeof(ThreadArray));

  //Set up the values for the return value ThreadArray struct
  myptrret->length = n; 
  myptrret->array = (double *)malloc(n * sizeof(double)); 
  memcpy(myptrret->array, arr, n * sizeof(double));
  //Selection sort to sort array
  for (i = 0; i < n - 1; i++) {
    int minIdx = i;
    for (j = i + 1; j < n; j++) {
      if (myptrret->array[j] < myptrret->array[minIdx]) {
        minIdx = j;
      }
    }
    //Swap found min lement with first element of unsorted part
    double temp = myptrret->array[minIdx];
    myptrret->array[minIdx] = myptrret->array[i];
    myptrret->array[i] = temp;
  }

  //Calculate and store average of overall array
  double sum = 0.0;
  for (i = 0; i < n; i++) {
    sum += myptrret->array[i];
  }
  myptrret->average = sum / n;
  //Return void ptr to return ThreadArray struct
  return (void *)myptrret;
}

/*
 * Function: merging_Avg(void *);
 * Usage: pthread_create(&tid4, NULL, merging_Avg, taArray);
 * -----------------------------------------------
 * This function called via pthreads will take a void ptr
 * which is interpreted as an array of ThreadArray structs.
 * It will then take the structs in the array, merge them.
 * and calcualte the overage all average.
*/
void *merging_Avg(void *arg) {
  //Cast input arg to a pointer to array of Threadarray Structs
  ThreadArray *localcpy = (ThreadArray *)arg;

  //Get sizes of both arrays
  int size1 = localcpy[0].length;
  int size2 = localcpy[1].length;

  //Get both arrays and allocate enouhg space for both in single array
  double *arr1 = localcpy[0].array;
  double *arr2 = localcpy[1].array;
  double *merged = (double *)malloc((size1 + size2) * sizeof(double));

  //Get averages of both arrays
  double avg1 = localcpy[0].average;
  double avg2 = localcpy[1].average;
  double retAvg;
  int i;

  //Create return value ThreadArray struct
  ThreadArray *myptrret = (ThreadArray *)malloc(sizeof(ThreadArray));

  //Indxes to traverse both  arrays and insert into merged array
  int i1 = 0, i2 = 0, mergedIndex = 0;

  //Merge both sorted arrays into one sorted array
  while (i1 < size1 && i2 < size2) {
    if (arr1[i1] < arr2[i2]) {
      merged[mergedIndex++] = arr1[i1++];
    } 
    else {
      merged[mergedIndex++] = arr2[i2++];
    }
  }

  // Copy remaining elements from arr1
  while (i1 < size1) {
    merged[mergedIndex++] = arr1[i1++];
  }

  // Copy remaining elements from arr2
  while (i2 < size2) {
    merged[mergedIndex++] = arr2[i2++];
  }

  //Simple calculation of average overall
  retAvg = (avg1 + avg2) / 2;

  //Set return value struct's fields, and return void ptr of it
  myptrret->array = merged;
  myptrret->average = retAvg;
  myptrret->length = size1 + size2;
  return (void *)myptrret;
}


