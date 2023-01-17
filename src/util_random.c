// NOTE(ljre): this is a xorshift128+
// https://prng.di.unimi.it/

struct UtilRandom_State
{
	uint64 seed[2];
}
typedef UtilRandom_State;

//~ Functions
static inline uint64
BitRotateLeft(const uint64 x, int32 k)
{
	return (x << k) | (x >> (64 - k));
}

//~ Internal API
// Generates a random number between 0 and UINT64_MAX
static uint64
UtilRandom_U64(UtilRandom_State* state)
{
	const uint64 s0 = state->seed[0];
	uint64 s1 = state->seed[1];
	const uint64 result = BitRotateLeft(s0 + s1, 17) + s0;
	
	s1 ^= s0;
	state->seed[0] = BitRotateLeft(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	state->seed[1] = BitRotateLeft(s1, 28); // c
	
	return result;
}

static void
UtilRandom_Init(UtilRandom_State* state)
{
	static const uint64 jump[] = { 0xdf900294d8f554a5, 0x170865df4b3201fc };
	
	state->seed[0] = state->seed[1] = Platform_CurrentPosixTime();
	
	uint64 s0 = 0;
	uint64 s1 = 0;
	
	for (int32  i = 0; i < ArrayLength(jump); ++i)
	{
		for (int32  b = 0; b < 64; ++b)
		{
			if (jump[i] & (uint64)1 << b)
			{
				s0 ^= state->seed[0];
				s1 ^= state->seed[1];
			}
			UtilRandom_U64(state);
		}
	}
	
	state->seed[0] = s0;
	state->seed[1] = s1;
}

// NOTE(ljre): good enough
static float64
UtilRandom_F64(UtilRandom_State* state)
{
	uint64 roll = UtilRandom_U64(state);
	union
	{
		uint64 i;
		float64 f;
	} cvt;
	
	cvt.i = 0x3FF0000000000000;
	cvt.i |= roll >> (64 - 52);
	
	return cvt.f - 1.0;
}

static float32
UtilRandom_F32Range(UtilRandom_State* state, float32 start, float32 end)
{
	uint64 roll = UtilRandom_U64(state);
	union
	{
		uint32 i;
		float32 f;
	} cvt;
	
	cvt.i = 0x3F800000;
	cvt.i |= roll >> (64 - 23);
	
	return (cvt.f - 1.0f) * (end - start) + start;
}

static uint32
UtilRandom_U32(UtilRandom_State* state)
{
	uint64 result = UtilRandom_U64(state);
	return (uint32)(result >> 32); // the higher bits are more random.
}