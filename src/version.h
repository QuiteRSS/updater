#include "version_rev.h"

#define STRDATE           "22.12.2019\0"
#define STRPRODUCTVER     "0.4.5\0"

#define VERSION           0,4,5
#define PRODUCTVER        VERSION,0
#define FILEVER           VERSION,VCS_REVISION

#define _STRFILE_BUILD(n) #n
#define STRFILE_BUILD(n)  _STRFILE_BUILD(n)
#define STRFILEVER_FULL   STRPRODUCTVER "." STRFILE_BUILD(VCS_REVISION) "\0"
