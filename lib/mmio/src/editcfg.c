/*
 Mikmod Portable Configuration File Utilities

 By Jake Stine of Divine Entertainment (1996-2000)

 --------------------------------------------------------------------------
 File: loadcfg.c

  Routines for editing the individual parts of a configuration file.

*/

#include "mmio.h"
#include "mmconfig.h"
#include <string.h>


#define WhiteSpace(x)     ((x <= 32 && x!=0) || (x == 255))
#define NotWhiteSpace(x)  (!(x <= 32) && (x != 255))

// Inserts a new variable / assignment entry into the current subsection.
// Insertion occurs either at the specified line, or if -1 is given, after
// the subsection header.

void cfg_insert(MMCONFIG *conf, int line, CHAR *var, CHAR *val)
{
    cfg_madechange(conf);
    if(line == -1) line = conf->subsec;
    memcpy(&conf->line[line+3],&conf->line[line],conf->length - line);
    conf->line[line] = strdup("# The following variable was inserted during game initialization");
    conf->line[line+1] = conf->line[line+2] = NULL;
    cfg_reconstruct(conf, line+2, var, val);
    conf->length += 3;
}


// Fixes errors in a configuration file and markes the file for saving when
// before it is closed - creating a backup file if first modification for
// the current session is being made.
//
// Input:
//   line  -  Line to find the variable (-1 for unknown)
//   var   -  variable to insert
//   val   -  value to assign to variable

void cfg_reconstruct(MMCONFIG *conf, int line, CHAR *var, CHAR *val)
{
    cfg_madechange(conf);
    if(line == -1)
    {   if((line = cfg_findvar(conf,var)) == -1)
        {  cfg_insert(conf,-1,var,val);
           return;
        }
    }

    if(conf->line[line]) _mm_free(conf->line);
    strcpy(conf->work,var);
    strcat(conf->work,"  = ");
    strcat(conf->work,val);
    conf->line[line] = strdup(conf->work);
}

// Reassigns a value to an already existing variable.  Maintains the user's
// formatting.
void cfg_reassign(MMCONFIG *conf, CHAR *var, CHAR *val)
{
    int    lpos,cpos;
    UBYTE *line;

    cfg_madechange(conf);
    if((lpos = cfg_findvar(conf, var)) == -1)
    {   cfg_insert(conf, -1,var,val);
        return;
    }
    line = conf->line[lpos];  cpos = 0;
    while((line[cpos] != '=') && (line[cpos])) cpos++;
    while(WhiteSpace(line[cpos])) cpos++;
    line[cpos] = 0;

    strcpy(conf->work,line); strcat(conf->work,val);
    free(line);  conf->line[lpos]=strdup(conf->work);
}


// Before any change to the configuration file has been made, this routine
// is called.  This is the routine that makes sure a backup is made and
// that a new replacement configuration file is made.

char cfg_backerr[] = "CFG Loader > Error creating backup file!\n";

void cfg_madechange(MMCONFIG *conf)
{
    FILE *fp;

    if(!conf->changed)
    {   conf->changed = 1;
        puts("CFG Loader > Generating backup copy of configuration file.\n");
        if((fp = fopen("backup.cfg","wb")) == NULL)
            puts(cfg_backerr);
        else
        {   if(!cfg_save(conf))
               puts(cfg_backerr);
            fclose(fp);
        }
    }
}


// saves the configuration file (stored in conf->line) to the specified file.
BOOL cfg_save(MMCONFIG *conf)
{
   int i;

   _mm_rewind(conf->fp);
   for(i=0; i<conf->length; i++)
      _mm_write_string(conf->line[i],conf->fp);
   return 1;
}


