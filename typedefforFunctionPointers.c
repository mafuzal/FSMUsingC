#include<stdio.h>

typedef void (*printer_t)(int);
void print_to_n(int n)
{
    for (int i = 1; i <= n; ++i)
        printf("%d\n", i);
}

void print_n(int n)
{
    printf("%d\n", n);
}

int main()
{
    print_to_n(10);
    printf("-----------------------------\n");
    print_n(20);

    printf("-----------------------------\n");
    printer_t p = &print_to_n;
    void (*g)(int) = &print_n; // This would be required without the type
    p(3);           // Prints 1 2 3 4 5 on separate lines
    printf("-----------------------------\n");
    (*p)(3);        // So does this
    printf("-----------------------------\n");
     g(3);
     printf("-----------------------------\n");
     (*g)(3); 
     printf("-----------------------------\n");
    return 0;
    
}