#include "RangeCoder.h"

#include <math.h>



void InitRangeEncoder(RangeEncoder *self,FILE *fh)
{
	self->range=0xffffffff;
	self->low=0;
	self->cache=0xff; // TODO: Is this right? Original has cache=0 and cachesize=1,
	self->cachesize=0; // and output a useless 0 byte at the start.

	self->fh=fh;
}

static void ShiftOutputFromRangeEncoder(RangeEncoder *self)
{
	if((self->low&0xffffffff)<0xff000000||(self->low>>32)!=0)
	{
		uint8_t temp=self->cache;
		for(int i=0;i<self->cachesize;i++)
		{
			fputc((temp+(self->low>>32))&0xff,self->fh);
			temp=0xff;
		}
		self->cachesize=0;
		self->cache=(self->low>>24)&0xff;
	}
	self->cachesize++;
	self->low=(self->low<<8)&0xffffffff;
}

void WriteBitAndUpdateWeight(RangeEncoder *self,int bit,uint16_t *weight,int shift)
{
	uint32_t threshold=(self->range>>12)*(*weight);

	if(bit==0)
	{
		self->range=threshold;
		*weight+=(0x1000-*weight)>>shift;
	}
	else
	{
		self->range-=threshold;
		self->low+=threshold;
		*weight-=*weight>>shift;
	}

	while(self->range<0x1000000)
	{
		self->range<<=8;
		ShiftOutputFromRangeEncoder(self);
	}
}

void WriteUniversalCode(RangeEncoder *self,uint32_t value,uint16_t *weights1,int shift1,uint16_t *weights2,int shift2)
{
	int maxbit=31;
	while(maxbit>=0 && (value>>maxbit&1)==0) maxbit--;

	for(int i=0;i<=maxbit;i++) WriteBitAndUpdateWeight(self,1,&weights1[i],shift1);
	WriteBitAndUpdateWeight(self,0,&weights1[maxbit+1],shift1);

	for(int i=maxbit-1;i>=0;i--) WriteBitAndUpdateWeight(self,(value>>i)&1,&weights2[i],shift2);
}

void FinishRangeEncoder(RangeEncoder *self)
{
	for(int i=0;i<5;i++)
	{
		ShiftOutputFromRangeEncoder(self);
	}
}




double CalculateCostOfBit(int bit,uint16_t *weight,int shift,bool updateweight)
{
	double cost;
	if(bit==0)
	{
		cost=log2((double)0x1000/(double)*weight);
		if(updateweight) *weight+=(0x1000-*weight)>>shift;
	}
	else
	{
		cost=log2((double)0x1000/(double)(0x1000-*weight));
		if(updateweight) *weight-=*weight>>shift;
	}
	return cost;
}

double CalculateCostOfUniversalCode(uint32_t value,uint16_t *weights1,int shift1,uint16_t *weights2,int shift2,bool updateweight)
{
	int maxbit=31;
	while(maxbit>=0 && (value>>maxbit&1)==0) maxbit--;

	double cost=0;

	for(int i=0;i<=maxbit;i++) cost+=CalculateCostOfBit(1,&weights1[i],shift1,updateweight);
	cost+=CalculateCostOfBit(0,&weights1[maxbit+1],shift1,updateweight);

	for(int i=maxbit-1;i>=0;i--) cost+=CalculateCostOfBit((value>>i)&1,&weights2[i],shift2,updateweight);

	return cost;
}
