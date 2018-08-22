#include <stdio.h>
int main()
{
  int sun=0;
  int i=100;
  for(;i<=999;i++)
  {
    if(i%8==1&&i%9==1)
    {
      sun+=i;
    }
  }
  printf("%d",sun);
}
