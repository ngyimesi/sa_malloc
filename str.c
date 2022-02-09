#include <stdio.h>
#include <stdlib.h>
#include "debugmalloc.h"
#include "sa_malloc.h"
typedef struct string
{
	char* str;
	unsigned int size;
} string;
string stringinit(char* be)
{
	//Here we have to count chars
	unsigned int len = 0;
	for(unsigned int i = 0; be[i] != '\0'; i = i+1)
		len = len+1;
	char *str = (char*)sa_malloc((len+1)*sizeof(char));
	if(str == NULL)
		return (string){NULL, 0};
	for(int i = 0; i < len; i = i+1 )
		str[i] = be[i];
	str[len] = '\0';
	return (string){str, len};
}
unsigned int stringlen(string str)
{
	return str.size;
}
string stringcpy(string str)
{
	unsigned int size = str.size; //why not...
	char *newstr = (char*)sa_malloc((size+1)*sizeof(char));
	for(int i = 0; i < size; i = i+1)
		newstr[i] = str.str[i];
	newstr[size] = '\0';
	return (string){newstr, size};
}
string stringcat(string first, string second)
{
	unsigned int newsize = first.size+second.size;
	char *newstr = (char*)sa_malloc((newsize+1)*sizeof(char));
	for(int i = 0; i < first.size; i = i+1)
		newstr[i] = first.str[i];
	unsigned int count = 0;
	for(int i = first.size; i < newsize; i = i+1)
	{	
		newstr[i] = second.str[count];
		count = count+1;
	}
	newstr[newsize] = '\0';
	return (string){newstr, newsize};
}
int main(void)
{
	string str = stringinit("valami");
	printf("%s\n", str);
	string str2 = stringcpy(str);
	printf("%s\n", str2);
	string str3 = stringcat(str, str2);
	printf("%s\n", str3);
	return 0;
}