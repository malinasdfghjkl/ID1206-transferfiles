#include "dlmall.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
printf("start start \n");
init();
printf("post-init \n");
sanity();
printf("post-sanity \n");
struct head *p= dalloc(32);
printf("dalloc done memory at %ud \n", &p);
dfree(p);
printf("dfree done %ud \n", &p);

}