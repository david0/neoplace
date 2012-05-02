#ifndef _TRAYICON_H
#define _TRAYICON_H

#include <windows.h>
#include <stdbool.h>

typedef void (*callback_functionPtr)(); 

bool trayicon_init(HICON icon, char tooltip[]);
void trayicon_remove();

void trayicon_add_item(char *text, callback_functionPtr functionPtr);

#endif
