#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define INIT_SIZE 4
int * str2int(char str[]){
	char delim[] = " ";
	char *ptr = strtok(str, delim);
	static int array1[INIT_SIZE];
	int i = 0;
	while(ptr != NULL)
	{
		array1[i] = (int) strtol(ptr, (char **)NULL, 10);
		i++;
		ptr = strtok(NULL, delim);
	}
return array1;
}
int main() {
    char str[] = "12 22 12 22";
    int *point1 = str2int(str);
    printf("answer is %d",*(point1+3));
    

    return 0;
}