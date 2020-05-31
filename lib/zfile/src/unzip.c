/* ==============================================================================================
	unzip.cpp -- IO for uncompress .zip files using zlib 
	Version 0.15 beta, Mar 19th, 1998,

	Copyright (C) 1998 Gilles Vollant	

	This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
	 WinZip, InfoZip tools and compatible.
	
	Encryption and multi volume ZipFile (span) are not supported.
    Old compressions used by old PKZip 1.x are not supported

	THIS IS AN ALPHA VERSION. AT THIS STAGE OF DEVELOPPEMENT, SOMES API OR STRUCTURE
	CAN CHANGE IN FUTURE VERSION !!
	I WAIT FEEDBACK at mail info@winimage.com
	Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

	Condition of use and distribution are the same than zlib :

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source distribution.

   ==============================================================================================

	May 20, 2000	modified by Nathan Youngman <contact@nathany.com>
					compiled as CPP and cleaned up source
					always case insensitve filename comparisons
					drop global comment, unz_global_info_s
					drop file comment and local extra field 
					
					central_directory struct
					LoadCentralDirectory into RAM when open archive (Unload when close archive)

	Jul 16, 2000	drop down to 'C' code

	Jul 18, 2000	uses mmio for memory allocation

	Jul 21, 2000	unzOpenCurrentFile returns error if there already an open file for given archive
	
============================================================================================== */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "mmio.h"
#include "zlib.h"
#include "unzip.h"

#ifndef UNZ_BUFSIZE						// unzip buffer size
#define UNZ_BUFSIZE				(16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP			// length of filename 
#define UNZ_MAXFILENAMEINZIP	(256)
#endif

#define SIZECENTRALDIRITEM		(0x2e)
#define SIZEZIPLOCALHEADER		(0x1e)

/* ==============================================================================================
	unz_file_info_interntal contain internal info about a file in zipfile
============================================================================================== */
typedef struct
{
    ULONG offset_curfile;			// relative offset of local header		4 bytes
} unz_file_info_internal;


typedef struct
{
	unz_file_info file_info;						// public info about the current file in zip
	unz_file_info_internal file_info_internal;		// private info about it
	char szFileName[UNZ_MAXFILENAMEINZIP+1];
} unz_central_directory_s;

/* ==============================================================================================
	file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it 
============================================================================================== */
typedef struct
{
	char  *read_buffer;				// internal buffer for compressed data
	z_stream stream;				// zLib stream structure for inflate

	ULONG pos_in_zipfile;			// position in byte on the zipfile, for fseek
	ULONG stream_initialised;		// flag set if stream structure is initialised

	ULONG offset_local_extrafield;	// offset of the local extra field
	UINT  size_local_extrafield;	// size of the local extra field
	ULONG pos_local_extrafield;		// position in the local extra field in read

	ULONG crc32;					// crc32 of all data uncompressed
	ULONG crc32_wait;				// crc32 we must obtain after decompress all
	ULONG rest_read_compressed;		// number of byte to be decompressed
	ULONG rest_read_uncompressed;	// number of byte to be obtained after decomp
	FILE* file;						// io structore of the zipfile
	ULONG compression_method;		// compression method (0==store)
	ULONG byte_before_the_zipfile;	// byte before the zipfile, (>0 for sfx)
} file_in_zip_read_info_s;


/* ==============================================================================================
	unz_s contain internal information about the zipfile
============================================================================================== */
typedef struct
{
	FILE* file;						// io structore of the zipfile	
	ULONG number_entry;				// total number of entries in
									//  the central dir on this disk

	ULONG byte_before_the_zipfile;	// byte before the zipfile, (>0 for sfx)
	ULONG num_file;					// number of the current file in the zipfile
									// (index into unz_central_directory)
	ULONG pos_in_central_dir;		// pos of the current file in the central dir
	ULONG central_pos;				// position of the beginning of the central dir

	ULONG size_central_dir;			// size of the central directory
	ULONG offset_central_dir;		// offset of start of central directory with
									//  respect to the starting disk number

	//unz_file_info cur_file_info;					// public info about the current file in zip
	//unz_file_info_internal cur_file_info_internal;	// private info about it
    file_in_zip_read_info_s *pfile_in_zip_read;		// structure about the current
													//  file if we are decompressing it

	unz_central_directory_s *central_directory;		// array of all files in archive	
} unz_s;


/* ==============================================================================================
	functio prototypes: local functions
============================================================================================== */
static int unzlocal_GetCurrentFileInfoInternal(unzFile file,unz_file_info *pfile_info,unz_file_info_internal *pfile_info_internal,char *szFileName, ULONG fileNameBufferSize);


/* ==============================================================================================
	unzlocal_LoadCentralDirectory
============================================================================================== */
static int unzlocal_LoadCentralDirectory(unzFile file)
{	
	unz_s* s;
	ULONG i;
	int err;

	if (file == NULL) return UNZ_PARAMERROR;
	s=(unz_s*)file;

	err = unzGoToFirstFile(file);
	if(err != UNZ_OK) return err;

	s->central_directory = _mm_malloc( sizeof(unz_central_directory_s) * s->number_entry );
	if(s->central_directory == NULL) return UNZ_INTERNALERROR;

	for(i = 0; i < s->number_entry; i++) 
	{
		err = unzlocal_GetCurrentFileInfoInternal(file,&s->central_directory[i].file_info,&s->central_directory[i].file_info_internal,s->central_directory[i].szFileName,UNZ_MAXFILENAMEINZIP);
		if(err != UNZ_OK) { _mm_free(s->central_directory); return err; }

		if( i < s->number_entry - 1 )	// go to next file (unless on last file)
		{
			err = unzGoToNextFile(file);
			if(err != UNZ_OK) { _mm_free(s->central_directory); return err; }
		}
	}

	return UNZ_OK;
}

/* ==============================================================================================
	unzlocal_UnloadCentralDirectory
============================================================================================== */
static int unzlocal_UnloadCentralDirectory(unzFile file)
{
	unz_s* s;

	if (file == NULL) return UNZ_PARAMERROR;
	s=(unz_s*)file;

	_mm_free(s->central_directory);

	return UNZ_OK;
}



/* ==============================================================================================
	Read a byte from a gz_stream; update next_in and avail_in. Return EOF
	for end of file.
	IN assertion: the stream s has been sucessfully opened for reading.
============================================================================================== */
static int unzlocal_getByte(FILE * fin,int * pi)	
{
    unsigned char c;
	int err = fread(&c, 1, 1, fin);
    if (err==1)
    {
        *pi = (int)c;
        return UNZ_OK;
    }
    else
    {
        if (ferror(fin)) 
            return UNZ_ERRNO;
        else
            return UNZ_EOF;
    }
}


/* ==============================================================================================
	Reads a short in LSB order from the given gz_stream. 
============================================================================================== */
static int unzlocal_getShort (FILE* fin,ULONG *pX)
{
    ULONG x ;
    int i;
    int err;

    err = unzlocal_getByte(fin,&i);
    x = (ULONG)i;
    
    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((ULONG)i)<<8;
   
    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

/* ==============================================================================================
	Reads a long in LSB order from the given gz_stream. 
============================================================================================== */
static int unzlocal_getLong (FILE* fin, ULONG *pX)
{
    ULONG x ;
    int i;
    int err;

    err = unzlocal_getByte(fin,&i);
    x = (ULONG)i;
    
    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((ULONG)i)<<8;

    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((ULONG)i)<<16;

    if (err==UNZ_OK)
        err = unzlocal_getByte(fin,&i);
    x += ((ULONG)i)<<24;
   
    if (err==UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}



#define BUFREADCOMMENT (0x400)

/* ==============================================================================================
	Locate the Central directory of a zipfile (at the end, just before the global comment)
============================================================================================== */
static ULONG unzlocal_SearchCentralDir(FILE *fin)	
{
	unsigned char* buf;
	ULONG uSizeFile;
	ULONG uBackRead;
	ULONG uMaxBack=0xffff; /* maximum size of global comment */
	ULONG uPosFound=0;
	
	if (fseek(fin,0,SEEK_END) != 0)
		return 0;

	uSizeFile = ftell( fin );
	
	if (uMaxBack>uSizeFile)
		uMaxBack = uSizeFile;

	buf = (unsigned char*)_mm_malloc(BUFREADCOMMENT+4);
	if (buf==NULL)
		return 0;

	uBackRead = 4;
	while (uBackRead<uMaxBack)
	{
		ULONG uReadSize,uReadPos ;
		int i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack) 
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = uSizeFile-uBackRead ;
		
		uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile-uReadPos)) ? 
                     (BUFREADCOMMENT + 4) : (uSizeFile-uReadPos);
		if (fseek(fin,uReadPos,SEEK_SET)!=0)
			break;

		if (fread(buf,(UINT)uReadSize,1,fin)!=1)
			break;

                for (i=(int)uReadSize-3; (i--)>0;)
			if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && 
				((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
			{
				uPosFound = uReadPos+i;
				break;
			}

		if (uPosFound!=0)
			break;
	}
	_mm_free(buf);
	return uPosFound;
}

/* ==============================================================================================
	Open a Zip file. path contain the full pathname (i.e. "c:\\test\\zlib109.zip")

	 If the zipfile cannot be opened (file don't exist or in not valid), the
	   return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
	   of this unzip package.
============================================================================================== */
extern unzFile unzOpen(const char *path)
{
	unz_s us;
	unz_s *s;
	ULONG central_pos,uL;
	FILE * fin ;

	ULONG number_disk;          /* number of the current dist, used for 
								   spaning ZIP, unsupported, always 0*/
	ULONG number_disk_with_CD;  /* number the the disk with central dir, used
								   for spaning ZIP, unsupported, always 0*/
	ULONG number_entry_CD;      /* total number of entries in
	                               the central dir 
	                               (same than number_entry on nospan) */

	int err=UNZ_OK;

	ULONG size_comment;

    fin=fopen(path,"rb");
	if (fin==NULL)
		return NULL;

	central_pos = unzlocal_SearchCentralDir(fin);
	if (central_pos == 0) err=UNZ_ERRNO;

	if (fseek(fin,central_pos,SEEK_SET) != 0) err = UNZ_ERRNO;

	// the signature, already checked 
	if (unzlocal_getLong(fin,&uL) != UNZ_OK) err = UNZ_ERRNO;

	// number of this disk 
	if (unzlocal_getShort(fin,&number_disk) != UNZ_OK) err = UNZ_ERRNO;

	// number of the disk with the start of the central directory 
	if (unzlocal_getShort(fin,&number_disk_with_CD) != UNZ_OK) err = UNZ_ERRNO;

	// total number of entries in the central dir on this disk 
	if (unzlocal_getShort(fin,&us.number_entry) != UNZ_OK) err = UNZ_ERRNO;

	// total number of entries in the central dir 
	if (unzlocal_getShort(fin,&number_entry_CD) != UNZ_OK) err=UNZ_ERRNO;

	if ((number_entry_CD!=us.number_entry) ||
		(number_disk_with_CD!=0) ||
		(number_disk!=0))
		err=UNZ_BADZIPFILE;

	// size of the central directory 
	if (unzlocal_getLong(fin,&us.size_central_dir)!=UNZ_OK) err = UNZ_ERRNO;

	// offset of start of central directory with respect to the starting disk number /
	if (unzlocal_getLong(fin,&us.offset_central_dir) != UNZ_OK) err=UNZ_ERRNO;

	// zipfile comment length 	
	if (unzlocal_getShort(fin,&size_comment)!=UNZ_OK) err = UNZ_ERRNO;	// 

	if ((central_pos<us.offset_central_dir+us.size_central_dir) && (err==UNZ_OK)) err=UNZ_BADZIPFILE;

	if (err != UNZ_OK)
	{
		fclose(fin);
		return NULL;
	}

	us.file = fin;
	us.byte_before_the_zipfile = central_pos - (us.offset_central_dir+us.size_central_dir);
	us.central_pos = central_pos;
    us.pfile_in_zip_read = NULL;
	
	s = (unz_s*)_mm_malloc(sizeof(unz_s));
	*s = us;

	err = unzlocal_LoadCentralDirectory((unzFile)s);
	if(err != UNZ_OK) 
	{
		fclose(fin);
		_mm_free(s);
		return NULL;
	}

	unzGoToFirstFile((unzFile)s);

	return (unzFile)s;	
}


/* ==============================================================================================
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. 
============================================================================================== */
extern int unzClose(unzFile file)	 
{
	unz_s* s;

	if (file == NULL) return UNZ_PARAMERROR;
	s=(unz_s*)file;

    if (s->pfile_in_zip_read != NULL) unzCloseCurrentFile(file);

	unzlocal_UnloadCentralDirectory(file);

	fclose(s->file);

	_mm_free(s);

	return UNZ_OK;
}


/* ==============================================================================================
	unzGetFileCount
	returns number of entries in central directory
============================================================================================== */
extern ULONG unzGetFileCount(unzFile file)
{
	unz_s* s;

	if (file == NULL) return 0;		// UNZ_PARAMERROR

	s=(unz_s*)file;

	return s->number_entry;	
}


/* ==============================================================================================
   Translate date/time from Dos format to tm_unz (readable more easilty)
============================================================================================== */
static void unzlocal_DosDateToTmuDate (ULONG ulDosDate, tm_unz* ptm) 
{
    ULONG uDate;
    uDate = (ULONG)(ulDosDate>>16);
    ptm->tm_mday = (UINT)(uDate&0x1f) ;
    ptm->tm_mon =  (UINT)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (UINT)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (UINT) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (UINT) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (UINT) (2*(ulDosDate&0x1f)) ;
}

/* ==============================================================================================
  Get Info about the current file in the zipfile, with internal only info
============================================================================================== */
static int unzlocal_GetCurrentFileInfoInternal(unzFile file,unz_file_info *pfile_info,unz_file_info_internal *pfile_info_internal,char *szFileName, ULONG fileNameBufferSize)
{
	unz_s* s;
	unz_file_info file_info;
	unz_file_info_internal file_info_internal;
	int err=UNZ_OK;
	ULONG uMagic;
	long lSeek=0;

	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
	if (fseek(s->file,s->pos_in_central_dir+s->byte_before_the_zipfile,SEEK_SET)!=0)
		err=UNZ_ERRNO;


	// we check the magic 
	if (err == UNZ_OK) 
	{
		if (unzlocal_getLong(s->file,&uMagic) != UNZ_OK)
			err = UNZ_ERRNO;
		else if (uMagic!=0x02014b50)
			err = UNZ_BADZIPFILE;
	}

	if (unzlocal_getShort(s->file,&file_info.version) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.version_needed) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.flag) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.compression_method) != UNZ_OK) err = UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&file_info.dosDate) != UNZ_OK) err = UNZ_ERRNO;
    unzlocal_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

	if (unzlocal_getLong(s->file,&file_info.crc) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getLong(s->file,&file_info.compressed_size) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getLong(s->file,&file_info.uncompressed_size) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.size_filename) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.size_file_extra) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.size_file_comment) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.disk_num_start) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getShort(s->file,&file_info.internal_fa) != UNZ_OK) err = UNZ_ERRNO;
	if (unzlocal_getLong(s->file,&file_info.external_fa) != UNZ_OK) err = UNZ_ERRNO;
	
	if (unzlocal_getLong(s->file,&file_info_internal.offset_curfile) != UNZ_OK) err = UNZ_ERRNO;

	lSeek += file_info.size_filename;

	if ((err == UNZ_OK) && (szFileName != NULL))
	{
		ULONG uSizeRead;
		if (file_info.size_filename < fileNameBufferSize)
		{
			*(szFileName+file_info.size_filename)='\0';
			uSizeRead = file_info.size_filename;
		}
		else
			uSizeRead = fileNameBufferSize;

		if ((file_info.size_filename>0) && (fileNameBufferSize>0))
			if (fread(szFileName,(UINT)uSizeRead,1,s->file) != 1)
				err=UNZ_ERRNO;
		lSeek -= uSizeRead;
	}

	lSeek+=file_info.size_file_extra;		// skip extraField
	lSeek+=file_info.size_file_comment;		// skip comment

	if ((err==UNZ_OK) && (pfile_info!=NULL))
		*pfile_info=file_info;

	if ((err==UNZ_OK) && (pfile_info_internal!=NULL))
		*pfile_info_internal=file_info_internal;

	return err;
}



/* ==============================================================================================
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
============================================================================================== */
extern int unzGetCurrentFileInfo(unzFile file,unz_file_info *pfile_info,char *szFileName, ULONG fileNameBufferSize)
{
	//return unzlocal_GetCurrentFileInfoInternal(file,pfile_info,NULL,szFileName,fileNameBufferSize);

	unz_s* s;
	if (file == NULL) return UNZ_PARAMERROR;
	s=(unz_s*)file;

	*pfile_info = s->central_directory[s->num_file].file_info;
	
	if(strlen(s->central_directory[s->num_file].szFileName) > fileNameBufferSize) return UNZ_ERRNO;
	strcpy(szFileName,s->central_directory[s->num_file].szFileName);
	
	return UNZ_OK;
}

/* ==============================================================================================
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
============================================================================================== */
extern int unzGoToFirstFile(unzFile file)	
{	
	unz_s* s;
	if (file == NULL) return UNZ_PARAMERROR;
	s=(unz_s*)file;

	s->pos_in_central_dir=s->offset_central_dir;
	s->num_file=0;
	
	return UNZ_OK;
}

/* ==============================================================================================
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
============================================================================================== */
extern int unzGoToNextFile(unzFile file)	 
{
	unz_s* s;	
	//int err;

	if (file == NULL) return UNZ_PARAMERROR;

	s=(unz_s*)file;	
	if (s->num_file + 1 == s->number_entry) return UNZ_END_OF_LIST_OF_FILE;

	s->pos_in_central_dir += SIZECENTRALDIRITEM + s->central_directory[s->num_file].file_info.size_filename +
			s->central_directory[s->num_file].file_info.size_file_extra + s->central_directory[s->num_file].file_info.size_file_comment;

	s->num_file++;
	
	return UNZ_OK;
}

/* ==============================================================================================
  Try locate the file szFileName in the zipfile.  

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
============================================================================================== */
extern int unzLocateFile(unzFile file, const char *szFileName)
{
	unz_s* s;	
	int err;
	ULONG num_fileSaved, pos_in_central_dirSaved;

	if (file == NULL) return UNZ_PARAMERROR;

    if (strlen(szFileName) >= UNZ_MAXFILENAMEINZIP) return UNZ_PARAMERROR;

	s=(unz_s*)file;
	
	num_fileSaved = s->num_file;
	pos_in_central_dirSaved = s->pos_in_central_dir;

	err = unzGoToFirstFile(file);

	while (err == UNZ_OK)
	{		
		if(_stricmp(s->central_directory[s->num_file].szFileName,szFileName) == 0) return UNZ_OK;
		err = unzGoToNextFile(file);
	}

	s->num_file = num_fileSaved;
	s->pos_in_central_dir = pos_in_central_dirSaved;
	
	return err;
}


/* ==============================================================================================
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
        directory about this file
  store in *piSizeVar the size of extra info in local header
        (filename and size of extra field data)
============================================================================================== */
static int unzlocal_CheckCurrentFileCoherencyHeader (unz_s* s,UINT* piSizeVar,ULONG *poffset_local_extrafield,UINT *psize_local_extrafield)
{
	ULONG uMagic,uData,uFlags;
	ULONG size_filename;
	ULONG size_extra_field;
	int err=UNZ_OK;

	*piSizeVar = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield = 0;

	if (fseek(s->file,s->central_directory[s->num_file].file_info_internal.offset_curfile +
								s->byte_before_the_zipfile,SEEK_SET)!=0)
		return UNZ_ERRNO;


	if (err==UNZ_OK)
		if (unzlocal_getLong(s->file,&uMagic) != UNZ_OK)
			err=UNZ_ERRNO;
		else if (uMagic!=0x04034b50)
			err=UNZ_BADZIPFILE;

	if (unzlocal_getShort(s->file,&uData) != UNZ_OK)
		err=UNZ_ERRNO;
/*
	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
		err=UNZ_BADZIPFILE;
*/
	if (unzlocal_getShort(s->file,&uFlags) != UNZ_OK)
		err=UNZ_ERRNO;

	if (unzlocal_getShort(s->file,&uData) != UNZ_OK)
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData != s->central_directory[s->num_file].file_info.compression_method))
		err=UNZ_BADZIPFILE;

    if ((err==UNZ_OK) && (s->central_directory[s->num_file].file_info.compression_method!=0) &&
                         (s->central_directory[s->num_file].file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) /* date/time */
		err=UNZ_ERRNO;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) /* crc */
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData != s->central_directory[s->num_file].file_info.crc) &&
		                      ((uFlags & 8)==0))
		err=UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) /* size compr */
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData != s->central_directory[s->num_file].file_info.compressed_size) &&
							  ((uFlags & 8)==0))
		err=UNZ_BADZIPFILE;

	if (unzlocal_getLong(s->file,&uData) != UNZ_OK) /* size uncompr */
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (uData != s->central_directory[s->num_file].file_info.uncompressed_size) && 
							  ((uFlags & 8)==0))
		err=UNZ_BADZIPFILE;


	if (unzlocal_getShort(s->file,&size_filename) != UNZ_OK)
		err=UNZ_ERRNO;
	else if ((err==UNZ_OK) && (size_filename != s->central_directory[s->num_file].file_info.size_filename))
		err=UNZ_BADZIPFILE;

	*piSizeVar += (UINT)size_filename;

	if (unzlocal_getShort(s->file,&size_extra_field) != UNZ_OK)
		err=UNZ_ERRNO;
	*poffset_local_extrafield= s->central_directory[s->num_file].file_info_internal.offset_curfile +
									SIZEZIPLOCALHEADER + size_filename;
	*psize_local_extrafield = (UINT)size_extra_field;

	*piSizeVar += (UINT)size_extra_field;

	return err;
}
												
/* ==============================================================================================
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
============================================================================================== */
extern int unzOpenCurrentFile(unzFile file)
{
	int err=UNZ_OK;
	int Store;
	UINT iSizeVar;
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	ULONG offset_local_extrafield;  /* offset of the local extra field */
	UINT  size_local_extrafield;    /* size of the local extra field */

	if (file == NULL) return UNZ_PARAMERROR;
	
	s=(unz_s*)file;
	
    //if (s->pfile_in_zip_read != NULL) unzCloseCurrentFile(file);
	if (s->pfile_in_zip_read != NULL) return UNZ_FILE_ALREADY_OPEN;

	if (unzlocal_CheckCurrentFileCoherencyHeader(s,&iSizeVar,
		&offset_local_extrafield,&size_local_extrafield)!=UNZ_OK) return UNZ_BADZIPFILE;

	pfile_in_zip_read_info = (file_in_zip_read_info_s*) _mm_malloc(sizeof(file_in_zip_read_info_s));
	if (pfile_in_zip_read_info == NULL) return UNZ_INTERNALERROR;

	pfile_in_zip_read_info->read_buffer = (char*)_mm_malloc(UNZ_BUFSIZE);
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
	pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
	pfile_in_zip_read_info->pos_local_extrafield=0;

	if (pfile_in_zip_read_info->read_buffer == NULL)
	{
		_mm_free(pfile_in_zip_read_info);
		return UNZ_INTERNALERROR;
	}

	pfile_in_zip_read_info->stream_initialised=0;
	
	if ((s->central_directory[s->num_file].file_info.compression_method!=0) &&
        (s->central_directory[s->num_file].file_info.compression_method!=Z_DEFLATED))
		err=UNZ_BADZIPFILE;
	Store = s->central_directory[s->num_file].file_info.compression_method == 0;

	pfile_in_zip_read_info->crc32_wait = s->central_directory[s->num_file].file_info.crc;
	pfile_in_zip_read_info->crc32 = 0;
	pfile_in_zip_read_info->compression_method = s->central_directory[s->num_file].file_info.compression_method;
	pfile_in_zip_read_info->file = s->file;
	pfile_in_zip_read_info->byte_before_the_zipfile = s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

	if (!Store)
	{
	  pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
	  pfile_in_zip_read_info->stream.zfree = (free_func)0;
	  pfile_in_zip_read_info->stream.opaque = (voidpf)0; 
      
	  err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
	  if (err == Z_OK)
	    pfile_in_zip_read_info->stream_initialised = 1;
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END. 
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the 
         * size of both compressed and uncompressed data
         */
	}

	pfile_in_zip_read_info->rest_read_compressed = s->central_directory[s->num_file].file_info.compressed_size;
	pfile_in_zip_read_info->rest_read_uncompressed = s->central_directory[s->num_file].file_info.uncompressed_size ;

	
	pfile_in_zip_read_info->pos_in_zipfile = 
		s->central_directory[s->num_file].file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar;
	
	pfile_in_zip_read_info->stream.avail_in = (UINT)0;

	s->pfile_in_zip_read = pfile_in_zip_read_info;
    return UNZ_OK;
}


/* ==============================================================================================
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
============================================================================================== */
extern int unzReadCurrentFile(unzFile file, void *buf, unsigned len)	 
{
	int err = UNZ_OK;
	UINT iRead = 0;
	unz_s* s;

	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file == NULL) return UNZ_PARAMERROR;
	
	s=(unz_s*)file;

    pfile_in_zip_read_info=s->pfile_in_zip_read;
	if (pfile_in_zip_read_info == NULL) return UNZ_PARAMERROR;

	if ((pfile_in_zip_read_info->read_buffer == NULL)) return UNZ_END_OF_LIST_OF_FILE;
	if (len == 0) return 0;

	pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

	pfile_in_zip_read_info->stream.avail_out = (UINT)len;
	
	if (len>pfile_in_zip_read_info->rest_read_uncompressed)
	{
		pfile_in_zip_read_info->stream.avail_out = 
		  (UINT)pfile_in_zip_read_info->rest_read_uncompressed;
	}

	while (pfile_in_zip_read_info->stream.avail_out>0)
	{
		if ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
		{
			UINT uReadThis = UNZ_BUFSIZE;
			if (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
				uReadThis = (UINT)pfile_in_zip_read_info->rest_read_compressed;
			if (uReadThis == 0)
				return UNZ_EOF;
			if (fseek(pfile_in_zip_read_info->file,
                      pfile_in_zip_read_info->pos_in_zipfile + 
                         pfile_in_zip_read_info->byte_before_the_zipfile,SEEK_SET)!=0)
				return UNZ_ERRNO;
			if (fread(pfile_in_zip_read_info->read_buffer,uReadThis,1,
                         pfile_in_zip_read_info->file)!=1)
				return UNZ_ERRNO;
			pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

			pfile_in_zip_read_info->rest_read_compressed-=uReadThis;
			
			pfile_in_zip_read_info->stream.next_in = 
                (Bytef*)pfile_in_zip_read_info->read_buffer;
			pfile_in_zip_read_info->stream.avail_in = (UINT)uReadThis;
		}

		if (pfile_in_zip_read_info->compression_method==0)
		{
			UINT uDoCopy,i ;
			if (pfile_in_zip_read_info->stream.avail_out < 
                            pfile_in_zip_read_info->stream.avail_in)
				uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
			else
				uDoCopy = pfile_in_zip_read_info->stream.avail_in ;
				
			for (i=0;i<uDoCopy;i++)
				*(pfile_in_zip_read_info->stream.next_out+i) =
                        *(pfile_in_zip_read_info->stream.next_in+i);
					
			pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
								pfile_in_zip_read_info->stream.next_out,
								uDoCopy);
			pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
			pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
			pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
			pfile_in_zip_read_info->stream.next_out += uDoCopy;
			pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
			iRead += uDoCopy;
		}
		else
		{
			ULONG uTotalOutBefore,uTotalOutAfter;
			const Bytef *bufBefore;
			ULONG uOutThis;
			int flush=Z_SYNC_FLUSH;

			uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
			bufBefore = pfile_in_zip_read_info->stream.next_out;

			/*
			if ((pfile_in_zip_read_info->rest_read_uncompressed ==
			         pfile_in_zip_read_info->stream.avail_out) &&
				(pfile_in_zip_read_info->rest_read_compressed == 0))
				flush = Z_FINISH;
			*/
			err=inflate(&pfile_in_zip_read_info->stream,flush);

			uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
			uOutThis = uTotalOutAfter-uTotalOutBefore;
			
			pfile_in_zip_read_info->crc32 = 
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (UINT)(uOutThis));

			pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

			iRead += (UINT)(uTotalOutAfter - uTotalOutBefore);
            
			if (err==Z_STREAM_END)
				return (iRead==0) ? UNZ_EOF : iRead;
			if (err!=Z_OK) 
				break;
		}
	}

	if (err==Z_OK) return iRead;
	return err;
}


/* ==============================================================================================
  Give the current position in uncompressed data
============================================================================================== */
extern long unztell(unzFile file)
{
	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;

	return (long)pfile_in_zip_read_info->stream.total_out;
}


/* ==============================================================================================
  return 1 if the end of file was reached, 0 elsewhere 
============================================================================================== */
extern int unzeof(unzFile file)	
{
	unz_s* s;
	
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;

	s=(unz_s*)file;
    
	pfile_in_zip_read_info=s->pfile_in_zip_read;
	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;
	
	if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
		return 1;
	else
		return 0;
}


/* ==============================================================================================
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
============================================================================================== */
extern int unzCloseCurrentFile(unzFile file)	 
{
	int err=UNZ_OK;

	unz_s* s;
	file_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return UNZ_PARAMERROR;
	s=(unz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return UNZ_PARAMERROR;


	if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
	{
		if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
			err=UNZ_CRCERROR;
	}

	_mm_free(pfile_in_zip_read_info->read_buffer);
	pfile_in_zip_read_info->read_buffer = NULL;
	if (pfile_in_zip_read_info->stream_initialised)
		inflateEnd(&pfile_in_zip_read_info->stream);

	pfile_in_zip_read_info->stream_initialised = 0;
	_mm_free(pfile_in_zip_read_info);

    s->pfile_in_zip_read=NULL;

	return err;
}


