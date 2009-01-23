#include "AObject.h"

/* simple autorelease pool implementation - static list */
int AObject::arpe;
AObject *AObject::arp[1024];

/* buffer for describe() */
char AObject::desc_buf[512];

#ifdef ODEBUG
/* global serial number - increased for each created object to uniquely
   identify it in the app */
object_serial_t _globalObjectSerial = 1;
#endif
