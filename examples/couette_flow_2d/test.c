#include <stdio.h>
#include <string.h>
void test(int argc, char** argv)
{
  for (int i=0;i<argc;i++)
    printf("%s\n",argv[i]);
}
