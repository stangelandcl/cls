#include <stdlib.h>

typedef string
{
	char *data;
	size_t size;
} string;

static string string_init(const char* data, size_t size)
{
	union { string s; char p[sizeof(string)]; } x;
	if(size <= sizeof(x) - 1)
	{
		memcpy(x.p, data, size);
	        x.p[sizeof(x) - 1] = (char)(size << 1);
	}
	else
	{
		x.s.data = malloc(size);
		if(x.s.data)
		{
			memcpy(x.s.data, data, size);
			x.s.size = (size << 1) | 1;		
		} else x.s.size = 0;
		return x.s;
	}
}
static size_t string_size(string s) 
{ 
	union { string *s; char* c; } x = { &s };	
	if(x.c[sizeof(x) - 1] & 1) return s.size >> 1;
	else return x.c[sizeof(x) - 1] >> 1; 
}
static char* string_data(string s) 
{ 
	union { string *s; char* c; } x = { &s };
	if(x.c[sizeof(x) - 1] & 1) return s.data;
	else return &x.c[0];
}
static void string_destroy(string s)
{
	union { string *s; char* c; } x = { &s };
	if(x.c[sizeof(x) - 1] & 1) free(s.data);
}

