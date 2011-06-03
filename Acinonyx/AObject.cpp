#include "AObject.h"

#ifdef AUTORELEASE_SUPPORT
/* simple autorelease pool implementation - static list */
int AObject::arpe;
AObject *AObject::arp[1024];
#endif

/* buffer for describe() */
char AObject::desc_buf[512];

#ifdef ODEBUG
/* global serial number - increased for each created object to uniquely
   identify it in the app */
object_serial_t _globalObjectSerial = 1;

#define A_alloc_sentinel0 0x55aa33cc
#define A_alloc_sentinel1 0x6699a55a



// NOTE: we can't use describe() in A_alloc since the owner may not be initialized! Must use _ptr_describe instead!
void* A_alloc(vsize_t size, vsize_t elt_size, AObject *owner) {
	vsize_t len = size * elt_size;
	void *mem = malloc(64 + len);
	if (!mem) {
		AError("***ERROR: alloc of %ld bytes failed!", (long) (len + 64));
		return NULL;
	}
	char *memc = (char*) mem;
	void **memO = (void**) mem;
	int *memI = (int*) mem;
	vsize_t *memV = (vsize_t*) mem;
	int s1 = A_alloc_sentinel1;
	memI[0] = A_alloc_sentinel0;
	memV[1] = len;
	memO[2] = owner;
	memcpy(memc + 32 + len, &s1, sizeof(s1));
	ALog("A_alloc:%p (%d) for %s", mem, len, owner ? owner->_ptr_describe() : "<unassigned>");
	return memc + 32;
}

// this is less efficient than the usual AZAlloc since it actually accesses the memory, but it's for debugging after all ..
void* A_calloc(vsize_t size, vsize_t elt_size, AObject *owner) {
	void *mem = A_alloc(size, elt_size, owner);
	memset(mem, 0, size * elt_size);
	return mem;
}

void* A_realloc(void *ptr, vsize_t size, AObject *owner) {
	char *memc = ((char*) ptr) - 32;
	vsize_t *memV = (vsize_t*) memc;
	// void **memO = (void**) memc;
	int *memI = (int*) memc;
	if (*memI != A_alloc_sentinel0)
		fprintf(stderr, "ERROR: A_realloc(%p) - pointer has no head sentinel! Possible memory corruption!", ptr);
	void *tail = memc + 32 + memV[1];
	int s1 = A_alloc_sentinel1;
	if (memcmp(tail, &s1, sizeof(s1)))
		AError("ERROR: A_realloc(%p) - tail sentinel (size=%ld) missing or corrupted!", ptr, (long) memV[1]);
	void *mem = realloc(memc, size + 64);
	if (!mem) {
		AError("***ERROR: realloc (for %p to %ld) failed!", ptr, (long) (size + 64));
		return NULL;
	}
	memc = (char*) mem;
	tail = memc + 32 + size;
	memcpy(tail, &s1, sizeof(s1));
	return memc + 32;
}

void A_free(void *ptr) {
	char *memc = ((char*) ptr) - 32;
	vsize_t *memV = (vsize_t*) memc;
	// void **memO = (void**) memc;
	int *memI = (int*) memc;
	if (*memI != A_alloc_sentinel0)
		AError("ERROR: Afree(%p) - pointer has no head sentinel! Possible memory corruption!", ptr);
	void *tail = memc + 32 + memV[1];
	int s1 = A_alloc_sentinel1;
	if (memcmp(tail, &s1, sizeof(s1)))
		AError("ERROR: Afree(%p) - tail sentinel missing or corrupted!", ptr);
	ALog("A_free:%x (%ld)", ptr, (long) memV[1]);
	free(memc);
}

void A_transfer(void *ptr, AObject *obj) {
	char *memc = ((char*) ptr) - 32;
	vsize_t *memV = (vsize_t*) memc;
	AObject **memO = (AObject**) memc;
	int *memI = (int*) memc;
	if (*memI != A_alloc_sentinel0)
		AError("ERROR: A_transfer(%p) - pointer has no head sentinel! Possible memory corruption!", ptr);
	void *tail = memc + 32 + memV[1];
	int s1 = A_alloc_sentinel1;
	if (memcmp(tail, &s1, sizeof(s1)))
		AError("ERROR: A_transfer(%p) - tail sentinel missing or corrupted!", ptr);
	// AObject *prev = memO[2];
	memO[2] = obj;
	ALog("A_transfer:%x (%d) transfer to %s", ptr, *memI, obj ? obj->_ptr_describe() : "<undefined>");
}

void* A_memdup(const void *ptr, vsize_t len, AObject *owner) {
	void *mem = A_alloc(len, 1, owner);
	AMEM(mem);
	memcpy(mem, ptr, len);
	return mem;
}

char* A_strdup(const char *str, AObject *owner) {
	vsize_t len = strlen(str);
	char *ns = (char*) A_alloc(len + 1, 1, owner);
	AMEM(ns);
	strcpy(ns, str);
	return ns;
}

#else

void* A_memdup(const void *ptr, vsize_t len, AObject *owner) {
	void *mem = malloc(len);
	AMEM(mem);
	memcpy(mem, ptr, len);
	return mem;
}

char* A_strdup(const char *str, AObject *owner) {
	vsize_t len = strlen(str);
	char *ns = (char*) malloc(len + 1);
	AMEM(ns);
	strcpy(ns, str);
	return ns;
}

#endif

void AObject_retain(void *o)
{
       ((AObject*)o)->retain();
}

void AObject_release(void *o)
{
       ((AObject*)o)->release();
}
