#ifndef _unzip_H
#define _unzip_H

#define Z_DEFLATED						8
	
typedef void * unzFile;

#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE			(-100)
#define UNZ_ERRNO						(-1)	// Z_ERRNO
#define UNZ_EOF							(0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)
#define UNZ_FILE_ALREADY_OPEN           (-106)

// tm_unz contain date/time info
typedef struct tm_unz_s 
{
	uint tm_sec;			// seconds after the minute - [0,59]
	uint tm_min;            // minutes after the hour - [0,59]
	uint tm_hour;           // hours since midnight - [0,23]
	uint tm_mday;           // day of the month - [1,31]
	uint tm_mon;            // months since January - [0,11]
	uint tm_year;           // years - [1980..2044]
} tm_unz;

// unz_file_info contain information about a file in the zipfile
typedef struct unz_file_info_s
{
    ULONG version;              // version made by                 2 bytes
    ULONG version_needed;       // version needed to extract       2 bytes
    ULONG flag;                 // general purpose bit flag        2 bytes
    ULONG compression_method;   // compression method              2 bytes
    ULONG dosDate;              // last mod file date in Dos fmt   4 bytes
    ULONG crc;                  // crc-32                          4 bytes
    ULONG compressed_size;      // compressed size                 4 bytes
    ULONG uncompressed_size;    // uncompressed size               4 bytes
    ULONG size_filename;        // filename length                 2 bytes
    ULONG size_file_extra;      // extra field length              2 bytes
    ULONG size_file_comment;    // file comment length             2 bytes

    ULONG disk_num_start;       // disk number start               2 bytes
    ULONG internal_fa;          // internal file attributes        2 bytes
    ULONG external_fa;          // external file attributes        4 bytes

    tm_unz tmu_date;
} unz_file_info;

extern unzFile unzOpen(const char *path);
extern int unzClose(unzFile file);
extern ULONG unzGetFileCount(unzFile file);
extern int unzGoToFirstFile(unzFile file);
extern int unzGoToNextFile(unzFile file);
extern int unzLocateFile(unzFile file, const char *szFileName);
extern int unzGetCurrentFileInfo(unzFile file,unz_file_info *pfile_info,char *szFileName,ULONG fileNameBufferSize);
extern int unzOpenCurrentFile(unzFile file);
extern int unzCloseCurrentFile(unzFile file);
extern int unzReadCurrentFile(unzFile file,void * buf,unsigned len);
extern long unztell(unzFile file);
extern int unzeof(unzFile file);

#endif	// _unzip_H
