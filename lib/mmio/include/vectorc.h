
#ifndef __VECTORC
#define restrict
#define alignvalue(a)
#define __hint__(a)
#define div12 /
#define sqrt12 sqrt
#else
#define sqrt12 __hint__((precision(12))) sqrt
#define div12 __hint__((precision(12))) /
#define alignvalue(a) __declspec (alignedvalue (a))
#endif

#if !defined (__VECTORC) && !defined (__ICL)
#define align(a)
#else
#define align(a) __declspec (align (a))
#endif


//#ifdef __MMX__
//#define P_INTEL
//#endif
