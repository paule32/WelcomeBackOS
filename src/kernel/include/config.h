#ifndef __CONFIG_H__
#define __CONFIG_H__

# include "stdint.h"

// ------------------------------------------------------------
// language constant's ...
// ------------------------------------------------------------
# define LANG_ENU 1         // English US
# define LANG_DEU 2         // German

#ifndef ISOGUI
# define ISOGUI 1
#endif
#ifndef ISOLANG
# define ISOLANG LANG_ENU
#endif

#ifdef __cplusplus
extern "C" void printformat (char *args, ...);
#else
           void printformat (char *args, ...);
#endif

extern int graph_mode;

#endif  // __CONFIG_H__
