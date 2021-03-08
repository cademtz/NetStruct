/*
* Compile this file as Cpp to get Cpp-only examples
*/

#include "include/netstruct.h"
#include <stdio.h>

#ifdef __cplusplus

typedef struct _innerstruct
{
	const char* s;
	NetStruct_Bytes bytes;

	NETSTRUCT_MAP(s, bytes);
} InnerStruct;

typedef struct _teststruct
{
	InnerStruct base;
	int i;
	const char* s;
	double d;

	NETSTRUCT_MAP(base, i, s, d);
} TestStruct;

#endif

int main()
{
#ifdef __cplusplus
	puts("Running C++ example");

	uint8_t arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	TestStruct test = { {0}, 6887, "Hello from TestStruct", 9.321 };
	uint8_t* bytes;
	int buflen;

	test.base.s = "String in base";
	test.base.bytes.buf = arr;
	test.base.bytes.len = sizeof(arr);

	buflen = NetStruct::Pack(&bytes, test);
	if (buflen <= 0)
	{
		puts("Failed to pack struct");
		return -1;
	}

	// --- Unpack only test.base ---

	InnerStruct base;

	if (NetStruct::Unpack(bytes, buflen, base) <= 0)
	{
		puts("Failed to unpack struct");
		return -1;
	}

	printf("\"%s\", base.bytes.len: %d\n", base.s, base.bytes.len);
	NetStruct::Free(bytes);
#else
	puts("Running C example");

	int i;
	char* s;
	double d;
	uint8_t* bytes;
	//int buflen = NetStruct::PackFmt(&bytes, 6887, u8"kewlio", 9.321); // Cpp-version
	int buflen = NetStruct_PackFmt(&bytes, "isd", 6887, u8"kewlio", 9.321); // C-version
	
	if (buflen <= 0)
	{
		puts("Failed to pack items");
		return -1;
	}

	//if (NetStruct::UnpackFmt(bytes, buflen, i, s, d) <= 0) // Cpp-version
	if (NetStruct_UnpackFmt(bytes, buflen, "isd", &i, &s, &d) <= 0) // C-version
	{
		puts("Failed to unpack items");
		return -1;
	}

	printf("%d \"%s\" %f\n", i, s, d);
	NetStruct_FreeBytes(bytes);
#endif

	return 0;
}