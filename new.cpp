#include <stdio.h>
#include <iostream>

using namespace std;

typedef struct mradul
{
    int a;
    char b;
}mradul;

signed main()
{
    mradul m[4];
    char a[10]; 

    printf("%d\n%d\n", sizeof(m), sizeof(&a[0]));
    printf("%d\n", sizeof(char *));

    return 0;
}