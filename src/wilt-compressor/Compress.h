#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

int FindMatches(uint8_t *buf,uint32_t size,
FILE *tmptypes,FILE *tmpliterals,FILE *tmplengths,FILE *tmpoffsets,
int *literalcount,int *matchcount);

double CalculateCostOfBits(FILE *bits,int count,int shift);
double CalculateCostOfLiterals(FILE *bytes,int count,int shift);
double CalculateCostOfIntegers(FILE *ints,int count,int shift1,int shift2);

void WriteCompressedData(FILE *fh,FILE *tmptypes,FILE *tmpliterals,FILE *tmplengths,FILE *tmpoffsets,
int count,int typeshift,int litshift,int lengthshift1,int lengthshift2,int offsshift1,int offsshift2);

#endif
