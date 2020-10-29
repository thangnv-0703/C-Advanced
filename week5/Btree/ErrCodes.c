/* ****************   MACHINE GENERATED   ***************** */

#include <stdio.h>

#include "jsconfig.h"

/*********     DEFINES						*********/
/*  START-DEFINITIONS  */
#define _ERR_BFILETRUNC        1
#define _ERR_BFILEGET          2
#define _ERR_BFILESET          4
#define _ERR_BAYTREE           5
#define _ERR_BSORTOPEN        11
/*  END-DEFINITIONS  */

#define Prototype extern
/*********     PROTOTYPES					*********/
Prototype const char    *JSErrors[];

const char *JSErrors[] = {
#ifdef CONFIG_ERRMSGFILE
  CONFIG_ERRMSGFILE
#else
  NULL,
  /* ****   1: File: bufffile.c  *** */
  /* 1 */ "bFileTruncate: Locked pages beyond file size found",

  /* 2 */ "bFileGet: Cannot map complete region",
  /* 3 */ "bFileGet: End of file",

  /* 4 */ "bFileGet: Page not in memory",

  /* ****   4: File: btreefile.c  *** */
  /* 5 */ "bayTree: Page size too small",
  /* 6 */ "bayTree: Set not found",
  /* 7 */ "bayTree: Field content not unique",
  /* 8 */ "bayTree: Illegal page length",
  /* 9 */ "bayTree: Fields not sorted",
  /* 10 */ "bayTree: Tree's depth changed",

  /* ****   5: File: bigsort.c  *** */
  /* 11 */ "bigSortOpen: BSortMaxMem to small",

  NULL
#endif
};
