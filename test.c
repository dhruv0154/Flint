#include <stdio.h>

int main(int argc, char const *argv[])
{
    int arr[5] = {1, 2, 3, 2, 1};

    for (size_t i = 0; i < 5; i++)
    {
        int b = 0;
        for (size_t j = 0; j < 5; i++)
        {
            if(arr[i] == arr[j] && j > i)
                b++;
        }
    }
    
    return 0;
}
