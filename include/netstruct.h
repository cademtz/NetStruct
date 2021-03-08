#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// - Takes in a format string 'Fmt' (see: ENetStructFmt)
// - Returns a length > 0 on success, setting 'out_pBytes' to a new array (see: NetStruct_FreeBytes)
int NetStruct_PackFmt(uint8_t** out_pBytes, const char* Fmt, ...);
int NetStruct_PackFmtBuffer(uint8_t* Buffer, int BufLen, const char* Fmt, ...);
int NetStruct_PackFmtBufferVa(uint8_t* Buffer, int BufLen, const char* Fmt, va_list Args);
int NetStruct_UnpackFmt(const uint8_t* Bytes, int Len, const char* Fmt, ...); // - Returns read length > 0 on success

// - Writes a packed item to 'Buffer'
// - Returns written length > 0 on success
int NetStruct_PackItem(uint8_t* Buffer, int BufLen, char Code, const void* Item);
int NetStruct_UnpackItem(const uint8_t* Buffer, int BufLen, char Code, void* out_Item); // - Returns read length > 0 on success

void* _NetStruct_Alloc(size_t Size);
void _NetStruct_Free(void* Memory);
void NetStruct_FreeBytes(uint8_t* Bytes);

int NetStruct_FmtLen(const char* Fmt, ...); // - Length of arguments
int NetStruct_FmtLenVa(const char* Fmt, va_list Args);
char NetStruct_FmtToCode(char Fmt);
int NetStruct_ItemLen(char Code, const void* Item);

enum ENetStructFmt
{
	NetStructFmt_Bytes = 'B',	// - Pointer to NetStruct_Bytes struct
	NetStructFmt_String = 's',	// - UTF-8
	NetStructFmt_Int = 'i',
	NetStructFmt_Long = 'l',
	NetStructFmt_Float = 'f',
	NetStructFmt_Double = 'd',
};

enum ENetStructCode
{
	NetStructCode_Bytes = 0,
	NetStructCode_String,
	NetStructCode_Int,
	NetStructCode_Long,
	NetStructCode_Float,
	NetStructCode_Double,
};

typedef struct
{
	const uint8_t* buf;
	int len;
} NetStruct_Bytes;

#ifdef __cplusplus
}
	#include "netstruct.hpp"
#endif
