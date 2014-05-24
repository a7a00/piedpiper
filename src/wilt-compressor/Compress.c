#include "Compress.h"
#include "RangeCoder.h"
#include "Dictionary.h"
#include "TempFiles.h"




int FindMatches(uint8_t *buf,uint32_t size,
FILE *tmptypes,FILE *tmpliterals,FILE *tmplengths,FILE *tmpoffsets,
int *literalcount,int *matchcount)
{
	DictionaryLookup dict;
	InitDictionaryLookup(&dict,buf,size);

	BitState typestate;
	InitBitState(&typestate);

	*literalcount=0;
	*matchcount=0;

	uint32_t pos=0;
	while(pos<size)
	{
		int length,offs;
		if(FindDictionaryMatch(&dict,pos,&length,&offs))
		{
			WriteTempBit(tmptypes,&typestate,1);

			WriteTempInteger(tmplengths,length-3);
			WriteTempInteger(tmpoffsets,(pos-offs)-1);

			(*matchcount)++;
			pos+=length;
		}
		else
		{
			WriteTempBit(tmptypes,&typestate,0);

			uint8_t val=buf[pos];
			WriteTempByte(tmpliterals,val);

			(*literalcount)++;
			pos+=1;
		}
	}

	CleanupDictionaryLookup(&dict);

	FlushBitState(tmptypes,&typestate);
}



double CalculateCostOfBits(FILE *bits,int count,int shift)
{
	uint16_t weight=0x800;

	BitState bitstate;
	InitBitState(&bitstate);

	double cost=0;
	for(int i=0;i<count;i++)
	{
		int bit=ReadTempBit(bits,&bitstate);
		cost+=CalculateCostOfBit(bit,&weight,shift,true);
	}
	return cost;
}

double CalculateCostOfLiterals(FILE *bytes,int count,int shift)
{
	uint16_t literalbitweights[256];
	for(int i=0;i<256;i++) literalbitweights[i]=0x800;

	double cost=0;
	for(int i=0;i<count;i++)
	{
		uint8_t val=ReadTempByte(bytes);
		for(int i=7;i>=0;i--)
		cost+=CalculateCostOfBit((val>>i)&1,&literalbitweights[(val|0x100)>>(i+1)],shift,true);
	}

	return cost;
}

double CalculateCostOfIntegers(FILE *ints,int count,int shift1,int shift2)
{
	uint16_t weights1[32],weights2[32];
	for(int i=0;i<32;i++) weights1[i]=weights2[i]=0x800;

	double cost=0;
	for(int i=0;i<count;i++)
	{
		uint32_t val=ReadTempInteger(ints);
		cost+=CalculateCostOfUniversalCode(val,weights1,shift1,weights2,shift2,true);
	}

	return cost;
}



void WriteCompressedData(FILE *fh,FILE *tmptypes,FILE *tmpliterals,FILE *tmplengths,FILE *tmpoffsets,
int count,int typeshift,int literalshift,int lengthshift1,int lengthshift2,int offsetshift1,int offsetshift2)
{
	RangeEncoder comp;
	InitRangeEncoder(&comp,fh);

	BitState typestate;
	InitBitState(&typestate);

	uint16_t typeweight=0x800;

	uint16_t lengthweights1[32],lengthweights2[32];
	uint16_t offsetweights1[32],offsetweights2[32];
	for(int i=0;i<32;i++)
	lengthweights1[i]=lengthweights2[i]=offsetweights1[i]=offsetweights2[i]=0x800;

	uint16_t literalbitweights[256];
	for(int i=0;i<256;i++) literalbitweights[i]=0x800;

	for(int i=0;i<count;i++)
	{
		if(ReadTempBit(tmptypes,&typestate)==1)
		{
			WriteBitAndUpdateWeight(&comp,1,&typeweight,typeshift);
			WriteUniversalCode(&comp,ReadTempInteger(tmplengths),lengthweights1,lengthshift1,lengthweights2,lengthshift2);
			WriteUniversalCode(&comp,ReadTempInteger(tmpoffsets),offsetweights1,offsetshift1,offsetweights2,offsetshift2);
		}
		else
		{
			WriteBitAndUpdateWeight(&comp,0,&typeweight,typeshift);

			uint8_t val=ReadTempByte(tmpliterals);
			for(int i=7;i>=0;i--)
			WriteBitAndUpdateWeight(&comp,(val>>i)&1,&literalbitweights[(val|0x100)>>(i+1)],literalshift);
		}
	}

	FinishRangeEncoder(&comp);
}
