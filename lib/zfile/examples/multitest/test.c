/* ==============================================================================================
	MultiTest for ZFile module 
	Copyright (c) 2000 Nathan Youngman. All rights reserved.
	<contact@nathany.com>
	
	Jul 21, 2000	test opening multiple archives at the same time

============================================================================================== */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "mmio.h"
#include "zfile.h"

#define WRITEBUFFERSIZE (8192)

/* ==============================================================================================
	ExtractFile
============================================================================================== */
int ExtractFile(char * filename, ZArchive archive)
{
	int BytesRead;

    FILE *fout = NULL;
    void *buf;
    const char * write_filename;

	ZFile file;
	
	// allocate buffer    
    buf = (void*) _mm_malloc(WRITEBUFFERSIZE);
    if (buf == NULL) return 0;	// Error allocating memory (_mm_error set)
        
	// open files	
	write_filename = ZFile_GetFileNameFromPath(filename);

	if( !(file = ZFile_OpenFile(filename, archive)) ) return false;
		
	fout = fopen(write_filename,"wb");
	if(fout == NULL)
	{
		printf("error opening %s\n",write_filename);
		return false;
	}
	
	// extract data
	printf(" extracting: %s\n",write_filename);

	do
	{
		BytesRead = ZFile_ReadFile(buf, WRITEBUFFERSIZE, file);
		if( BytesRead == -1 ) return false;
				
		if(BytesRead > 0)
		{
			if( fwrite(buf,BytesRead,1,fout) != 1 ) 
			{ 
				// error in writing extracted file
				_mmerr_set(ZFERR_UNABLE_TO_WRITE, "ExtractFile : unable to write.");
				return false;				
			}
		}
	}
	while( BytesRead > 0 );

	// close files
	fclose(fout);
	if( !ZFile_CloseFile(file) ) return false;
	
    _mm_free(buf);
	
    return true;
}

/* ==============================================================================================
	main
============================================================================================== */
int main(int argc, char *argv[])
{	
	int i;
	ZArchive archive1, archive2;	
	//ZFile file1, file2;
			
	printf("zFile multitest app by Nathan Youngman\n");

	for(i = 0; i < 10; i++) 
	{

		archive1 = ZFile_OpenArchive("shmup1.zip");
		archive2 = ZFile_OpenArchive("matrix.zip");


			
		// extract files
		ExtractFile("shmup/sprites.bmp", archive1);
		ExtractFile("matrix.bmp", archive2);

		/* // only one file per archive can be opened at a time
		if( !(file1 = ZFile_OpenFile("shmup/sprites.bmp", archive1)) ) 
		{
			printf("Error: %s (%d) \n", _mmerr_getstring(), _mmerr_getinteger() );
		}
		if( !(file2 = ZFile_OpenFile("shmup/tiles.bmp", archive1)) ) 
		{
			printf("Error: %s (%d) \n", _mmerr_getstring(), _mmerr_getinteger() );
		}
		ZFile_CloseFile(file1);
		ZFile_CloseFile(file2); */

		ZFile_CloseArchive(archive1);
		ZFile_CloseArchive(archive2);

	}

	if ( _mmerr_getinteger() != 0 ) 
	{
		printf("Error: %s (%d) \n", _mmerr_getstring(), _mmerr_getinteger() );
	}
	
	return 0;
}
