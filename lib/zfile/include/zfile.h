#ifndef _ZFILE_H

typedef void * ZArchive;
typedef void * ZFile;

/* ==============================================================================================
	ZFile interface
	public methods are defined here (private data is defined in zfile.c)
============================================================================================== */
ZArchive ZFile_OpenArchive(char * szPath);
//ZArchive ZFile_OpenAttachedArchive(HINSTANCE hInstance);
bool ZFile_CloseArchive(ZArchive archive);

ZFile ZFile_OpenFile(char *szFileName, ZArchive archive);
bool ZFile_CloseFile(ZFile file);

int ZFile_ReadFile(void * buf, DWORD dwBytesToRead,ZFile file);
int ZFile_Skip(DWORD dwBytesToSkip,ZFile file);
int ZFile_GetChar(ZFile file);
long ZFile_GetFilePosition(ZFile file);
	
char * ZFile_GetFileNameFromPath(char * szPath);


// error codes
// this is actually no good, because ZFERRs have the same values as some MMERRs - n8
enum
{   ZFERR_NONE = 0,
	ZFERR_ARCHIVE_NOT_OPEN,
	ZFERR_CRC_ERROR,
	ZFERR_ARCHIVE_NOT_FOUND,
	ZFERR_ARCHIVE_ALREADY_OPEN,
	ZFERR_FILE_NOT_FOUND,
	ZFERR_FILE_UNABLE_TO_OPEN,
	ZFERR_FILE_UNABLE_TO_CLOSE,
	ZFERR_FILE_UNABLE_TO_READ,
	ZFERR_UNABLE_TO_WRITE
};

#define _ZFILE_H
#endif
