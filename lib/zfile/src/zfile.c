/* ==============================================================================================
	ZFile module 
	Copyright (c) 2000 Nathan Youngman. All rights reserved.
	<contact@nathany.com>
	
	May 21, 2000	Initial Development
					- Created ZFile wrapper for Unzip module
					- Pass through by opening a folder (path terminated with '\')
					- OpenArchive(hInstance)
					- GetFilePosition, Skip, GetChar

	Jul 16, 2000	drop down to 'C' code

	Jul 18, 2000	uses mmio for memory allocation and error handling

    Jul 19, 2000    Altered new C version to be more object-like.
                    Moved all zfile variables into a structure, and altered OpenArchive
                    to return a pointer to a handle that is subsequently used with all
                    other zfile interface functions.

	Jul 21, 2000	multi-archive support. pass-through method changed.
					GetArchivePath dropped

					
                      More file functions:
					- get words, dwords (little/big endian)
					- get char strings (reads to newline char, appends null)
					- file exists, file length, eof
					- open for reading as text (Ctrl+Z = EOF and CRLF translated to LF)

					Error handling:
					- when 'throw' an error, should be closing down archive
					- error messages type system?

	Later?			- Write access for pass through routines, and eventually for archive
					- Multi archive handling (structs, ids, or merge to one directory with clashes 
					  resolved by file timestamps).
					- Multiple files open at the same time (current limitation of Unzip system)
					
					- SetPosition (fseek), would require uncompression of entire file to RAM (?)					
	
============================================================================================== */
#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "mmio.h"
#include "zfile.h"
#include "unzip.h"

#define MAX_PATH_LENGTH		(256)

typedef struct
{
	bool       bPassThrough;
	ZArchive  *lpArchive;
	FILE      *lpFile;
} zfile_s;

/* ==============================================================================================
	Open Archive
	Opens an archive or folder (pass through) for access by OpenFile, ReadFile, etc.	

	szPath:	path to archive file to open (default extension is .zip)			
============================================================================================== */
ZArchive ZFile_OpenArchive(char * szPath)
{
	ZArchive archive;
	char path[MAX_PATH_LENGTH];
				
	if( (szPath == NULL) || (strlen(szPath) > MAX_PATH_LENGTH) || !ZFile_GetFileNameFromPath(szPath))
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_OpenArchive : invalid path specifed."); 
		return NULL; 
	}

	archive = unzOpen(szPath);

	if (archive == NULL)
	{
		// warning if szPath is too long for extension test
		if(strlen(szPath) + 4 > MAX_PATH_LENGTH)
		{
			_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_OpenArchive : invalid path specifed."); 
			return NULL; 
		}

		strcpy(path,szPath);
		strcat(path,".zip");
		archive = unzOpen(path);
	}
	
	if (archive == NULL)
	{
		_mmerr_set(ZFERR_ARCHIVE_NOT_FOUND, "ZFile_OpenArchive : archive not found."); 
		return NULL;							// or other error?	
	}

	return archive;	// success
}
	
/* ==============================================================================================
	Open Archive
	Opens archive attached to end of self (.EXE file)

	hInstance:	application instance used to get path (NULL is the default application)
============================================================================================== */
/*ZArchive ZFile_OpenAttachedArchive(HINSTANCE hInstance)
{
	char szPath[MAX_PATH_LENGTH];

	if( GetModuleFileName(hInstance,szPath,MAX_PATH_LENGTH) == 0 ) 
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_OpenAttachedArchive : unable to get module filename."); 
		return false; 
	}			
	
	return ZFile_OpenArchive(szPath);
}*/

/* ==============================================================================================
	Close Archive
============================================================================================== */
bool ZFile_CloseArchive(ZArchive archive)
{
	if(archive == NULL) return false;	// already closed

	// closes any open files and release central directory
	if( unzClose(archive) != UNZ_OK ) 
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_CloseArchive : error closing archive."); 
		return false; 
	}

	archive = NULL;

	return true;	// success
}

/* ==============================================================================================
	Open File
============================================================================================== */
ZFile ZFile_OpenFile(char *szFileName, ZArchive archive)
{
	int err;
	FILE * fp;
	zfile_s * file;
		
	if ( !ZFile_GetFileNameFromPath(szFileName) ) // tried to open a folder
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_OpenFile : invalid filename specified."); 
		return NULL; 
	}
	
	// pass through
	if(archive == NULL)
	{	
		//printf("%s\n", szFileName);
		fp = fopen(szFileName, "rb");
		if( fp == NULL ) 
		{
			_mmerr_set(ZFERR_FILE_NOT_FOUND, "ZFile_OpenFile : file not found."); 
			return NULL;			// not found (or error?)
		}
		
		file = (zfile_s*)_mm_malloc(sizeof(zfile_s));
		file->bPassThrough = true;
		file->lpArchive = NULL;
		file->lpFile = fp;
	}
	// archive
	else
	{
		if( unzLocateFile(archive,szFileName) != UNZ_OK ) 
		{
			_mmerr_set(ZFERR_FILE_NOT_FOUND, "ZFile_OpenFile : file not found."); 
			return NULL;
		}			
		
		err = unzOpenCurrentFile(archive);

		if( err == UNZ_PARAMERROR )
		{
			_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_OpenFile : invalid parameter specified."); 
			return NULL;
		}
		else if( err == UNZ_INTERNALERROR )
		{
			_mmerr_set(MMERR_OUT_OF_MEMORY, "ZFile_OpenFile : out of memory."); 
			return NULL;			
		}
		else if( err == UNZ_FILE_ALREADY_OPEN )
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_OPEN, "ZFile_OpenFile : only one file per archive can be opened at a time."); 
			return NULL;
		}
		else if( err != UNZ_OK )
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_OPEN, "ZFile_OpenFile : unable to open."); 
			return NULL;			
		}

		file = (zfile_s *)_mm_malloc(sizeof(zfile_s));
		file->bPassThrough = false;
		file->lpArchive    = archive;
		file->lpFile       = NULL;
	}	

	return (ZFile)file;
}

/* ==============================================================================================
	Close File
============================================================================================== */
bool ZFile_CloseFile(ZFile file)
{
	int err;
	bool success = true;
	zfile_s * file_s;
	
	if( file == NULL )
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_CloseFile : invalid parameter specified."); 
		return false;
	}

	file_s = (zfile_s*)file;

	// pass through
	if( file_s->bPassThrough )
	{
		if( file_s->lpFile ) fclose( file_s->lpFile );	
	}
	// archive
	else
	{
		err = unzCloseCurrentFile(file_s->lpArchive);
		if( err == UNZ_PARAMERROR ) 
		{
			_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_CloseFile : invalid parameter specified."); 
			success = false;
		}
		else if( err == UNZ_CRCERROR )
		{
			_mmerr_set(ZFERR_CRC_ERROR, "ZFile_CloseFile : CRC error."); 
			success = false;	// somewhat unfortunate time to find out
		}			
		else if( err != UNZ_OK )
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_CLOSE, "ZFile_CloseFile : Unable to close file.");			
			success = false;
		}
	}
	
	// free ZFile pointer
	_mm_free(file);
	file = NULL;

	return success;
}

/* ==============================================================================================
	Read File
============================================================================================== */
int ZFile_ReadFile(void * buf, DWORD dwBytesToRead, ZFile file)
{
	int err;
	zfile_s * file_s;

	if( file == NULL )
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_ReadFile : invalid parameter specified."); 
		return -1;
	}	

	file_s = (zfile_s*)file;

	// pass through
	if(file_s->bPassThrough)
	{
		if(file_s->lpFile == NULL) 
		{			
			_mmerr_set(ZFERR_FILE_UNABLE_TO_READ, "ZFile_ReadFile : Unable to read."); 
			return -1;
		}
		
		err = fread(buf,1,dwBytesToRead,file_s->lpFile);
		
		if(err < 0)
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_READ, "ZFile_ReadFile : Unable to read."); 
			return -1;			
		}

		return err;			// bytes read
	}
	// archive
	else
	{
		err = unzReadCurrentFile(file_s->lpArchive, buf, dwBytesToRead);

		if( err == UNZ_PARAMERROR )
		{
			_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_ReadFile : invalid parameter specified."); 
			return -1;
		}
		else if( err == UNZ_EOF )		
		{
			return 0;	// hm.		
		}
		else if( err < 0 ) 
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_READ, "ZFile_ReadFile : Unable to read."); 
			return -1;
		}
		else
			return err;		// bytes read
	}

	return -1;
}

/* ==============================================================================================
	Skip
	skip over bytes in file
============================================================================================== */
int ZFile_Skip(DWORD dwBytesToSkip, ZFile file)
{
	int     BytesRead;
    UBYTE  *buf;

	// create temporary buffer
	buf = (UBYTE *)_mm_malloc(dwBytesToSkip);
	if(buf == NULL) return 0;	// Error allocating memory (_mm_error set)

	// read bytes
	BytesRead = ZFile_ReadFile(buf,dwBytesToSkip, file);

	// discard buffer
	_mm_free(buf);

	return BytesRead;
}

/* ==============================================================================================
	Get Char
	gets one char
============================================================================================== */
int ZFile_GetChar(ZFile file)
{
	int ch;
	zfile_s * file_s;
	
	if( file == NULL )
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_GetChar : invalid parameter specified."); 
		return -1; 
	}

	file_s = (zfile_s*)file;

	// pass through
	if(file_s->bPassThrough)
	{
		if(file_s->lpFile == NULL) 
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_READ, "ZFile_GetChar : Unable to read."); 
			return -1;
		}
		
		ch = fgetc(file_s->lpFile);

		if(ch < 0)
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_READ, "ZFile_GetChar : Unable to read."); 
			return -1;
		}

		return ch;
	}
	// archive
	else
	{
		int err = unzReadCurrentFile(file_s->lpArchive, &ch, 1);

		if( err == UNZ_PARAMERROR )
		{
			_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_GetChar : invalid parameter specified."); 
			return -1;
		}
		else if( err == UNZ_EOF )
		{
			// hm.
		}
		else if( err < 0 ) 
		{
			_mmerr_set(ZFERR_FILE_UNABLE_TO_READ, "ZFile_GetChar : Unable to read."); 
			return -1;
		}
		
		return ch;
	}	
}

/* ==============================================================================================
	Get File Position
	returns position in uncompressed datta	
============================================================================================== */
long ZFile_GetFilePosition(ZFile file)
{
	zfile_s * file_s;
	long pos;
	
	if( file == NULL )
	{
		_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_GetFilePosition : invalid parameter specified."); 
		return -1; 
	}

	file_s = (zfile_s*)file;

	// pass through
	if(file_s->bPassThrough)
	{
		return ftell(file_s->lpFile);
	}
	// archive
	else
	{
		pos = unztell(file_s->lpArchive);

		if(pos == UNZ_PARAMERROR) 
		{
			_mmerr_set(MMERR_INVALID_PARAMS, "ZFile_GetFilePosition : invalid parameter specified."); 
			return -1;
		}

		return pos;
	}
}

/* ==============================================================================================
	GetFileNameFromPath (utility function)
	returns filename part of path
============================================================================================== */
char * ZFile_GetFileNameFromPath(char * szPath)
{
	char *p;
	char *szFileName;

	// get filename (without path)
	p = szFileName = szPath;
	while( (*p) != '\0' )
	{
		if( ((*p) == '/') || ((*p) == '\\')) szFileName = p + 1;
		p++;
	}

	//printf("%s -> %s (%d)\n",szPath,szFileName, !szFileName );

	if(strlen(szFileName) == 0) 
		return NULL;
	else
		return szFileName;

}


