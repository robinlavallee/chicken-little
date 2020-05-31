/* ==============================================================================================
	Test for ZFile module 
	Copyright (c) 2000 Nathan Youngman. All rights reserved.
	<contact@nathany.com>
	
	May 21, 2000	Initial Development
					- test app based on miniunz by Gilles Vollant
					- uses ZFile interfaces
					- can extract a file from a .zip or a folder (pass through) or
					  from a zip attached to itself

	Jul 16, 2000	drop down to 'C' code

	Jul 18, 2000	uses mmio for memory allocation and error handling

	Jul 21, 2000	multi-archive support. pass-through method changed.

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
	char *zipfilename=NULL;
    char *filename_to_extract=NULL;
	ZArchive archive;
		
	printf("zFile test app by Nathan Youngman\n");

	if(argc <= 1 )
	{
		printf("Usage: test [file.zip] file_to_extract\n\n");
		return 0;
	}
	else if( argc == 2 )
	{			
		filename_to_extract = argv[1];

		archive = NULL;	// 'extract' from current directory
		// ZFile_OpenAttachedArchive(NULL);		// extract from self
	}
	else if( argc == 3 )
	{	
		zipfilename = argv[1];
		filename_to_extract = argv[2];

		archive = ZFile_OpenArchive(zipfilename);
	}
	
	// extract file		
	
	printf("%s opened\n",zipfilename);	//ZFile_GetArchivePath());

	ExtractFile(filename_to_extract, archive);

	ZFile_CloseArchive(archive);


	if ( _mmerr_getinteger() != 0 ) 
	{
		printf("Error: %s (%d) \n", _mmerr_getstring(), _mmerr_getinteger() );
	}

	// handle errors
	/*catch( int err ) 
	{
		switch(err)
		{
			case ERR_ZFILE_OPENARCHIVE_NOT_FOUND:
				{
					printf("Error: Archive not found\n");
					break;
				}
			case ERR_ZFILE_OPENFILE_NOT_FOUND:
				{
					printf("Error: File not found.\n");
					break;
				}
			default:				
				{
					printf("Error: An error has occured (%d)\n", err);
					break;
				}
		}
		return 1;
	}*/

	return 0;
}
