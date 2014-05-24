#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct DictionaryEntry
{
	uint32_t dataoffset,nextoffset;
} DictionaryEntry;

typedef struct DictionaryLookup
{
	uint8_t *buf;
	uint32_t size;
	DictionaryEntry *entries;
	uint32_t offsets[65536];
} DictionaryLookup;

void InitDictionaryLookup(DictionaryLookup *self,uint8_t *buf,uint32_t size);
void CleanupDictionaryLookup(DictionaryLookup *self);
bool FindDictionaryMatch(DictionaryLookup *self,int start,int *length,int *offs);

#endif
