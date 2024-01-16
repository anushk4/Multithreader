#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>

int user_main(int argc, char **argv);

// A struct to store all the arguments of the thread_func functions
typedef struct
{
  int start1;
  int start2;
  std::function<void(int)> lambda1;
  std::function<void(int, int)> lambda2;
  int end1;
  int end2;
} thread_args;

// thread_func to implement the vector addition in 1D
void *thread_func_1(void *ptr)
{
  // typecasting
  thread_args *t = ((thread_args *)ptr);
  for (int i = t->start1; i < t->end1; i++)
  {
    t->lambda1(i);
  }
  return NULL;
}

// thread_func to implement matrix multiplication in 2D
void *thread_func_2(void *ptr)
{
  // typecasting
  thread_args *t = ((thread_args *)ptr);
  for (int i = t->start1; i < t->end1; i++)
  {
    for (int j = t->start2; j < t->end2; j++)
    {
      t->lambda2(i, j);
    }
  }
  return NULL;
}

// Parallelising the lambda function in 1D
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads)
{
  struct timespec start_time;
  if (clock_gettime(CLOCK_MONOTONIC, &start_time) == -1)
  {
    perror("simple-multithreader.h: clock_gettime");
    exit(EXIT_FAILURE);
  }
  // initialising
  pthread_t tid[numThreads];
  thread_args args[numThreads];
  int size = high - low;
  int chunk = size / numThreads;
  int rem = size % numThreads;
  for (int i = 0; i < numThreads; i++)
  {
    args[i].start1 = low + i * chunk;
    args[i].lambda1 = lambda;
    args[i].end1 = low + (i + 1) * chunk;
    if (pthread_create(&tid[i],
                       NULL,
                       thread_func_1,
                       (void *)&args[i]) == -1)
    {
      std::cout << "pthread_create failed" << std::endl;
    }
  }
  for (int i = 0; i < numThreads; i++)
  {
    if (pthread_join(tid[i], NULL) != 0)
    {
      std::cout << "pthread_join failed" << std::endl;
    }
  }
  // calculating for remainder chunks who could not be implemented by threads in the for loop
  if (rem != 0)
  {
    for (int i = chunk * numThreads; i < size; i++)
    {
      lambda(i);
    }
  }
  struct timespec end_time;
  if (clock_gettime(CLOCK_MONOTONIC, &end_time) == -1)
  {
    perror("simple-multithreader.h: clock_gettime");
    exit(EXIT_FAILURE);
  }
  double execution_time = ((end_time.tv_sec - start_time.tv_sec) * 1000) + (end_time.tv_nsec + start_time.tv_nsec) / 1e6;
  std::cout << "Execution time: " << execution_time << " ms" << std::endl;
}

void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads)
{
  struct timespec start_time;
  if (clock_gettime(CLOCK_MONOTONIC, &start_time) == -1)
  {
    perror("simple-multithreader.h: clock_gettime");
    exit(EXIT_FAILURE);
  }
  // initialising 
  pthread_t tid[numThreads];
  thread_args args[numThreads];
  int size = high1 - low1;
  int chunk = size / numThreads;
  int rem = size % numThreads;
  for (int i = 0; i < numThreads; i++)
  {
    // Dividing the first matrix into sub matrix
    args[i].start1 = low1 + i * chunk;
    args[i].end1 = low1 + (i + 1) * chunk;
    // Every element of matrix 1 is multiplied by all the columns of the second matrix
    args[i].start2 = low2;
    args[i].end2 = high2;
    args[i].lambda2 = lambda;
    if (pthread_create(&tid[i],
                       NULL,
                       thread_func_2,
                       (void *)&args[i]) == -1)
    {
      std::cout << "pthread_create failed" << std::endl;
    }
  }
  for (int i = 0; i < numThreads; i++)
  {
    if (pthread_join(tid[i], NULL) != 0)
    {
      std::cout << "pthread_join failed" << std::endl;
    }
  }
  // calculating for remainder chunks who could not be implemented by threads in the for loop
  if (rem != 0)
  {
    for (int i = chunk * numThreads; i < size; i++)
    {
      for (int j = low2; j < high2; j++)
      {

        lambda(i, j);
      }
    }
  }
  struct timespec end_time;
  if (clock_gettime(CLOCK_MONOTONIC, &end_time) == -1)
  {
    perror("simple-multithreader.h: clock_gettime");
    exit(EXIT_FAILURE);
  }
  double execution_time = ((end_time.tv_sec - start_time.tv_sec) * 1000) + (end_time.tv_nsec + start_time.tv_nsec) / 1e6;
  std::cout << "Execution time: " << execution_time << " ms" << std::endl;
}

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> &&lambda)
{
  lambda();
}

int main(int argc, char **argv)
{
  /*
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be
   * explicity captured if they are used inside lambda.
   */
  int x = 5, y = 1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/ [/*by value*/ x, /*by reference*/ &y](void)
  {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout << "====== Welcome to Assignment-" << y << " of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);

  auto /*name*/ lambda2 = [/*nothing captured*/]()
  {
    std::cout << "====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main
