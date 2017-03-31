#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>     
                       
main()                 
{                      
  char c;              
                                             
  c = getchar();       
  while(c != EOF) {    
    putchar(c);        
    c = getchar();     
  }                    
}                      
