#include "../include/netstruct.h"
#include "../include/netstruct_endian.h"
#include <string.h>
#include <malloc.h>

#if defined(NETSTRUCT_DEBUG)
#include <assert.h>
#define NETSTRUCT_ASSERT assert
#else
#define NETSTRUCT_ASSERT(x)
#endif

void* _NetStruct_Alloc(size_t Size) { return malloc(Size); }
void _NetStruct_Free(void* Memory) { free(Memory); }
void NetStruct_FreeBytes(uint8_t* Bytes) { return _NetStruct_Free(Bytes); }

int NetStruct_PackFmt(uint8_t** out_pBytes, const char* Fmt, ...)
{
	va_list va;
	va_start(va, Fmt);
	int len = NetStruct_PackFmtVa(out_pBytes, Fmt, va);
	va_end(va);
	return len;
}

int NetStruct_PackFmtVa(uint8_t** out_pBytes, const char* Fmt, va_list Args)
{
	va_list va_pack;
	va_list va_len;

	va_copy(va_pack, Args);
	va_copy(va_len, Args);

	int len, written;
	uint8_t* bytes;
	if ((len = NetStruct_FmtLenVa(Fmt, va_len)) <= 0)
	{
		va_end(va_len);
		va_end(va_pack);
		return -1;
	}

	va_end(va_len);

	bytes = (uint8_t*)_NetStruct_Alloc(len);
	if ((written = NetStruct_PackFmtBufferVa(bytes, len, Fmt, va_pack)) <= 0)
	{
		va_end(va_pack);
		return _NetStruct_Free(bytes), -1;
	}
	
	va_end(va_pack);

	*out_pBytes = bytes;
	return len;
}

int NetStruct_PackFmtBuffer(uint8_t* Buffer, int BufLen, const char* Fmt, ...)
{
	va_list va;
	va_start(va, Fmt);
	int len = NetStruct_PackFmtBufferVa(Buffer, BufLen, Fmt, va);
	va_end(va);
	return len;
}

int NetStruct_PackFmtBufferVa(uint8_t* Buffer, int BufLen, const char* Fmt, va_list Args)
{
	int off = 0;
	va_list va;
	va_copy(va, Args);

	for (int i = 0; Fmt[i]; i++)
	{
		const void* var;
		int count;
		int code = NetStruct_FmtToCode(Fmt[i]);
		union
		{
			float fl; // Exists as a hack to avoid packing doubles where varargs do not allow floats
			int i;
			double d;
			int64_t il;
		} uni; // Exists as a DOUBLE hack. Can't use &var_arg(...) on MSVC x64 Release mode

		switch (code) // Must use appropriate va_arg calls to avoid size problems
		{
		case NetStructCode_Bytes: var = va_arg(va, const NetStruct_Bytes*); break;
		case NetStructCode_String: var = va_arg(va, const char*); break;
		case NetStructCode_Int: uni.i = va_arg(va, int), var = &uni.i; break;
		case NetStructCode_Long: uni.il = va_arg(va, int64_t), var = &uni.il; break;
		case NetStructCode_Float: uni.fl = (float)va_arg(va, double), var = &uni.fl; break;
		case NetStructCode_Double: uni.d = va_arg(va, double), var = &uni.d; break;
		default:
			va_end(va);
			return -1;
		}

		count = NetStruct_PackItem(Buffer + off, BufLen - off, code, var);
		if (count <= 0)
		{
			va_end(va);
			return -1;
		}
		off += count;
	}

	va_end(va);
	return off;
}

int NetStruct_UnpackFmt(const uint8_t* Bytes, int Len, const char* Fmt, ...)
{
	va_list va;
	int off = 0;

	va_start(va, Fmt);

	for (int i = 0; Fmt[i]; i++)
	{
		int count = NetStruct_UnpackItem(Bytes + off, Len - off, NetStruct_FmtToCode(Fmt[i]), va_arg(va, void*));
		if (count <= 0)
		{
			va_end(va);
			return -1;
		}

		off += count;
	}

	va_end(va);
	return off;
}

int NetStruct_FmtLen(const char* Fmt, ...)
{
	va_list va;
	va_start(va, Fmt);
	int len = NetStruct_FmtLenVa(Fmt, va);
	va_end(va);
	return len;
}

int NetStruct_FmtLenVa(const char* Fmt, va_list Args)
{
	int len = 0;
	va_list va;
	va_copy(va, Args);

	for (int i = 0; Fmt[i]; i++)
	{
		char code = NetStruct_FmtToCode(Fmt[i]);
		union
		{
			float fl;
			int i;
			double d;
			int64_t il;
		} uni; // Exists as a DOUBLE hack. Can't use &var_arg(...) on MSVC x64 Release mode

		if (code <= 0)
		{
			va_end(va);
			return -1;
		}

		switch (code)
		{
		case NetStructCode_Bytes: len += NetStruct_ItemLen(code, va_arg(va, NetStruct_Bytes*)); break;
		case NetStructCode_String: len += NetStruct_ItemLen(code, va_arg(va, const char*)); break;
		case NetStructCode_Int: len += NetStruct_ItemLen(code, (uni.i = va_arg(va, int), &uni.i)); break;
		case NetStructCode_Long: len += NetStruct_ItemLen(code, (uni.il = va_arg(va, int64_t), &uni.il)); break;
		case NetStructCode_Float: len += NetStruct_ItemLen(code, (uni.fl = (float)va_arg(va, double), &uni.fl)); break;
		case NetStructCode_Double: len += NetStruct_ItemLen(code, (uni.d = va_arg(va, double), &uni.d)); break;
		default:
			va_end(va);
			return -1;
		}
	}

	va_end(va);
	return len;
}

char NetStruct_FmtToCode(char Fmt)
{
	switch (Fmt)
	{
	case NetStructFmt_Bytes: return NetStructCode_Bytes;
	case NetStructFmt_String: return NetStructCode_String;
	case NetStructFmt_Int: return NetStructCode_Int;
	case NetStructFmt_Long: return NetStructCode_Long;
	case NetStructFmt_Float: return NetStructCode_Float;
	case NetStructFmt_Double: return NetStructCode_Double;
	default: NETSTRUCT_ASSERT(0 && "Unknown NetStruct format");
	}
	return -1;
}

int NetStruct_ItemLen(char Code, const void* Item)
{
	switch (Code)
	{
	case NetStructCode_Bytes: return ((const NetStruct_Bytes*)Item)->len + sizeof(int);
	case NetStructCode_String: return (Item ? strlen((const char*)Item) : 0) + 1;
	case NetStructCode_Int: return sizeof(int);
	case NetStructCode_Long: return sizeof(int64_t);
	case NetStructCode_Float: return sizeof(float);
	case NetStructCode_Double: return sizeof(double);
	default: NETSTRUCT_ASSERT(0 && "Unknown NetStruct code");
	}
	return -1;
}

int NetStruct_PackItem(uint8_t* Buffer, int BufLen, char Code, const void* Item)
{
	int len = NetStruct_ItemLen(Code, Item);

	if (len <= 0 || BufLen < len)
	{
		printf("BufLen: %d, ItemLen: %d, ItemCode: %d\n", BufLen, len, Code);
		NETSTRUCT_ASSERT(len > 0 && "NetStruct_ItemLen failed");
		NETSTRUCT_ASSERT(len <= BufLen && "Item length exceeds buffer");
		return -1;
	}

	switch (Code)
	{
	case NetStructCode_Bytes:
	{
		const NetStruct_Bytes* bytes = (const NetStruct_Bytes*)Item;
		int datlen = htobe32(bytes->len);
		memcpy(Buffer, &datlen, sizeof(datlen));
		memcpy(Buffer + sizeof(datlen), bytes->buf, bytes->len);
		break;
	}
	case NetStructCode_String:
		if (Item)
			memcpy(Buffer, Item, (size_t)len);
		else
			Buffer[len - 1] = 0;
		break;
	case NetStructCode_Int: 
	case NetStructCode_Float:
	{
		int i = htobe32(*(int*)Item);
		memcpy(Buffer, &i, sizeof(i));
		break;
	}
	case NetStructCode_Long:
	case NetStructCode_Double:
	{
		int64_t i = htobe64(*(int64_t*)Item);
		memcpy(Buffer, &i, sizeof(i));
		break;
	}
	default:
		NETSTRUCT_ASSERT(0 && "Unknown NetStruct code");
		len = -1;
	}
	return len;
}

int NetStruct_UnpackItem(const uint8_t* Buffer, int BufLen, char Code, void* out_Item)
{
	int off = 0;

	if (BufLen <= 0)
		return -1;

	switch (Code)
	{
	case NetStructCode_Bytes:
	{
		int len;
		if ((off = NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_Int, &len)) <= 0 || len <= 0)
			return -1;

		Buffer += off, BufLen -= off;
		if (BufLen < len)
			return -1;

		NetStruct_Bytes* bytes = (NetStruct_Bytes*)out_Item;
		bytes->len = len;
		bytes->buf = Buffer;
		off += len;
		break;
	}
	case NetStructCode_String:
	{
		int len = 0;
		const char* src = (const char*)Buffer;

		for (; len + 1 <= BufLen && src[len]; len++);
		NETSTRUCT_ASSERT(len <= BufLen && "ASCII exceeds buffer length");
		NETSTRUCT_ASSERT(!src[len] && "No ASCII null-terminator");
		if (len + 1 > BufLen || src[len]) // No null-terminator
			return -1;

		*(const char**)out_Item = src;
		off = len + 1; // Include null-terminator in offset
		break;
	}
	case NetStructCode_Int:
	{
		NETSTRUCT_ASSERT(BufLen >= sizeof(int));
		if (BufLen < sizeof(int))
			return -1;

		*(int*)out_Item = be32toh(*(int*)Buffer);
		off += sizeof(int);
		break;
	}
	case NetStructCode_Float:
	{
		NETSTRUCT_ASSERT(BufLen >= sizeof(float));
		if (BufLen < sizeof(float))
			return -1;

		int i = be32toh(*(int*)Buffer);
		*(float*)out_Item = *(float*)&i;
		off += sizeof(float);
		break;
	}
	case NetStructCode_Long:
	{
		NETSTRUCT_ASSERT(BufLen >= sizeof(int64_t));
		if (BufLen < sizeof(int64_t))
			return -1;

		*(int64_t*)out_Item = be64toh(*(int64_t*)Buffer);
		off += sizeof(int64_t);
		break;
	}
	case NetStructCode_Double:
	{
		NETSTRUCT_ASSERT(BufLen >= sizeof(double));
		if (BufLen < sizeof(double))
			return -1;

		int64_t i = be64toh(*(int64_t*)Buffer);
		*(double*)out_Item = *(double*)&i;
		off += sizeof(double);
		break;
	}
	default:
		NETSTRUCT_ASSERT(0 && "Unknown NetStruct code");
		off = -1;
	}
	return off;
}
