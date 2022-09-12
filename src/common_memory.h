#ifndef COMMON_MEMORY_H
#define COMMON_MEMORY_H

#include <immintrin.h>

internal inline int32
BitCtz(int32 i)
{
	Assume(i != 0);
	int32 result;
	
#if defined(__GNUC__) || defined(__clang__)
	result = __builtin_ctz(i);
#elif defined(_MSC_VER)
	_BitScanForward(&result, i);
#else
	result = 0;
	
	while ((i & 1<<result) == 0)
		++result;
#endif
	
	return result;
}

internal inline int32
BitClz(int32 i)
{
	Assume(i != 0);
	int32 result;
	
#if defined(__GNUC__) || defined(__clang__)
	result = __builtin_clz(i);
#elif defined(_MSC_VER)
	_BitScanReverse(&result, i);
	result = 31 - result;
#else
	result = 0;
	
	while ((i & 1<<(31-result)) == 0)
		++result;
#endif
	
	return result;
}

// NOTE(ljre): https://en.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
internal inline int32
PopCnt64(uint64 x)
{
	x -= (x >> 1) & 0x5555555555555555u;
	x = (x & 0x3333333333333333u) + ((x >> 2) & 0x3333333333333333u);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fu;
	return (x * 0x0101010101010101u) >> 56;
}

internal inline int32
PopCnt32(uint32 x)
{
	x -= (x >> 1) & 0x55555555u;
	x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
	x = (x + (x >> 4)) & 0x0f0f0f0fu;
	return (x * 0x01010101u) >> 24;
}

internal inline int32
PopCnt16(uint16 x)
{
	return PopCnt32(x);
}

internal inline uint16
ByteSwap16(uint16 x)
{
	return (uint16)((x >> 8) | (x << 8));
}

internal inline uint32
ByteSwap32(uint32 x)
{
	uint32 result;
	
#if defined(__GNUC__) || defined(__clang__)
	result = __builtin_bswap32(x);
#elif defined (_MSC_VER)
	extern unsigned long _byteswap_ulong(unsigned long);
	
	result = _byteswap_ulong(x);
#else
	result = (x << 24) | (x >> 24) | (x >> 8 & 0xff00) | (x << 8 & 0xff0000);
#endif
	
	return result;
}

internal inline uint64
ByteSwap64(uint64 x)
{
	uint64 result;
	
#if defined(__GNUC__) || defined(__clang__)
	result = __builtin_bswap64(x);
#elif defined (_MSC_VER)
	extern unsigned __int64 _byteswap_uint64(unsigned __int64);
	
	result = _byteswap_uint64(x);
#else
	union
	{
		uint64 v;
		uint8 arr[8];
	}
	r = { .v = x };
	uint8 tmp;
	
	tmp = r.arr[0]; r.arr[0] = r.arr[7]; r.arr[7] = tmp;
	tmp = r.arr[1]; r.arr[1] = r.arr[6]; r.arr[6] = tmp;
	tmp = r.arr[2]; r.arr[2] = r.arr[5]; r.arr[5] = tmp;
	tmp = r.arr[3]; r.arr[3] = r.arr[4]; r.arr[4] = tmp;
	
	result = r.v;
#endif
	
	return result;
}

#ifdef COMMON_DONT_USE_CRT

// NOTE(ljre): Loop vectorization when using clang is disabled.
//             this thing is already vectorized, though it likes to vectorize the 1-by-1 bits still.
//
//             GCC only does this at -O3, which we don't care about. MSVC is ok.

// NOTE(ljre): the *_by_* labels lead directly inside the loop since the (size >= N) condition should
//             already be met.

internal inline void*
MemCopy(void* restrict dst, const void* restrict src, uintsize size)
{
	uint8* restrict d = (uint8*)dst;
	const uint8* restrict s = (const uint8*)src;
	
	if (Unlikely(size == 0))
		return dst;
	if (size < 8)
		goto one_by_one;
	if (size < 32)
		goto qword_by_qword;
	if (size < 128)
		goto xmm2_by_xmm2;
	
	// NOTE(ljre): Simply use 'rep movsb'.
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
	if (Unlikely(size >= 2048))
	{
#   if defined(__clang__) || defined(__GNUC__)
		__asm__ __volatile__ (
			"rep movsb"
			: "+D"(d), "+S"(s), "+c"(size)
			:: "memory");
#   else
		__movsb(d, s, size);
#   endif
		return dst;
	}
#endif
	
	// fallthrough
#ifdef __clang__
#   pragma clang loop vectorize(disable)
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
#   pragma clang loop vectorize(disable)
#endif
	while (size >= 32) xmm2_by_xmm2:
	{
		size -= 32;
		_mm_storeu_si128((__m128i*)(d+size+ 0), _mm_loadu_si128((__m128i*)(s+size+ 0)));
		_mm_storeu_si128((__m128i*)(d+size+16), _mm_loadu_si128((__m128i*)(s+size+16)));
	}
	
#ifdef __clang__
#   pragma clang loop vectorize(disable)
#endif
	while (size >= 8) qword_by_qword:
	{
		size -= 8;
		*(uint64*)(d+size) = *(uint64*)(s+size);
	}
	
#ifdef __clang__
#   pragma clang loop vectorize(disable)
#endif
	while (size) one_by_one:
	{
		size -= 1;
		d[size] = s[size];
	}
	
	return dst;
}

internal inline void*
MemMove(void* dst, const void* src, uintsize size)
{
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
		return MemCopy(dst, src, size);
	
	if (d <= s)
	{
		// NOTE(ljre): Forward copy.
		
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
		if (Unlikely(size >= 2048))
		{
#   if defined(__clang__) || defined(__GNUC__)
			__asm__ __volatile__("rep movsb"
				:"+D"(d), "+S"(s), "+c"(size)
				:: "memory");
#   else
			__movsb(dst, src, size);
#   endif
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
#   pragma clang loop vectorize(disable)
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
#   pragma clang loop vectorize(disable)
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
#   pragma clang loop vectorize(disable)
#endif
		while (size >= 8) inc_by_qword:
		{
			*(uint64*)d = *(uint64*)s;
			
			d += 8;
			s += 8;
			size -= 8;
		}
		
#ifdef __clang__
#   pragma clang loop vectorize(disable)
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
		if (Unlikely(size >= 2048))
		{
#   if defined(__clang__) || defined(__GNUC__)
			__asm__ __volatile__("std\n"
				"rep movsb\n"
				"cld"
				:"+D"(d), "+S"(s), "+c"(size)
				:: "memory");
#   else
			// TODO(ljre): maybe reconsider this? I couldn't find a way for MSVC to directly
			//     generate 'std' & 'cld' instructions. (SeT Direction flag & CLear Direction flag)
			__writeeflags(__readeflags() | 0x0400);
			__movsb(dst, src, size);
			__writeeflags(__readeflags() & ~(uint64)0x0400);
#   endif
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
#   pragma clang loop vectorize(disable)
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
#   pragma clang loop vectorize(disable)
#endif
		while (size >= 32) dec_by_xmm2:
		{
			size -= 32;
			_mm_storeu_si128((__m128i*)(d+size+ 0), _mm_loadu_si128((__m128i*)(s+size+ 0)));
			_mm_storeu_si128((__m128i*)(d+size+16), _mm_loadu_si128((__m128i*)(s+size+16)));
		}
		
#ifdef __clang__
#   pragma clang loop vectorize(disable)
#endif
		while (size >= 8) dec_by_qword:
		{
			size -= 8;
			*(uint64*)(d+size) = *(uint64*)(s+size);
		}
		
#ifdef __clang__
#   pragma clang loop vectorize(disable)
#endif
		while (size) dec_by_one:
		{
			size -= 1;
			d[size] = s[size];
		}
	}
	
	return dst;
}

internal inline void*
MemSet(void* restrict dst, uint8 byte, uintsize size)
{
	uint8* restrict d = (uint8*)dst;
	
#if defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__("rep stosb"
		:"+D"(d), "+a"(byte), "+c"(size)
		:: "memory");
#elif defined(_MSC_VER)
	__stosb(d, byte, size);
#else
	while (size--)
		*d[size] = byte;
#endif
	
	return dst;
}

internal inline int32
MemCmp(const void* left_, const void* right_, uintsize size)
{
	const uint8* left = (const uint8*)left_;
	const uint8* right = (const uint8*)right_;
	
	while (size >= 16)
	{
		__m128i ldata, rdata;
		
		ldata = _mm_loadu_si128((const __m128i_u*)left);
		rdata = _mm_loadu_si128((const __m128i_u*)right);
		
		int32 cmp = _mm_movemask_epi8(_mm_cmpeq_epi8(ldata, rdata));
		cmp = ~cmp & 0xffff;
		
		if (Unlikely(cmp != 0))
		{
			union
			{
				__m128i reg;
				int8 bytes[16];
			} diff = { _mm_sub_epi8(ldata, rdata) };
			
			return diff.bytes[BitCtz(cmp)] < 0 ? -1 : 1;
		}
		
		size -= 16;
		left += 16;
		right += 16;
	}
	
	while (size --> 0)
	{
		if (Unlikely(*left != *right))
			return (*left - *right < 0) ? -1 : 1;
		
		++left;
		++right;
	}
	
	return 0;
}

#else // COMMON_DONT_USE_CRT

#include <string.h>

internal inline void*
MemCopy(void* restrict dst, const void* restrict src, uintsize size)
{ return memcpy(dst, src, size); }

internal inline void*
MemMove(void* dst, const void* src, uintsize size)
{ return memmove(dst, src, size); }

internal inline void*
MemSet(void* restrict dst, uint8 byte, uintsize size)
{ return memset(dst, byte, size); }

internal inline int32
MemCmp(const void* left_, const void* right_, uintsize size)
{ return memcmp(left_, right_, size); }

#endif

#endif // COMMON_MEMORY_H
