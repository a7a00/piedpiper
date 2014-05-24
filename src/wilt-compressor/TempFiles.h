#ifndef __TEMPFILES_H__
#define __TEMPFILES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void WriteTempByte(FILE *tmpfh,uint8_t val);
uint8_t ReadTempByte(FILE *tmpfh);
void WriteTempInteger(FILE *tmpfh,uint32_t val);
uint32_t ReadTempInteger(FILE *tmpfh);

typedef struct BitState
{
	uint8_t byte,bitnum;
} BitState;

void InitBitState(BitState *state);
void WriteTempBit(FILE *tmpfh,BitState *state,int bit);
int ReadTempBit(FILE *tmpfh,BitState *state);
void FlushBitState(FILE *tmpfh,BitState *state);

#endif
