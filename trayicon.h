#ifndef _TRAYICON_H
#define _TRAYICON_H

#include <windows.h>
#include <stdbool.h>

typedef enum {
	TRAYICON_CLICKED,
	TRAYICON_EXIT_CLICKED,
	TRAYICON_BEFORE_REMOVE
} TRAYICON_EVENT;

bool trayicon_init(HICON icon, char tooltip[]);
void trayicon_remove();

void trayicon_add_item(char *text, void (functionPtr) ());

#endif
