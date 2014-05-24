#include "Compress.h"

#include <stdlib.h>
#include <stdio.h>




uint8_t *AllocAndReadFile(FILE *fh,uint32_t *size)
{
	const int blocksize=4096;
	uint8_t *buf=malloc(blocksize);
	uint32_t total=0;
	for(;;)
	{
		uint32_t actual=fread(buf+total,1,blocksize,fh);
		total+=actual;
		if(actual<blocksize) break;
		buf=realloc(buf,total+blocksize);
	}

	*size=total;
	return buf;
}




typedef double (*CostFunction)(FILE *,int,int);

static int FindOptimalShift(FILE *tmpfh,CostFunction func,int count)
{
	double mincost=1e30;
	int minshift=1;
	for(int shift=1;shift<=12;shift++)
	{
		fseek(tmpfh,0,SEEK_SET);
		double cost=func(tmpfh,count,shift);
		if(cost<mincost)
		{
			minshift=shift;
			mincost=cost;
		}
	}
	return minshift;
}

static double CalculateCostOfIntegers1(FILE *fh,int count,int shift)
{
	return CalculateCostOfIntegers(fh,count,shift,4);
}

static double CalculateCostOfIntegers2(FILE *fh,int count,int shift)
{
	return CalculateCostOfIntegers(fh,count,4,shift);
}




int main(int argc,char **argv)
{
	FILE *in;
	if(argc>=2)
	{
		in=fopen(argv[1],"rb");
		if(!in)
		{
			fprintf(stderr,"Couldn't not read file \"%s\".\n",argv[1]);
			exit(1);
		}
	}
	else in=stdin;

	FILE *out;
	if(argc>=3)
	{
		out=fopen(argv[2],"wb");
		if(!out)
		{
			fprintf(stderr,"Couldn't not create file \"%s\".\n",argv[2]);
			exit(1);
		}
	}
	else out=stdout;

	uint32_t size;
	uint8_t *file=AllocAndReadFile(in,&size);

	FILE *tmptypes=tmpfile();
	FILE *tmpliterals=tmpfile();
	FILE *tmplengths=tmpfile();
	FILE *tmpoffsets=tmpfile();

	int literalcount,matchcount;
	FindMatches(file,size,tmptypes,tmpliterals,tmplengths,tmpoffsets,&literalcount,&matchcount);

	int typeshift=FindOptimalShift(tmptypes,CalculateCostOfBits,literalcount+matchcount);
	int literalshift=FindOptimalShift(tmptypes,CalculateCostOfLiterals,literalcount);
	int lengthshift1=FindOptimalShift(tmplengths,CalculateCostOfIntegers1,matchcount);
	int lengthshift2=FindOptimalShift(tmplengths,CalculateCostOfIntegers2,matchcount);
	int offsetshift1=FindOptimalShift(tmpoffsets,CalculateCostOfIntegers1,matchcount);
	int offsetshift2=FindOptimalShift(tmpoffsets,CalculateCostOfIntegers2,matchcount);

	//fprintf(stderr,"%d %d %d %d %d %d\n",typeshift,literalshift,lengthshift1,lengthshift2,offsetshift1,offsetshift2);

	fseek(tmptypes,0,SEEK_SET);
	fseek(tmpliterals,0,SEEK_SET);
	fseek(tmplengths,0,SEEK_SET);
	fseek(tmpoffsets,0,SEEK_SET);

	fputc(size&0xff,out);
	fputc((size>>8)&0xff,out);
	fputc((size>>16)&0xff,out);
	fputc((size>>24)&0xff,out);

	fputc((offsetshift1<<4)|offsetshift2,out);
	fputc((lengthshift1<<4)|lengthshift2,out);
	fputc((typeshift<<4)|literalshift,out);

	WriteCompressedData(out,tmptypes,tmpliterals,tmplengths,tmpoffsets,
	literalcount+matchcount,typeshift,literalshift,lengthshift1,
	lengthshift2,offsetshift1,offsetshift2);
}
