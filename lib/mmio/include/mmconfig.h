/*
  Configuration File Utilities
   - Load and save configuration files for game, applications,
     and most any other program.

  Copyright 1996 by Jake Stine [Air Richter] of Divine Entertainment
  All Rights Reserved.

              -- Prototype definitions and some such --
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "mmio.h"

#define MMCONF_MAXNAMELEN        128

#define MMCONF_CASE_INSENSITIVE    1

typedef struct MMCFG_SUBSEC
{   CHAR  *name;               // name
    uint   line;               // line (of the tag itself)
} MMCFG_SUBSEC;

typedef struct MMCONFIG
{
    uint      flags;
    CHAR    **line,            // storage for each line of the configuration file
             *work;            // general workspace for variable checks, etc.

    uint      length;          // length of the config file, in lines
    int       cursubsec;       // current working subsection

    BOOL      changed;         // changed-flag (indicates whether or not we save on exit)

    uint      numsubsec;
    MMCFG_SUBSEC *subsec;      // list of all subsections (name and line)

    MMSTREAM *fp;

    uint subsec_alloc;
    uint length_alloc;

} MMCONFIG;



// --> LOADCFG.C Prototypes
//  -> These are the only procedures the average user should have to call.

extern MMCONFIG *_mmcfg_initfn(CHAR *fname);
extern void      _mmcfg_exit(MMCONFIG *conf);
extern BOOL      _mmcfg_set_subsection(MMCONFIG *conf, const CHAR *var);
extern BOOL      _mmcfg_set_subsection_int(MMCONFIG *conf, uint var, CHAR name[MMCONF_MAXNAMELEN]);
extern int       _mmcfg_findvar(MMCONFIG *conf, const CHAR *var);
extern BOOL      _mmcfg_request_string(MMCONFIG *conf, const CHAR *var, CHAR *val);
extern int       _mmcfg_request_integer(MMCONFIG *conf, const CHAR *var, int val);
extern BOOL      _mmcfg_request_boolean(MMCONFIG *conf, const CHAR *var, BOOL val);
extern int       _mmcfg_request_enum(MMCONFIG *conf, const CHAR *var, const CHAR **enu, int val);

extern CHAR     *_mmcfg_request_string_ex(MMCONFIG *conf, const CHAR *var, CHAR *str);
extern int       _mmcfg_request_integer_ex(MMCONFIG *conf, const CHAR *var, int val);
extern BOOL      _mmcfg_request_boolean_ex(MMCONFIG *conf, const CHAR *var, BOOL val);


// --> EDITCFG.C Protoypes
//  -> For use either internally or by special programs - ie. SETUP.EXE.

extern void      _mmcfg_insert(MMCONFIG *conf, int line, const CHAR *var, CHAR *val);
extern void      _mmcfg_reconstruct(MMCONFIG *conf, int line, const CHAR *var, CHAR *val);
extern void      _mmcfg_reassign(MMCONFIG *conf, const CHAR *var, CHAR *val);
extern BOOL      _mmcfg_save(MMCONFIG *conf);
extern void      _mmcfg_madechange(MMCONFIG *conf);

#endif

