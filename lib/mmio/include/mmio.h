#ifndef _MMIO_H_
#define _MMIO_H_

#include <stdio.h>
#include <stdlib.h>
#include "mmtypes.h"

// CPU mode options.  These will force the use of the selected mode, whether that
// cpu is available or not, so make sure you check first!  Speed enhancments are
// present in sample loading and software mixing.

enum
{   CPU_NONE = 0,
    CPU_MMX,
    CPU_3DNOW,
    CPU_AUTODETECT = 0xff
};

extern void _mm_memcpy_quad(void *dst, const void *src, int count);
extern void _mm_memcpy_long(void *dst, const void *src, int count);
extern void _mm_memcpy_word(void *dst, const void *src, int count);


// Miscellaneous Macros -- I should probably put these somewhere else.

// boundschecker macro.  I do a lot of bounds checking ;)
#define _mm_boundscheck(v,a,b)  (v > b) ? b : ((v < a) ? a : v)


// 64 bit integer macros.  These allow me to get pointers to the high
// and low portions of a 64 bit integer.  Used mostly by the Mikmod
// software mixer - but could be useful for other things.

#ifdef MM_BIG_ENDIAN

#define _mm_HI_SLONG(x) ((SLONG *)&x)
#define _mm_LO_SLONG(x) ((SLONG *)&x + 1)
#define _mm_HI_ULONG(x) ((ULONG *)&x)
#define _mm_LO_ULONG(x) ((ULONG *)&x + 1)

#else

#define _mm_HI_SLONG(x) ((SLONG *)&x + 1)
#define _mm_LO_SLONG(x) ((SLONG *)&x)
#define _mm_HI_ULONG(x) ((ULONG *)&x + 1)
#define _mm_LO_ULONG(x) ((ULONG *)&x)

#endif


#ifdef __cplusplus
extern "C" {
#endif


// ===============================================
//  Error handling and logging stuffs - MMERROR.C
// ===============================================

// Generic Error Codes
// -------------------
// Notes:
//  - These should always be relatively generalized. (ie, you probably
//    shouldn't have to add any new errors).  More specific error infor-
//    mation is provided via logging and error strings.
//

enum
{   MMERR_NONE = 0,
    MMERR_INVALID_PARAMS,
    MMERR_OPENING_FILE,
    MMERR_UNSUPPORTED_FILE,
    MMERR_OUT_OF_MEMORY,
    MMERR_END_OF_FILE,
    MMERR_DISK_FULL,
    MMERR_DETECTING_DEVICE,
    MMERR_INITIALIZING_DEVICE,
    MMERR_FATAL
};

// Error / Logging prototypes
// --------------------------

extern void   _mmlog_init(void (*log)(const CHAR *fmt, ... ), void (*logv)(const CHAR *fmt, ... ));
extern void   _mmlog_exit(void);

extern void   (*_mmlog)(const CHAR *fmt, ... );
extern void   (*_mmlogv)(const CHAR *fmt, ... );

extern void   _mmerr_sethandler(void (*func)(int, CHAR *));
extern void   _mmerr_set(int err_is_human, const CHAR *human_is_err);
extern void   _mmerr_setsub(int err_is_human, const CHAR *human_is_err);
extern int    _mmerr_getinteger(void);
extern CHAR  *_mmerr_getstring(void);
extern void   _mmerr_handler(void);


#ifdef _DEBUG
#define _mmlog_debug _mmlog
#else
// Suppress Visual C's 'Unreachable Code' error.

//#define _mmlog_debug if(0) _mmlog
#endif

// =======================================
//  Input / Output and Streaming - MMIO.C
// =======================================

typedef struct MMSTREAM

// Mikmod customized module file pointer structure.
// Contains various data that may be required for a module.

{   FILE   *fp;      // File pointer
    UBYTE  *dp;

    long   iobase;       // base seek position within the file
    long   seekpos;      // used in data(mem) streaming mode only.
} MMSTREAM;

extern MMSTREAM *_mmstream_createfp(FILE *fp, int iobase);
extern MMSTREAM *_mmstream_createmem(void *data, int iobase);


// ===================================================
//  Memory allocation with error handling - MMALLOC.C
// ===================================================

extern void *_mm_malloc(size_t size);
extern void *_mm_calloc(size_t nitems, size_t size);
extern void *_mm_realloc(void *old_blk, size_t size);
extern void *_mm_recalloc(void *old_blk, size_t nitems, size_t size);
extern void _mm_a_free(void *d);
extern void _mm_b_free(void *d, const CHAR *logmsg);

#ifdef MM_LOG_VERBOSE

#define _mm_free(a,t)   _mm_b_free(&a,t)
#define _mmlogd         _mmlog

#else

#define _mm_free(a,t)   _mm_a_free(&a)

static void __inline _mmlogd(const CHAR *crap)
{
}

#endif

extern void _mm_RegisterErrorHandler(void (*proc)(int, CHAR *));
extern BOOL _mm_fexist(CHAR *fname);

extern void StringWrite(CHAR *s, MMSTREAM *fp);
extern CHAR *StringRead(MMSTREAM *fp);


//  MikMod/DivEnt style file input / output -
//    Solves several portability issues.
//    Notably little vs. big endian machine complications.

#define _mm_rewind(x) _mm_fseek(x,0,SEEK_SET)

extern int      _mm_fseek(MMSTREAM *stream, long offset, int whence);
extern long     _mm_ftell(MMSTREAM *stream);
extern BOOL     _mm_feof(MMSTREAM *stream);
extern MMSTREAM *_mm_fopen(const CHAR *fname, const CHAR *attrib);
extern void     _mm_fclose(MMSTREAM *mmfile);

extern long     _mm_flength(FILE *stream);
extern void     _mm_fputs(MMSTREAM *fp, CHAR *data);
extern BOOL     _mm_copyfile(FILE *fpi, FILE *fpo, uint len);
extern void     _mm_write_string(CHAR *data, MMSTREAM *fp);
extern int      _mm_read_string (CHAR *buffer, int number, MMSTREAM *fp);


extern SBYTE _mm_read_SBYTE (MMSTREAM *fp);
extern UBYTE _mm_read_UBYTE (MMSTREAM *fp);

extern SWORD _mm_read_M_SWORD (MMSTREAM *fp);
extern SWORD _mm_read_I_SWORD (MMSTREAM *fp);

extern UWORD _mm_read_M_UWORD (MMSTREAM *fp);
extern UWORD _mm_read_I_UWORD (MMSTREAM *fp);

extern SLONG _mm_read_M_SLONG (MMSTREAM *fp);
extern SLONG _mm_read_I_SLONG (MMSTREAM *fp);

extern ULONG _mm_read_M_ULONG (MMSTREAM *fp);
extern ULONG _mm_read_I_ULONG (MMSTREAM *fp);


extern int _mm_read_SBYTES    (SBYTE *buffer, int number, MMSTREAM *fp);
extern int _mm_read_UBYTES    (UBYTE *buffer, int number, MMSTREAM *fp);

extern int _mm_read_M_SWORDS  (SWORD *buffer, int number, MMSTREAM *fp);
extern int _mm_read_I_SWORDS  (SWORD *buffer, int number, MMSTREAM *fp);

extern int _mm_read_M_UWORDS  (UWORD *buffer, int number, MMSTREAM *fp);
extern int _mm_read_I_UWORDS  (UWORD *buffer, int number, MMSTREAM *fp);

extern int _mm_read_M_SLONGS  (SLONG *buffer, int number, MMSTREAM *fp);
extern int _mm_read_I_SLONGS  (SLONG *buffer, int number, MMSTREAM *fp);

extern int _mm_read_M_ULONGS  (ULONG *buffer, int number, MMSTREAM *fp);
extern int _mm_read_I_ULONGS  (ULONG *buffer, int number, MMSTREAM *fp);


extern void _mm_write_SBYTE     (SBYTE data, MMSTREAM *fp);
extern void _mm_write_UBYTE     (UBYTE data, MMSTREAM *fp);

extern void _mm_write_M_SWORD   (SWORD data, MMSTREAM *fp);
extern void _mm_write_I_SWORD   (SWORD data, MMSTREAM *fp);

extern void _mm_write_M_UWORD   (UWORD data, MMSTREAM *fp);
extern void _mm_write_I_UWORD   (UWORD data, MMSTREAM *fp);

extern void _mm_write_M_SLONG   (SLONG data, MMSTREAM *fp);
extern void _mm_write_I_SLONG   (SLONG data, MMSTREAM *fp);

extern void _mm_write_M_ULONG   (ULONG data, MMSTREAM *fp);
extern void _mm_write_I_ULONG   (ULONG data, MMSTREAM *fp);

extern void _mm_write_SBYTES    (SBYTE *data, int number, MMSTREAM *fp);
extern void _mm_write_UBYTES    (UBYTE *data, int number, MMSTREAM *fp);

extern void _mm_write_M_SWORDS  (SWORD *data, int number, MMSTREAM *fp);
extern void _mm_write_I_SWORDS  (SWORD *data, int number, MMSTREAM *fp);

extern void _mm_write_M_UWORDS  (UWORD *data, int number, MMSTREAM *fp);
extern void _mm_write_I_UWORDS  (UWORD *data, int number, MMSTREAM *fp);

extern void _mm_write_M_SLONGS  (SLONG *data, int number, MMSTREAM *fp);
extern void _mm_write_I_SLONGS  (SLONG *data, int number, MMSTREAM *fp);

extern void _mm_write_M_ULONGS  (ULONG *data, int number, MMSTREAM *fp);
extern void _mm_write_I_ULONGS  (ULONG *data, int number, MMSTREAM *fp);

#ifdef __WATCOMC__
#pragma aux _mm_fseek      parm nomemory modify nomemory
#pragma aux _mm_ftell      parm nomemory modify nomemory
#pragma aux _mm_flength    parm nomemory modify nomemory
#pragma aux _mm_fopen      parm nomemory modify nomemory
#pragma aux _mm_fputs      parm nomemory modify nomemory
#pragma aux _mm_copyfile   parm nomemory modify nomemory
#pragma aux _mm_iobase_get parm nomemory modify nomemory
#pragma aux _mm_iobase_set parm nomemory modify nomemory
#pragma aux _mm_iobase_setcur parm nomemory modify nomemory
#pragma aux _mm_iobase_revert parm nomemory modify nomemory
#pragma aux _mm_write_string  parm nomemory modify nomemory
#pragma aux _mm_read_string   parm nomemory modify nomemory

#pragma aux _mm_read_M_SWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_M_UWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_I_UWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_M_SLONG parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SLONG parm nomemory modify nomemory; 
#pragma aux _mm_read_M_ULONG parm nomemory modify nomemory; 
#pragma aux _mm_read_I_ULONG parm nomemory modify nomemory; 

#pragma aux _mm_read_M_SWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_M_UWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_UWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_M_SLONGS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SLONGS parm nomemory modify nomemory; 
#pragma aux _mm_read_M_ULONGS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_ULONGS parm nomemory modify nomemory; 

#pragma aux _mm_write_M_SWORD parm nomemory modify nomemory; 
#pragma aux _mm_write_I_SWORD parm nomemory modify nomemory; 
#pragma aux _mm_write_M_UWORD parm nomemory modify nomemory; 
#pragma aux _mm_write_I_UWORD parm nomemory modify nomemory; 
#pragma aux _mm_write_M_SLONG parm nomemory modify nomemory; 
#pragma aux _mm_write_I_SLONG parm nomemory modify nomemory; 
#pragma aux _mm_write_M_ULONG parm nomemory modify nomemory; 
#pragma aux _mm_write_I_ULONG parm nomemory modify nomemory; 

#pragma aux _mm_write_M_SWORDS parm nomemory modify nomemory; 
#pragma aux _mm_write_I_SWORDS parm nomemory modify nomemory; 
#pragma aux _mm_write_M_UWORDS parm nomemory modify nomemory; 
#pragma aux _mm_write_I_UWORDS parm nomemory modify nomemory; 
#pragma aux _mm_write_M_SLONGS parm nomemory modify nomemory; 
#pragma aux _mm_write_I_SLONGS parm nomemory modify nomemory; 
#pragma aux _mm_write_M_ULONGS parm nomemory modify nomemory; 
#pragma aux _mm_write_I_ULONGS parm nomemory modify nomemory; 
#endif


extern CHAR *_mm_strdup(const CHAR *src);
extern void *_mm_structdup(const void *src, size_t size);

#ifdef __cplusplus
};
#endif

#endif
