#include <stdlib.h>
struct Stat {
  int sum; 
  int avg;
  int mean;
}; 

void function(void) {
 int* p = malloc(sizeof(int)*1024);
 struct Stat* stat = malloc(sizeof(struct Stat));
 for (int i = 0; i < 1024; i++) {
  p[i] = rand()*100;
 }
 int sum = 0;
 for (int i = 0; i < 1024; i++) {
  sum += p[i];
 }
 stat->sum = sum;
}

