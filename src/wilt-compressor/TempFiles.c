#include "TempFiles.h"




void WriteTempByte(FILE *tmpfh,uint8_t val)
{
	fwrite(&val,1,1,tmpfh);
}

uint8_t ReadTempByte(FILE *tmpfh)
{
	return fgetc(tmpfh);
}




void WriteTempInteger(FILE *tmpfh,uint32_t val)
{
	fwrite(&val,1,4,tmpfh);
}

uint32_t ReadTempInteger(FILE *tmpfh)
{
	uint32_t val;
	fread(&val,1,4,tmpfh);
	return val;
}




void InitBitState(BitState *state)
{
	state->byte=0;
	state->bitnum=0;
}

void WriteTempBit(FILE *tmpfh,BitState *state,int bit)
{
	state->byte|=bit<<state->bitnum;
	state->bitnum=(state->bitnum+1)&7;

	if(state->bitnum==0)
	{
		WriteTempByte(tmpfh,state->byte);
		state->byte=0;
	}
}

int ReadTempBit(FILE *tmpfh,BitState *state)
{
	if(state->bitnum==0) state->byte=ReadTempByte(tmpfh);

	int bit=(state->byte>>state->bitnum)&1;
	state->bitnum=(state->bitnum+1)&7;

	return bit;
}

void FlushBitState(FILE *tmpfh,BitState *state)
{
	if(state->bitnum!=0)  WriteTempByte(tmpfh,state->byte);
}
