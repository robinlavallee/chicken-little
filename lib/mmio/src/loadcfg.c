/*
 
 Mikmod Portable System Management Facilities (the MMIO)

  By Jake Stine of Divine Entertainment (1996-2000) and

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 ------------------------------------------------------
 module: loadcfg.c

 Routines for loading the individual parts of a configuration file. This
 configuration code adheres to the basic standard of unix/windows-style
 conf/ini files.

 A configuration file would look something like this -

 [Video]
 page_flipping       = enabled
 physical_resolution = 640x480

 # this line is a comment, ignored by the reader.
 [Audio]
 master_volume       = 60
 music_volume        = 100
 soundfx_volume      = 50

*/

#include "mmio.h"
#include "mmconfig.h"
#include <string.h>

#define WhiteSpace(x)     ((x <= 32 && x) || (x == 255))
#define NotWhiteSpace(x)  (!(x <= 32) && (x != 255))


#define CFGLEN_THRESHOLD   256      // configuration line-by-line
#define SUBSEC_THRESHOLD    16       // subsection info allocations

// ===========================================================================
    static BOOL ParseSubSection(MMCONFIG *conf, CHAR *line)
// ===========================================================================
{
    int i=0,cpos;

    while(WhiteSpace(line[i])) i++;

    switch(line[i])
    {   case NULL:
        case  '#': break;

        case  '[':
            i++; cpos = 0;
            while((line[i] != ']') && line[i]) i++;
            if(!line[i])
            {   _mmlog("mmconfig > Line %d: Missing closing bracket",conf->length);
                return 0;
            }
        break;

        default:
            while((line[i] != '=') && line[i]) i++;
            if(!line[i])
            {   _mmlog("mmconfg > Line %d: Meaningless use of an expression",conf->length);
            }
            while(WhiteSpace(line[i])) i++;
            if(!line[i])
            {   _mmlog("mmconfig > Line %d: Missing value to right of variable",conf->length);
            }
        break;
    }
    return -1;
}


// ===========================================================================
    static BOOL AllocLine(MMCONFIG *conf, uint numlines)
// ===========================================================================
{
    if((conf->length+numlines) >= conf->length_alloc)
    {   uint  i;

        if((conf->line = (CHAR **)_mm_realloc(conf->line, (conf->length_alloc + CFGLEN_THRESHOLD) * (sizeof(CHAR *)))) == NULL)
        {   _mmcfg_exit(conf);
            return 0;
        }

        i = conf->length_alloc;
        conf->length_alloc += CFGLEN_THRESHOLD;
        for(; i<conf->length_alloc; i++) conf->line[i] = NULL;
    }
    return -1;
}


// ===========================================================================
    static BOOL AllocSubsection(MMCONFIG *conf, CHAR *str, uint line)
// ===========================================================================
// Add a subsection to the subsection list.  Note that this does not alter the
// configuration file in any way.  This function should only be called if the
// subsection already exists in the configuration file.
{
    if(conf->numsubsec >= conf->subsec_alloc)
    {   conf->subsec = (MMCFG_SUBSEC *)_mm_realloc(conf->subsec, (conf->subsec_alloc + SUBSEC_THRESHOLD) * sizeof(MMCFG_SUBSEC));
        conf->subsec_alloc += SUBSEC_THRESHOLD;
    }
    
    conf->subsec[conf->numsubsec].name = _mm_strdup(str);
    conf->subsec[conf->numsubsec].line = line;
    conf->numsubsec++;

    return -1;
}


// ===========================================================================
    static BOOL ListSubsections(MMCONFIG *conf)
// ===========================================================================
// Creates a list of all subsections in this configuration file.  This allows
// quicker indexing and access, and also allows the end user program to util-
// ize dynamic subsection names.
{
    uint    i,cpos,vlen;

    for(i=0; i<conf->length; i++)
    {   CHAR  *line;
    
        if(line = conf->line[i])
        {   cpos=0; while(WhiteSpace(line[cpos])) cpos++;
            if(line[cpos] == '[')
            {   cpos++; vlen=0;
                while(line[cpos] != ']' && line[cpos])
                {   conf->work[vlen] = line[cpos];
                    cpos++; vlen++;
                }
                conf->work[vlen] = 0;

                AllocSubsection(conf, conf->work, i);
            }
        }
    }

    return -1;
}


// ===========================================================================
    static MMCONFIG *_mmcfg_initfp(MMSTREAM *fp)
// ===========================================================================
// loads in the entire configuration file and processes it for optimized
// indexing of the sub-section indexes.  Not to be called by the user!
{
    UBYTE      c,c2;
    int        cpos;                  // character position on line
    MMCONFIG  *conf;

    conf       = (MMCONFIG *)_mm_calloc(1,sizeof(MMCONFIG));
    conf->work = (CHAR *)_mm_malloc(512);

    // read in all lines of the configuration file and mark each of the
    // bracketed sub-headers for future search referencing. ALSO: Check
    // syntax for validity.  If the syntax has major errors, then the 
    // configuration loading will be cancelled.

    c = _mm_read_UBYTE(fp);

    do
    {   cpos = 0;
        while(c!=13 && c!=10 && !_mm_feof(fp))
        {   conf->work[cpos] = c;
            cpos++;
            c = _mm_read_UBYTE(fp);
        }

        AllocLine(conf, 2);

        if(cpos==0)
            conf->line[conf->length] = NULL;
        else
        {   // check the syntax of this line for validity
            conf->work[cpos] = 0;
            if(!ParseSubSection(conf, conf->work))
            {   _mmcfg_exit(conf);
                return NULL;
            }

            conf->line[conf->length] = _mm_strdup(conf->work);
        }

        c2 = _mm_read_UBYTE(fp);
        if(c2!=c && (c2==10 || c2==13))
            c = _mm_read_UBYTE(fp);
        else
            c = c2;
        
        conf->length++;
    } while(!_mm_feof(fp));

    ListSubsections(conf);

    return conf;
}


// ===========================================================================
    MMCONFIG *_mmcfg_initfn(CHAR *fname)
// ===========================================================================
// opens the specified filename for reading / writing and then calls the
// main intialization procedure [detailed below].

{
    MMSTREAM  *fp;
    MMCONFIG  *conf;

    if((fp=_mm_fopen(fname,"rwb")) == NULL) return NULL;

    if(!(conf = _mmcfg_initfp(fp)))
    {   //_mmlog("mmconfig > Out of memory error opening %s.",fname);
        _mm_fclose(fp);  return NULL;
    }
    return conf;
}


// ===========================================================================
    void _mmcfg_exit(MMCONFIG *conf)
// ===========================================================================
// Frees up all resources allocated by the configuration libraries and saves
// any changes made to the configuration file.
{
    uint i;

    _mmlogd("mmconfig > Starting unloading sequence");

    if(conf->changed) _mmcfg_save(conf);

    _mm_fclose(conf->fp);

    _mmlogd(" > Freeing configuration file");

    for(i=0; i<conf->length; i++)
        if(conf->line[i]) _mm_free(conf->line[i], NULL);

    if(conf->line) _mm_free(conf->line, NULL);
    if(conf->work) _mm_free(conf->work, "workspace sting");
}


// ===========================================================================
    BOOL _mmcfg_set_subsection_int(MMCONFIG *conf, uint var, CHAR name[MMCONF_MAXNAMELEN])
// ===========================================================================
// set the active sub-section based on a simple integer index.  All requests
// for variable information will take place under this sub-section only.
{

    if(var < conf->numsubsec)
        conf->cursubsec = var;
    else
    {   _mmlog("Config > Invalid subsection range specified: %d",var);
        name[0] = 0;
        return 0;
    }

    strcpy(name, conf->subsec[var].name);

    return 1;
}


// ===========================================================================
BOOL _mmcfg_set_subsection(MMCONFIG *conf, const CHAR *var)
// ===========================================================================
// set the active sub-section.  All requests for variable information will
// take place under this sub-section only.  A second call to this function
// will change the sub-section - no other red-tape involved.
{
    uint    i;
    CHAR    work[MMCONF_MAXNAMELEN];

    if(!conf || !var) return 0;
    
    strcpy(work, var);

    if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(work);

    for(i=0; i<conf->numsubsec; i++)
    {   if(!strcmp(work, conf->subsec[i].name))
        {   conf->cursubsec = i;
            return -1;
        }
    }

    _mmlog("Config > Requested subsection '%s' was not found.", conf->subsec[i].name);
    return 0;
}


// ===========================================================================
int _mmcfg_findvar(MMCONFIG *conf, const CHAR *var)
// ===========================================================================
// searches for the given variable within the currently selected subsection
// and returns the line on which the variable is found.
// Returns -1 if the variable is not found.
{
    uint   lpos,cpos,vpos;
    CHAR  *line;

    for(lpos=conf->cursubsec; lpos<conf->length; lpos++)
    {  if((line = conf->line[lpos]))
       {  cpos=0; while(WhiteSpace(line[cpos])) cpos++;
          switch(line[cpos])
          {  case  '[':  return -1;

             case NULL:
             case  '#':  break;

             default:
                cpos++; vpos = 0;
                while(NotWhiteSpace(line[cpos]))
                {  conf->work[vpos] = line[cpos];
                   cpos++; vpos++;
                }
                conf->work[vpos] = 0;
                if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(conf->work);
                if(!strcmp(conf->work,var)) return lpos;
             break;
          }
       }
    }
    return -1;
}

// ===========================================================================
// The following three functions are designed to request certain types of
// information from any specified variable.  Only the currently indexed
// sub-section scope is searched.  The second parameter is the "default" value
// for the variable in question.

// If the search fails, the default value passed is again returned, and the
// cofiguration file is "patched" to contain the missing variable, properly
// assigned the default value.

static int reqline;   // used by req_int and req_bool for fixing faulty var settings.

BOOL _mmcfg_request_string(MMCONFIG *conf, const CHAR *var, CHAR *val)
{
    uint    lpos,cpos,vpos;
    CHAR   *line;
   
    for(lpos=conf->subsec[conf->cursubsec].line+1; lpos<conf->length; lpos++)
    {   if(line = conf->line[lpos])
        {   cpos=0; while(WhiteSpace(line[cpos])) cpos++;
            switch(line[cpos])
            {   case  '[':  return 0;  // end of subsection
                case NULL:
                case  '#':  break;

                default:
                    vpos = 0;
                    while(NotWhiteSpace(line[cpos]))
                    {   conf->work[vpos] = line[cpos];
                        cpos++; vpos++;
                    }

                    conf->work[vpos] = 0;

                    if(!strcmp(conf->work,var))
                    {   while(line[cpos] && (line[cpos] != '=')) cpos++;
                        cpos++;                                     // skip the equals
                        while(WhiteSpace(line[cpos])) cpos++;       // and skip the whitespace
                        if(line[cpos])
                        {   vpos = 0;
                            while(NotWhiteSpace(line[cpos]))
                            {   conf->work[vpos] = line[cpos];
                                cpos++; vpos++;
                            }
                            conf->work[vpos] = 0;
                            reqline = lpos;
                            strcpy(val, conf->work);
                            return 1;
                        }
                    }
                break;
            }
        }
    }

    //_mmcfg_reconstruct(conf, reqline=conf->cursubsec+1,var,str);

    return 0;
}


CHAR *_mmcfg_request_string_ex(MMCONFIG *conf, const CHAR *var, CHAR *str)
{
    /*if(!_mmcfg_request_string(conf,var))
    {   _mmcfg_reconstruct(conf,reqline=lpos,var,str);
        return strdup(str);
    }*/
    return NULL;
}


int _mmcfg_request_integer(MMCONFIG *conf, const CHAR *var, int val)
{
    CHAR    work[MMCONF_MAXNAMELEN];

    if(!conf || !var) return val;

    if(_mmcfg_request_string(conf, var, work)) val = atol(work);

    return val;
}


int _mmcfg_request_integer_ex(MMCONFIG *conf, const CHAR *var, int val)
{
    /*UBYTE tmpstr[33], *newstr;

    ltoa(val,tmpstr,10);
    newstr = _mmcfg_request_string(conf, var, tmpstr);
    val = atol(newstr); free(newstr);*/

    return val;
}


static CHAR *val_enabled  = "enabled",
            *val_true     = "true",
            *val_yes      = "yes",
            *val_on       = "on",

            *val_disabled = "disabled",
            *val_false    = "false",
            *val_off      = "off",
            *val_no       = "no";

BOOL _mmcfg_request_boolean(MMCONFIG *conf, const CHAR *var, BOOL val)
{
    CHAR    work[MMCONF_MAXNAMELEN];

    if(!conf || !var) return val;

    if(_mmcfg_request_string(conf, var, work))
    {   if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(work);

        if(strcmp(work,val_enabled)==0 || strcmp(work,val_true)==0 || strcmp(work,val_yes)==0 || strcmp(work,val_on)==0)
            val = 1;
        else if(strcmp(work,val_disabled)==0 || strcmp(work,val_false)==0 || strcmp(work,val_off)==0 || strcmp(work,val_no)==0)
            val = 0;
    }
    return val;
}

BOOL _mmcfg_request_boolean_ex(MMCONFIG *conf, const CHAR *var, BOOL val)
{
    //CHAR    work[MMCONF_MAXNAMELEN];

    /*newstr = _mmcfg_request_string_ex(conf, var, val ? val_enabled : val_disabled);
    strlwr(newstr);

    if(_mmcfg_request_boolean(conf, var);
    //_mmcfg_reconstruct(conf, reqline, var, newstr);*/

    return 0; //retval;
}

int _mmcfg_request_enum(MMCONFIG *conf, const CHAR *var, const CHAR **enu, int val)
{
    CHAR    work[MMCONF_MAXNAMELEN];

    if(!conf || !var) return val;

    if(_mmcfg_request_string(conf, var, work))
    {   int   i=0;

        if(conf->flags & MMCONF_CASE_INSENSITIVE) strlwr(work);
        while(enu[i])
        {   if(!strcmp(work, enu[i]))
            {   val = i;  break;  }
            i++;
        }
    }
    return val;
}


// ===========================================================================
    void _mmcfg_insert(MMCONFIG *conf, int line, const CHAR *var, CHAR *val)
// ===========================================================================
// Inserts a new variable / assignment entry into the current subsection.
// Insertion occurs either at the specified line, or if -1 is given, after
// the subsection header.
{
    _mmcfg_madechange(conf);
    if(line == -1) line = conf->subsec[conf->cursubsec].line+1;

    AllocLine(conf, 4);
    memcpy(&conf->line[line+3],&conf->line[line],conf->length - line);

    //sprintf(conf->work, "%45s %s", 
    conf->line[line] = strdup("# The following variable was inserted during game initialization");
    conf->line[line+1] = conf->line[line+2] = NULL;
    _mmcfg_reconstruct(conf, line+2, var, val);
    conf->length += 3;
}


// ===========================================================================
    void _mmcfg_reconstruct(MMCONFIG *conf, int line, const CHAR *var, CHAR *val)
// ===========================================================================
// Fixes errors in a configuration file and markes the file for saving when
// before it is closed - creating a backup file if first modification for
// the current session is being made.
//
// Input:
//   line  -  Line to find the variable (-1 for unknown)
//   var   -  variable to insert
//   val   -  value to assign to variable
{
    _mmcfg_madechange(conf);
    if(line == -1)
    {   if((line = _mmcfg_findvar(conf,var)) == -1)
        {  _mmcfg_insert(conf,-1,var,val);
           return;
        }
    }

    if(conf->line[line]) _mm_free(conf->line, NULL);
    strcpy(conf->work,var);
    strcat(conf->work,"  = ");
    strcat(conf->work,val);
    conf->line[line] = strdup(conf->work);
}

// ===========================================================================
    void _mmcfg_reassign(MMCONFIG *conf, const CHAR *var, CHAR *val)
// ===========================================================================
// Reassigns a value to an already existing variable.  Maintains the user's
// formatting.
{
    int    lpos,cpos;
    CHAR  *line;

    _mmcfg_madechange(conf);
    if((lpos = _mmcfg_findvar(conf, var)) == -1)
    {   _mmcfg_insert(conf, -1,var,val);
        return;
    }
    line = conf->line[lpos];  cpos = 0;
    while((line[cpos] != '=') && (line[cpos])) cpos++;
    while(WhiteSpace(line[cpos])) cpos++;
    line[cpos] = 0;

    strcpy(conf->work,line); strcat(conf->work,val);
    free(line);  conf->line[lpos]=strdup(conf->work);
}


char _mmcfg_backerr[] = "CFG Loader > Error creating backup file!\n";

// ===========================================================================
    void _mmcfg_madechange(MMCONFIG *conf)
// ===========================================================================
// Before any change to the configuration file has been made, this routine
// is called.  This is the routine that makes sure a backup is made and
// that a new replacement configuration file is made.
{
    FILE *fp;

    if(!conf->changed)
    {   conf->changed = 1;
        puts("CFG Loader > Generating backup copy of configuration file.\n");
        if((fp = fopen("backup.cfg","wb")) == NULL)
            puts(_mmcfg_backerr);
        else
        {   if(!_mmcfg_save(conf))
               puts(_mmcfg_backerr);
            fclose(fp);
        }
    }
}


// ===========================================================================
    BOOL _mmcfg_save(MMCONFIG *conf)
// ===========================================================================
// saves the configuration file (stored in conf->line) to the specified file.
{
   uint i;

   _mm_rewind(conf->fp);
   for(i=0; i<conf->length; i++)
      _mm_write_string(conf->line[i],conf->fp);
   return 1;
}


