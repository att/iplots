#include "AObject.h"

/* simple autorelease pool implementation - static list */
int AObject::arpe;
AObject *AObject::arp[1024];
