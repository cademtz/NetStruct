#pragma once
#include "netstruct.h"
#include <string.h>

#define NETSTRUCT_MAP(...)	\
	inline int _NetStruct_Pack(uint8_t** out_pBytes) const {			\
		return NetStruct::Pack(out_pBytes, __VA_ARGS__);				\
	}																	\
	inline int _NetStruct_Pack(uint8_t* Buffer, int BufLen) const {		\
		return NetStruct::PackFmtBuffer(Buffer, BufLen, __VA_ARGS__);	\
	}																	\
	inline int _NetStruct_Unpack(const uint8_t* Buffer, int BufLen) {	\
		return NetStruct::Unpack(Buffer, BufLen, __VA_ARGS__);			\
	}																	\
	inline int _NetStruct_FmtLen() const {								\
		return NetStruct::FmtLen(__VA_ARGS__);							\
	}

namespace NetStruct
{
	inline void Free(uint8_t* NetBytes) { NetStruct_FreeBytes(NetBytes); }

	template <class T>
	inline int FmtLen(const T& Item) { return Item._NetStruct_FmtLen(); }
	inline int FmtLen(const NetStruct_Bytes& Bytes) { return Bytes.len > 0 ? Bytes.len + sizeof(int) : -1; }
	inline int FmtLen(const char* Str) { return !Str ? 1 : strlen(Str) + 1; }
	inline int FmtLen(const int& Int) { return sizeof(Int); }
	inline int FmtLen(const int64_t& Long) { return sizeof(Long); }
	inline int FmtLen(const float& Fl) { return sizeof(Fl); }
	inline int FmtLen(const double& Db) { return sizeof(Db); }

	template<class T1, class T2, class ...TArgs>
	inline int FmtLen(const T1& Item1, const T2& Item2, const TArgs& ...Etc) {
		int len = FmtLen(Item1);
		if (len > 0)
			return len + FmtLen(Item2, Etc...);
		return -1;
	}

	template<class T>
	inline int PackItem(uint8_t* Buffer, int BufLen, const T& Item) { return Item._NetStruct_Pack(Buffer, BufLen); }
	inline int PackItem(uint8_t* Buffer, int BufLen, const NetStruct_Bytes* Item) {
		return NetStruct_PackItem(Buffer, BufLen, NetStructCode_Bytes, Item);
	}
	inline int PackItem(uint8_t* Buffer, int BufLen, const NetStruct_Bytes& Item) { return PackItem(Buffer, BufLen, &Item); }
	inline int PackItem(uint8_t* Buffer, int BufLen, const char* Item) {
		return NetStruct_PackItem(Buffer, BufLen, NetStructCode_String, Item);
	}
	inline int PackItem(uint8_t* Buffer, int BufLen, const int& Item) {
		return NetStruct_PackItem(Buffer, BufLen, NetStructCode_Int, &Item);
	}
	inline int PackItem(uint8_t* Buffer, int BufLen, const int64_t& Item) {
		return NetStruct_PackItem(Buffer, BufLen, NetStructCode_Long, &Item);
	}
	inline int PackItem(uint8_t* Buffer, int BufLen, const float& Item) {
		return NetStruct_PackItem(Buffer, BufLen, NetStructCode_Float, &Item);
	}
	inline int PackItem(uint8_t* Buffer, int BufLen, const double& Item) {
		return NetStruct_PackItem(Buffer, BufLen, NetStructCode_Double, &Item);
	}

	inline int _PackFmtBuffer(uint8_t* Buffer, int BufLen) { return 0; }
	template<class T, class ...TArgs>
	inline int _PackFmtBuffer(uint8_t* Buffer, int BufLen, const T& Item, const TArgs& ...Etc)
	{
		int wrote = PackItem(Buffer, BufLen, Item), moar;
		if (wrote <= 0 ||
			(moar = _PackFmtBuffer(Buffer + wrote, BufLen - wrote, Etc...)) < 0)
			return -1;
		return wrote + moar;
	}

	template<class T, class ...TArgs>
	inline int PackFmtBuffer(uint8_t* Buffer, int BufLen, const T& Item, const TArgs& ...Etc) {
		return _PackFmtBuffer(Buffer, BufLen, Item, Etc...);
	}

	// - Returns a length > 0 on success, setting 'out_pBytes' to a new[] array
	template<class T, class ...TArgs>
	inline int Pack(uint8_t** out_pBytes, const T& Item, const TArgs& ...Etc)
	{
		int len = FmtLen(Item, Etc...);
		uint8_t* bytes;

		if (len <= 0)
			return -1;

		bytes = (uint8_t*)_NetStruct_Alloc(len);
		if (PackFmtBuffer(bytes, len, Item, Etc...) <= 0)
			return _NetStruct_Free(bytes), -1;

		*out_pBytes = bytes;
		return len;
	}

	template<class T>
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, T& Item) { return Item._NetStruct_Unpack(Buffer, BufLen); }
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, NetStruct_Bytes* Item) {
		return NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_Bytes, Item);
	}
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, NetStruct_Bytes& Item) { return UnpackItem(Buffer, BufLen, &Item); }
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, const char*& Item) {
		return NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_String, &Item);
	}
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, int& Item) {
		return NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_Int, &Item);
	}
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, int64_t& Item) {
		return NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_Long, &Item);
	}
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, float& Item) {
		return NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_Float, &Item);
	}
	inline int UnpackItem(const uint8_t* Buffer, int BufLen, double& Item) {
		return NetStruct_UnpackItem(Buffer, BufLen, NetStructCode_Double, &Item);
	}

	inline int _UnpackFmt(const uint8_t* Buffer, int BufLen) { return 0; }
	template<class T, class ...TArgs>
	inline int _UnpackFmt(const uint8_t* Buffer, int BufLen, T& Item, TArgs& ...Etc)
	{
		int read = UnpackItem(Buffer, BufLen, Item), moar;
		if (read <= 0 ||
			(moar = _UnpackFmt(Buffer + read, BufLen - read, Etc...)) < 0)
			return -1;
		return read + moar;
	}

	template<class T, class ...TArgs>
	inline int Unpack(const uint8_t* Buffer, int BufLen, T& Item, TArgs& ...Etc) {
		return _UnpackFmt(Buffer, BufLen, Item, Etc...);
	}
}