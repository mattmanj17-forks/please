#ifndef COMMON_MEMORY_H
#define COMMON_MEMORY_H

//~ NOTE(ljre): Functions
static inline int32 BitCtz64(uint64 i);
static inline int32 BitCtz32(uint32 i);
static inline int32 BitCtz16(uint16 i);
static inline int32 BitCtz8(uint8 i);

static inline int32 BitClz64(uint64 i);
static inline int32 BitClz32(uint32 i);
static inline int32 BitClz16(uint16 i);
static inline int32 BitClz8(uint8 i);

static inline int32 PopCnt64(uint64 x);
static inline int32 PopCnt32(uint32 x);
static inline int32 PopCnt16(uint16 x);
static inline int32 PopCnt8(uint8 x);

static inline uint64 ByteSwap64(uint64 x);
static inline uint32 ByteSwap32(uint32 x);
static inline uint16 ByteSwap16(uint16 x);

static inline void MemoryCopyX16(void* restrict dst, const void* restrict src);

static inline uintsize MemoryStrlen(const char* restrict cstr);
static inline uintsize MemoryStrnlen(const char* restrict cstr, uintsize limit);
static inline int32 MemoryStrcmp(const char* left, const char* right);
static inline char* MemoryStrstr(const char* left, const char* right);
static inline const void* MemoryFindByte(const void* buffer, uint8 byte, uintsize size);
static inline void* MemoryZero(void* restrict dst, uintsize size);
static inline void* MemoryZeroSafe(void* restrict dst, uintsize size);

static inline void* MemoryCopy(void* restrict dst, const void* restrict src, uintsize size);
static inline void* MemoryMove(void* dst, const void* src, uintsize size);
static inline void* MemorySet(void* restrict dst, uint8 byte, uintsize size);
static inline int32 MemoryCompare(const void* left_, const void* right_, uintsize size);

//-
#if defined(_MSC_VER) && !defined(__clang__)
#	pragma optimize("", off)
#endif
static inline void*
MemoryZeroSafe(void* restrict dst, uintsize size)
{
	MemoryZero(dst, size);
#if defined(__GNUC__) || defined(__clang__)
	__asm__ __volatile__ ("" : "+X"(dst) :: "memory");
#elif defined(_MSC_VER)
	_ReadWriteBarrier();
#endif
	return dst;
}
#if defined(_MSC_VER) && !defined(__clang__)
#	pragma optimize("", on)
#endif

//-
static inline int32
GenericPopCnt64(uint64 x)
{
	x -= (x >> 1) & 0x5555555555555555u;
	x = (x & 0x3333333333333333u) + ((x >> 2) & 0x3333333333333333u);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fu;
	return (x * 0x0101010101010101u) >> 56;
}

static inline int32
GenericPopCnt32(uint32 x)
{
	x -= (x >> 1) & 0x55555555u;
	x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
	x = (x + (x >> 4)) & 0x0f0f0f0fu;
	return (x * 0x01010101u) >> 24;
}

static inline int32
GenericPopCnt16(uint16 x)
{ return GenericPopCnt32(x); }

static inline int32
GenericPopCnt8(uint8 x)
{ return GenericPopCnt32(x); }

static inline uint16
GenericByteSwap16(uint16 x)
{ return (uint16)((x >> 8) | (x << 8)); }

static inline uint32
GenericByteSwap32(uint32 x)
{
	uint32 result;
	result = (x << 24) | (x >> 24) | (x >> 8 & 0xff00) | (x << 8 & 0xff0000);
	return result;
}

static inline uint64
GenericByteSwap64(uint64 x)
{
	uint64 result;
	result  = (uint64)GenericByteSwap32(x >> 32);
	result |= (uint64)GenericByteSwap32((uint32)x) << 32;
	return result;
}

//~ NOTE(ljre): X86
#if defined(CONFIG_ARCH_X86FAMILY)
#include <immintrin.h>

static inline int32
BitCtz64(uint64 i)
{
	int32 result;
	
	if (i == 0)
		result = sizeof(i)*8;
	else
	{
#if defined(__GNUC__) || defined(__clang__)
		result = __builtin_ctzll(i);
#elif defined(_MSC_VER)
		_BitScanForward64((unsigned long*)&result, i);
#endif
	}
	
	return result;
}

static inline int32
BitCtz32(uint32 i)
{
	int32 result;
	
	if (i == 0)
		result = sizeof(i)*8;
	else
	{
#if defined(__GNUC__) || defined(__clang__)
		result = __builtin_ctz(i);
#elif defined(_MSC_VER)
		_BitScanForward((unsigned long*)&result, i);
#endif
	}
	
	return result;
}

static inline int32
BitCtz16(uint16 i)
{ return i == 0 ? sizeof(i)*8 : BitCtz32(i); }

static inline int32
BitCtz8(uint8 i)
{ return i == 0 ? sizeof(i)*8 : BitCtz32(i); }

static inline int32
BitClz64(uint64 i)
{
	int32 result;
	
	if (i == 0)
		result = sizeof(i)*8;
	else
	{
#if defined(__GNUC__) || defined(__clang__)
		result = __builtin_clzll(i);
#elif defined(_MSC_VER)
		_BitScanReverse((unsigned long*)&result, i);
		result = 63 - result;
#endif
	}
	
	return result;
}

static inline int32
BitClz32(uint32 i)
{
	int32 result;
	
	if (i == 0)
		result = sizeof(i)*8;
	else
	{
#if defined(__GNUC__) || defined(__clang__)
		result = __builtin_clz(i);
#elif defined(_MSC_VER)
		_BitScanReverse((unsigned long*)&result, i);
		result = 31 - result;
#endif
	}
	
	return result;
}

static inline int32
BitClz16(uint16 i)
{ return i == 0 ? sizeof(i)*8 : BitClz32(i) - 16; }

static inline int32
BitClz8(uint8 i)
{ return i == 0 ? sizeof(i)*8 : BitClz32(i) - 24; }

#ifdef __POPCNT__
static inline int32
PopCnt64(uint64 x)
{ return (int32)_mm_popcnt_u64(x); }

static inline int32
PopCnt32(uint32 x)
{ return (int32)_mm_popcnt_u32(x); }

static inline int32
PopCnt16(uint16 x)
{ return (int32)_mm_popcnt_u32(x); }

static inline int32
PopCnt8(uint8 x)
{ return (int32)_mm_popcnt_u32(x); }
#else //__POPCNT__
static inline int32
PopCnt64(uint64 x)
{ return GenericPopCnt64(x); }

static inline int32
PopCnt32(uint32 x)
{ return GenericPopCnt32(x); }

static inline int32
PopCnt16(uint16 x)
{ return GenericPopCnt16(x); }

static inline int32
PopCnt8(uint8 x)
{ return GenericPopCnt8(x); }
#endif //__POPCNT__

//- ByteSwap
static inline uint16
ByteSwap16(uint16 x)
{ return (uint16)((x >> 8) | (x << 8)); }

static inline uint32
ByteSwap32(uint32 x)
{
	uint32 result;
	
#if defined(__GNUC__) || defined(__clang__)
	result = __builtin_bswap32(x);
#elif defined (_MSC_VER)
	extern unsigned long _byteswap_ulong(unsigned long);
	
	result = _byteswap_ulong(x);
#endif
	
	return result;
}

static inline uint64
ByteSwap64(uint64 x)
{
	uint64 result;
	
#if defined(__GNUC__) || defined(__clang__)
	result = __builtin_bswap64(x);
#elif defined (_MSC_VER)
	extern unsigned __int64 _byteswap_uint64(unsigned __int64);
	
	result = _byteswap_uint64(x);
#endif
	
	return result;
}

static inline void
MemoryCopyX16(void* restrict dst, const void* restrict src)
{ _mm_store_ps((float32*)dst, _mm_load_ps((const float32*)src)); }

//- CRT memcpy, memmove, memset & memcmp functions
#ifdef CONFIG_DONT_USE_CRT

// NOTE(ljre): Loop vectorization when using clang is disabled.
//             this thing is already vectorized, though it likes to vectorize the 1-by-1 bits still.
//
//             GCC only does this at -O3, which we don't care about. MSVC is ok.

static inline void*
MemoryCopy(void* restrict dst, const void* restrict src, uintsize size)
{
	Trace();
	
	uint8* restrict d = (uint8*)dst;
	const uint8* restrict s = (const uint8*)src;
	
	if (size >= 32)
	{
		// NOTE(ljre): Simply use 'rep movsb'.
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
		if (Unlikely(size >= 4096))
		{
#	if defined(__clang__) || defined(__GNUC__)
			__asm__ __volatile__ (
				"rep movsb"
				: "+D"(d), "+S"(s), "+c"(size)
				:: "memory");
#	else
			__movsb(d, s, size);
#	endif
			return dst;
		}
#endif
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size >= 128)
		{
			size -= 128;
			_mm_storeu_si128((__m128i*)(d+size+  0), _mm_loadu_si128((__m128i*)(s+size+  0)));
			_mm_storeu_si128((__m128i*)(d+size+ 16), _mm_loadu_si128((__m128i*)(s+size+ 16)));
			_mm_storeu_si128((__m128i*)(d+size+ 32), _mm_loadu_si128((__m128i*)(s+size+ 32)));
			_mm_storeu_si128((__m128i*)(d+size+ 48), _mm_loadu_si128((__m128i*)(s+size+ 48)));
			_mm_storeu_si128((__m128i*)(d+size+ 64), _mm_loadu_si128((__m128i*)(s+size+ 64)));
			_mm_storeu_si128((__m128i*)(d+size+ 80), _mm_loadu_si128((__m128i*)(s+size+ 80)));
			_mm_storeu_si128((__m128i*)(d+size+ 96), _mm_loadu_si128((__m128i*)(s+size+ 96)));
			_mm_storeu_si128((__m128i*)(d+size+112), _mm_loadu_si128((__m128i*)(s+size+112)));
		}
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size >= 32)
		{
			size -= 32;
			_mm_storeu_si128((__m128i*)(d+size+ 0), _mm_loadu_si128((__m128i*)(s+size+ 0)));
			_mm_storeu_si128((__m128i*)(d+size+16), _mm_loadu_si128((__m128i*)(s+size+16)));
		}
	}
	
	switch (size)
	{
		case 0: break;
		
		case 31:        _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case 15:        *(uint64*)d = *(uint64*)s; d += 8; s += 8;
		case  7: lbl_7: *(uint32*)d = *(uint32*)s; d += 4; s += 4;
		case  3: lbl_3: *(uint16*)d = *(uint16*)s; d += 2; s += 2;
		case  1: lbl_1: *d = *s; break;
		
		case 30:        _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case 14:        *(uint64*)d = *(uint64*)s; d += 8; s += 8;
		case  6: lbl_6: *(uint32*)d = *(uint32*)s; d += 4; s += 4;
		case  2: lbl_2: *(uint16*)d = *(uint16*)s; d += 2; s += 2; break;
		
		case 29:        _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case 13:        *(uint64*)d = *(uint64*)s; d += 8; s += 8;
		case  5: lbl_5: *(uint32*)d = *(uint32*)s; d += 4; s += 4; goto lbl_1;
		
		case 28:        _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case 12:        *(uint64*)d = *(uint64*)s; d += 8; s += 8;
		case  4: lbl_4: *(uint32*)d = *(uint32*)s; break;
		
		case 27: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case 11: *(uint64*)d = *(uint64*)s; d += 8; s += 8; goto lbl_3;
		
		case 26: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case 10: *(uint64*)d = *(uint64*)s; d += 8; s += 8; goto lbl_2;
		
		case 25: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case  9: *(uint64*)d = *(uint64*)s; d += 8; s += 8; goto lbl_1;
		
		case 24: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16;
		case  8: *(uint64*)d = *(uint64*)s; break;
		
		case 23: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_7;
		case 22: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_6;
		case 21: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_5;
		case 20: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_4;
		case 19: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_3;
		case 18: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_2;
		case 17: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); d += 16; s += 16; goto lbl_1;
		case 16: _mm_storeu_si128((__m128i*)d, _mm_loadu_si128((__m128i*)s)); break;
	}
	
	return dst;
}

static inline void*
MemoryMove(void* dst, const void* src, uintsize size)
{
	Trace();
	
	uint8* d = (uint8*)dst;
	const uint8* s = (const uint8*)src;
	uintsize diff;
	{
		const intsize diff_signed = d - s;
		diff = diff_signed < 0 ? -diff_signed : diff_signed;
	}
	
	if (size == 0)
		return dst;
	
	if (diff > size)
		return MemoryCopy(dst, src, size);
	
	if (d <= s)
	{
		// NOTE(ljre): Forward copy.
		
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
		if (Unlikely(size >= 4096))
		{
#	if defined(__clang__) || defined(__GNUC__)
			__asm__ __volatile__("rep movsb"
				:"+D"(d), "+S"(s), "+c"(size)
				:: "memory");
#	else
			__movsb(d, s, size);
#	endif
			return dst;
		}
#endif
		
		if (diff < 8)
			goto inc_by_one;
		if (diff < 32)
			goto inc_by_qword;
		if (diff < 128)
			goto inc_by_xmm2;
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		do
		{
			_mm_storeu_si128((__m128i*)(d+  0), _mm_loadu_si128((__m128i*)(s+  0)));
			_mm_storeu_si128((__m128i*)(d+ 16), _mm_loadu_si128((__m128i*)(s+ 16)));
			_mm_storeu_si128((__m128i*)(d+ 32), _mm_loadu_si128((__m128i*)(s+ 32)));
			_mm_storeu_si128((__m128i*)(d+ 48), _mm_loadu_si128((__m128i*)(s+ 48)));
			_mm_storeu_si128((__m128i*)(d+ 64), _mm_loadu_si128((__m128i*)(s+ 64)));
			_mm_storeu_si128((__m128i*)(d+ 80), _mm_loadu_si128((__m128i*)(s+ 80)));
			_mm_storeu_si128((__m128i*)(d+ 96), _mm_loadu_si128((__m128i*)(s+ 96)));
			_mm_storeu_si128((__m128i*)(d+112), _mm_loadu_si128((__m128i*)(s+112)));
			
			d += 128;
			s += 128;
			size -= 128;
		}
		while (size >= 128);
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size >= 32) inc_by_xmm2:
		{
			_mm_storeu_si128((__m128i*)(d+ 0), _mm_loadu_si128((__m128i*)(s+ 0)));
			_mm_storeu_si128((__m128i*)(d+16), _mm_loadu_si128((__m128i*)(s+16)));
			
			d += 32;
			s += 32;
			size -= 32;
		}
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size >= 8) inc_by_qword:
		{
			*(uint64*)d = *(uint64*)s;
			
			d += 8;
			s += 8;
			size -= 8;
		}
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size) inc_by_one:
		{
			size -= 1;
			*d++ = *s++;
		}
	}
	else
	{
		// NOTE(ljre): Backwards copy.
		
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
		if (Unlikely(size >= 4096))
		{
			d += size-1;
			s += size-1;
#	if defined(__clang__) || defined(__GNUC__)
			__asm__ __volatile__("std\n"
				"rep movsb\n"
				"cld"
				:"+D"(d), "+S"(s), "+c"(size)
				:: "memory");
#	else
			// TODO(ljre): maybe reconsider this? I couldn't find a way for MSVC to directly
			//     generate 'std' & 'cld' instructions. (SeT Direction flag & CLear Direction flag)
			__writeeflags(__readeflags() | 0x0400);
			__movsb(d, s, size);
			__writeeflags(__readeflags() & ~(uint64)0x0400);
#	endif
			return dst;
		}
#endif
		
		if (diff < 8)
			goto dec_by_one;
		if (diff < 32)
			goto dec_by_qword;
		if (diff < 128)
			goto dec_by_xmm2;
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		do
		{
			size -= 128;
			_mm_storeu_si128((__m128i*)(d+size+  0), _mm_loadu_si128((__m128i*)(s+size+  0)));
			_mm_storeu_si128((__m128i*)(d+size+ 16), _mm_loadu_si128((__m128i*)(s+size+ 16)));
			_mm_storeu_si128((__m128i*)(d+size+ 32), _mm_loadu_si128((__m128i*)(s+size+ 32)));
			_mm_storeu_si128((__m128i*)(d+size+ 48), _mm_loadu_si128((__m128i*)(s+size+ 48)));
			_mm_storeu_si128((__m128i*)(d+size+ 64), _mm_loadu_si128((__m128i*)(s+size+ 64)));
			_mm_storeu_si128((__m128i*)(d+size+ 80), _mm_loadu_si128((__m128i*)(s+size+ 80)));
			_mm_storeu_si128((__m128i*)(d+size+ 96), _mm_loadu_si128((__m128i*)(s+size+ 96)));
			_mm_storeu_si128((__m128i*)(d+size+112), _mm_loadu_si128((__m128i*)(s+size+112)));
		}
		while (size >= 128);
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size >= 32) dec_by_xmm2:
		{
			size -= 32;
			_mm_storeu_si128((__m128i*)(d+size+ 0), _mm_loadu_si128((__m128i*)(s+size+ 0)));
			_mm_storeu_si128((__m128i*)(d+size+16), _mm_loadu_si128((__m128i*)(s+size+16)));
		}
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size >= 8) dec_by_qword:
		{
			size -= 8;
			*(uint64*)(d+size) = *(uint64*)(s+size);
		}
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (size) dec_by_one:
		{
			size -= 1;
			d[size] = s[size];
		}
	}
	
	return dst;
}

static inline void*
MemorySet(void* restrict dst, uint8 byte, uintsize size)
{
	Trace();
	
	uint8* restrict d = (uint8*)dst;
	uint64 qword;
	__m128i xmm;
	
	if (Unlikely(size == 0))
		return dst;
	if (size < 8)
		goto one_by_one;
	
	qword = byte * 0x0101010101010101;
	if (size < 32)
		goto qword_by_qword;
	
	xmm = _mm_set1_epi64x(qword);
	if (size < 128)
		goto xmm2_by_xmm2;
	
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
	if (Unlikely(size >= 2048))
	{
#	if defined(__clang__) || defined(__GNUC__)
		__asm__ __volatile__("rep stosb"
			:"+D"(d), "+a"(byte), "+c"(size)
			:: "memory");
#	elif defined(_MSC_VER)
		__stosb(d, byte, size);
#	endif
		
		return dst;
	}
#endif
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	do
	{
		size -= 128;
		_mm_storeu_si128((__m128i*)(d+size+0 ), xmm);
		_mm_storeu_si128((__m128i*)(d+size+16), xmm);
		_mm_storeu_si128((__m128i*)(d+size+32), xmm);
		_mm_storeu_si128((__m128i*)(d+size+48), xmm);
		_mm_storeu_si128((__m128i*)(d+size+64), xmm);
		_mm_storeu_si128((__m128i*)(d+size+80), xmm);
		_mm_storeu_si128((__m128i*)(d+size+96), xmm);
		_mm_storeu_si128((__m128i*)(d+size+112), xmm);
	}
	while (size >= 128);
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 32) xmm2_by_xmm2:
	{
		size -= 32;
		_mm_storeu_si128((__m128i*)(d+size+0 ), xmm);
		_mm_storeu_si128((__m128i*)(d+size+16), xmm);
	}
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 8) qword_by_qword:
	{
		size -= 8;
		*(uint64*)(d+size) = qword;
	}
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 1) one_by_one:
	{
		size -= 1;
		d[size] = byte;
	}
	
	return dst;
}

static inline int32
MemoryCompare(const void* left_, const void* right_, uintsize size)
{
	Trace();
	
	const uint8* left = (const uint8*)left_;
	const uint8* right = (const uint8*)right_;
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 16)
	{
		__m128i ldata, rdata;
		
		ldata = _mm_loadu_si128((const __m128i*)left);
		rdata = _mm_loadu_si128((const __m128i*)right);
		
		int32 cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(ldata, rdata));
		cmp = ~cmp & 0xffff;
		
		if (Unlikely(cmp != 0))
		{
			union
			{
				__m128i reg;
				int8 bytes[16];
			} diff = { _mm_sub_epi8(ldata, rdata) };
			
			return diff.bytes[BitCtz32(cmp)] < 0 ? -1 : 1;
		}
		
		size -= 16;
		left += 16;
		right += 16;
	}
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size --> 0)
	{
		if (Unlikely(*left != *right))
			return (*left - *right < 0) ? -1 : 1;
		
		++left;
		++right;
	}
	
	return 0;
}

static inline uintsize
MemoryStrlen(const char* restrict cstr)
{
	Trace();
	const char* begin = cstr;
	
	while (*cstr)
		++cstr;
	
	return cstr - begin;
}

static inline uintsize
MemoryStrnlen(const char* restrict cstr, uintsize limit)
{
	Trace();
	const char* begin = cstr;
	
	while (limit-- && *cstr)
		++cstr;
	
	return cstr - begin;
}

static inline int32
MemoryStrcmp(const char* left, const char* right)
{
	Trace();
	
	for (;;)
	{
		if (*left != *right)
			return *left - *right;
		if (!*left)
			return 0;
		++left;
		++right;
	}
}

static inline char*
MemoryStrstr(const char* left, const char* right)
{
	Trace();
	
	for (; *left; ++left)
	{
		const char* it_left = left;
		const char* it_right = right;
		while (*it_left == *it_right)
		{
			if (!*it_left)
				return (char*)it_left;
			++it_left;
			++it_right;
		}
	}
	
	return NULL;
}

static inline const void*
MemoryFindByte(const void* buffer, uint8 byte, uintsize size)
{
	Trace();
	
	const uint8* buf = (const uint8*)buffer;
	const uint8* const end = buf + size;
	
	if (Unlikely(size == 0))
		return NULL;
	if (size < 16)
		goto by_byte;
	
	// NOTE(ljre): XMM by XMM
	{
		__m128i mask = _mm_set1_epi8(byte);
		
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
		while (buf + 16 < end)
		{
			__m128i data = _mm_loadu_si128((const __m128i*)buf);
			int32 cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(data, mask));
			
			if (Unlikely(cmp != 0))
				return buf + BitCtz16((uint16)cmp);
			
			buf += 16;
		}
	}
	
	// NOTE(ljre): Byte by byte
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (buf < end) by_byte:
	{
		if (Unlikely(*buf == byte))
			return buf;
		
		++buf;
	}
	
	// NOTE(ljre): byte wasn't found.
	return NULL;
}

static inline void*
MemoryZero(void* restrict dst, uintsize size)
{
	Trace();
	
	uint8* restrict d = (uint8*)dst;
	__m128 mzero = _mm_setzero_ps();
	
	if (Unlikely(size == 0))
		return dst;
	if (size < 8)
		goto by_byte;
	if (size < 32)
		goto by_qword;
	if (size < 128)
		goto by_xmm2;
	
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
	if (Unlikely(size >= 2048))
	{
#	if defined(__clang__) || defined(__GNUC__)
		uint8 byte = 0;
		__asm__ __volatile__("rep stosb"
			:"+D"(d), "+a"(byte), "+c"(size)
			:: "memory");
#	elif defined(_MSC_VER)
		__stosb(d, 0, size);
#	endif
		
		return dst;
	}
#endif
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	do
	{
		_mm_storeu_ps((float*)(d+  0), mzero);
		_mm_storeu_ps((float*)(d+ 16), mzero);
		_mm_storeu_ps((float*)(d+ 32), mzero);
		_mm_storeu_ps((float*)(d+ 48), mzero);
		_mm_storeu_ps((float*)(d+ 64), mzero);
		_mm_storeu_ps((float*)(d+ 80), mzero);
		_mm_storeu_ps((float*)(d+ 96), mzero);
		_mm_storeu_ps((float*)(d+112), mzero);
		size -= 128;
		d += 128;
	}
	while (size >= 128);
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 32) by_xmm2:
	{
		_mm_storeu_ps((float*)(d+ 0), mzero);
		_mm_storeu_ps((float*)(d+16), mzero);
		size -= 32;
		d += 32;
	}
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 8) by_qword:
	{
		*(uint64*)d = 0;
		size -= 8;
		d += 8;
	}
	
#ifdef __clang__
#	pragma clang loop unroll(disable)
#endif
	while (size >= 1) by_byte:
	{
		*d = 0;
		size -= 1;
		d += 1;
	}
	
	return dst;
}

#	endif

#elif defined(CONFIG_ARCH_ARMFAMILY)
//~ NOTE(ljre): ARM
#ifdef CONFIG_ARCH_AARCH64
#	include <arm_neon.h>
#endif
#ifdef CONFIG_DONT_USE_CRT
#	error "ARM implementation needs CRT"
#endif

static inline int32 BitCtz64(uint64 i) { return __builtin_ctzll(i); }
static inline int32 BitCtz32(uint32 i) { return __builtin_ctz(i); }
static inline int32 BitCtz16(uint16 i) { return __builtin_ctz(i); }
static inline int32 BitCtz8(uint8 i) { return __builtin_ctz(i); }

static inline int32 BitClz64(uint64 i) { return __builtin_clzll(i); }
static inline int32 BitClz32(uint32 i) { return __builtin_clz(i); }
static inline int32 BitClz16(uint16 i) { return __builtin_clz(i)-16; }
static inline int32 BitClz8(uint8 i) { return __builtin_clz(i)-24; }

#ifdef CONFIG_ARCH_AARCH64
static inline int32 PopCnt64(uint64 x) { return __builtin_popcountll(x); }
static inline int32 PopCnt32(uint32 x) { return __builtin_popcount(x); }
static inline int32 PopCnt16(uint16 x) { return __builtin_popcount(x); }
static inline int32 PopCnt8(uint8 x) { return __builtin_popcount(x); }
#else //CONFIG_ARCH_AARCH64
static inline int32 PopCnt64(uint64 x) { return GenericPopCnt64(x); }
static inline int32 PopCnt32(uint32 x) { return GenericPopCnt32(x); }
static inline int32 PopCnt16(uint16 x) { return GenericPopCnt16(x); }
static inline int32 PopCnt8(uint8 x) { return GenericPopCnt8(x); }
#endif //CONFIG_ARCH_AARCH64

static inline uint64 ByteSwap64(uint64 x) { return __builtin_bswap64(x); }
static inline uint32 ByteSwap32(uint32 x) { return __builtin_bswap32(x); }
static inline uint16 ByteSwap16(uint16 x) { return __builtin_bswap16(x); }

#ifdef CONFIG_ARCH_AARCH64
static inline void
MemoryCopyX16(void* restrict dst, const void* restrict src)
{ vst1q_u64((uint64*)dst, vld1q_u64((const uint64*)src)); }
#else //CONFIG_ARCH_AARCH64
static inline void
MemoryCopyX16(void* restrict dst, const void* restrict src)
{
	((uint32*)dst)[0] = ((const uint32*)src)[0];
	((uint32*)dst)[1] = ((const uint32*)src)[1];
	((uint32*)dst)[2] = ((const uint32*)src)[2];
	((uint32*)dst)[3] = ((const uint32*)src)[3];
}
#endif //CONFIG_ARCH_AARCH64

#else //CONFIG_ARCH_*
#	error "Unknown architecture"
#endif //CONFIG_ARCH_*

//~ NOTE(ljre): CRT polyfill
#ifndef CONFIG_DONT_USE_CRT
#include <string.h>

static inline void*
MemoryCopy(void* restrict dst, const void* restrict src, uintsize size)
{ Trace(); return memcpy(dst, src, size); }

static inline void*
MemoryMove(void* dst, const void* src, uintsize size)
{ Trace(); return memmove(dst, src, size); }

static inline void*
MemorySet(void* restrict dst, uint8 byte, uintsize size)
{ Trace(); return memset(dst, byte, size); }

static inline int32
MemoryCompare(const void* left_, const void* right_, uintsize size)
{ Trace(); return memcmp(left_, right_, size); }

static inline uintsize
MemoryStrlen(const char* restrict cstr)
{ Trace(); return strlen(cstr); }

static inline uintsize
MemoryStrnlen(const char* restrict cstr, uintsize limit)
{ Trace(); return strnlen(cstr, limit); }

static inline int32
MemoryStrcmp(const char* left, const char* right)
{ Trace(); return strcmp(left, right); }

static inline char*
MemoryStrstr(const char* left, const char* right)
{ Trace(); return strstr(left, right); }

static inline const void*
MemoryFindByte(const void* buffer, uint8 byte, uintsize size)
{ Trace(); return memchr(buffer, byte, size); }

static inline void*
MemoryZero(void* restrict dst, uintsize size)
{ Trace(); return memset(dst, 0, size); }

#endif //CONFIG_DONT_USE_CRT

#endif // COMMON_MEMORY_H
