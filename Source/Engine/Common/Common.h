#pragma once


#ifdef _MSC_VER
	#pragma warning(disable: 4018) // signed/unsigned dismatch
	#pragma warning(disable: 4244) // unsigned long to byte
#endif


/*
#ifdef NULL
    #undef NULL
    #define NULL nullptr
#endif
*/

#ifdef _WIN32
	#ifdef ENGINE_EXPORTS
		#define ENGINE_API __declspec(dllexport)
	#else
		#define ENGINE_API __declspec(dllimport)
	#endif
#else
	#define ENGINE_API __attribute__((visibility("default")))
#endif

#ifdef _MSC_VER
	#define POWER_INLINE __forceinline
#else
	#define POWER_INLINE inline
#endif

/*
Copy memory, if error, returns NULL.
*/
POWER_INLINE void* copyMemory(void* pvsrc, unsigned int size) 
{
	void* p = malloc(size);
	if (p == NULL) return NULL;

	memcpy(p, pvsrc, size);

	return p;
}
/*
Grows or shrink memory
*/
POWER_INLINE void resizeMemory(void** ppsrc, unsigned int newSize)
{
	void* p = realloc(ppsrc, newSize);

	if (p == NULL) return;

	*ppsrc = p;
}

POWER_INLINE const char* toCString(std::string& str)
{
	return str.c_str();
}

POWER_INLINE unsigned int _swap32(unsigned int i)
{
	return (i << 24) | ((i & 0x0000FF00) << 8) | ((i & 0x00FF0000) >> 8) | (i >> 24);
}

POWER_INLINE  short _swap16( short i )
{
	return (i >> 8) | (i << 8);
}

POWER_INLINE float _swapFloat( float f ) 
{
	int i = _swap32(*(int*)&f);
	return *(float*)&i;
}

POWER_INLINE long long _swap64(long long i)
{
	unsigned long long u = (unsigned long long)i;
	u = ((u & 0x00000000FFFFFFFFull) << 32) | ((u & 0xFFFFFFFF00000000ull) >> 32);
	u = ((u & 0x0000FFFF0000FFFFull) << 16) | ((u & 0xFFFF0000FFFF0000ull) >> 16);
	u = ((u & 0x00FF00FF00FF00FFull) << 8)  | ((u & 0xFF00FF00FF00FF00ull) >> 8);
	return (long long)u;
}


POWER_INLINE short LittleShort(short i)
{
	return _swap16(i);
}


