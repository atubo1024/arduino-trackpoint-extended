#ifndef __HAVE_HELPER_H
#define __HAVE_HELPER_H

#include "../serialframe.h"

int    GetConfigItemCount(void);
char** GetConfigItemNames(void);
char** GetConfigItemTypes(void);

int    GetOpCodeItemCount(void);
char** GetOpCodeItemNames(void);
int*   GetOpCodeItemValues(void);

int GenerateSerialFrame(struct SerialFrame *pSerialFrame, uint8_t opcode, uint8_t* data, uint8_t datalen);

#endif		/* #ifndef __HAVE_HELPER_H */


