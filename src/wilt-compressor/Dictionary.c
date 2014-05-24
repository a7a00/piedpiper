#include "Dictionary.h"

#include <stdlib.h>
#include <string.h>


static inline uint16_t GetUInt16LE(uint8_t *ptr)
{
	return ptr[0]+(ptr[1]<<8);
}

void InitDictionaryLookup(DictionaryLookup *self,uint8_t *buf,uint32_t size)
{
	self->buf=buf;
	self->size=size;
	self->entries=malloc(size*sizeof(DictionaryEntry));
	memset(self->offsets,0xff,sizeof(self->offsets));

	for(int i=size-3;i>=0;i--)
	{
		uint16_t val=GetUInt16LE(&buf[i]);

		DictionaryEntry *entry=&self->entries[i];
		entry->dataoffset=i;
		entry->nextoffset=self->offsets[val];
		self->offsets[val]=i;
	}
}

void CleanupDictionaryLookup(DictionaryLookup *self)
{
	free(self->entries);
}

bool FindDictionaryMatch(DictionaryLookup *self,int start,int *length,int *offs)
{
	int maxlength=0,maxpos=-1;

	uint16_t first=GetUInt16LE(&self->buf[start]);
	uint32_t entryoffset=self->offsets[first];

	while(entryoffset!=0xffffffff && self->entries[entryoffset].dataoffset<start)
	{
		int pos=self->entries[entryoffset].dataoffset;
		int matchlen=2;
		while(pos+matchlen+2<=self->size && start+matchlen+1<=self->size
		&& self->buf[pos+matchlen]==self->buf[start+matchlen]) matchlen+=1;

		if(matchlen>=maxlength) // Use >= to capture the *last* hit for multiples.
		{
			maxlength=matchlen;
			maxpos=pos;
		}

		entryoffset=self->entries[entryoffset].nextoffset;
	}

	if(maxlength>=3)
	{
		*length=maxlength;
		*offs=maxpos;
		return true;
	}
	else return false;
}
