/* Turing Machine emulator w/ 5 syntax
 * Made for debugging homeworks from MAI about
 * Creating algos for TM.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define USAGE_MSG "TM Emulator (5 syntax)\nUsage:\n  -p   Programm file\n  -i   Input string\n  -io  Input offset\n"

typedef uint32_t cmd_t;
#define CMD_ACT_U 0
#define CMD_ACT_R 0x00010000
#define CMD_ACT_L 0x00020000
#define CMD_ACT_S 0x00030000

static inline uint8_t cmd_qF(cmd_t a)
{
	return (a&0xFE000000) >> 27;
}

static inline uint8_t cmd_qT(cmd_t a)
{
	return (a&0x01FC0000) >> 22;
}

static inline uint32_t cmd_act(cmd_t a)
{
	return  a&0x00030000;
}

static inline uint8_t cmd_cF(cmd_t a)
{
	return (a&0x0000FF00)>>8;
}

static inline int8_t cmd_cT(cmd_t a)
{
	return a&0x000000FF;
}

static inline cmd_t cmd_build(int qF, uint8_t cF, char act, int qT, uint8_t cT)
{
	register cmd_t cmd=0;
	cmd |= (qF & 0x0000007F)<<27;
	cmd |= cF << 8;
	if(act=='s'||act=='S')
		cmd |= CMD_ACT_S;
	else if(act=='r'||act=='R')
		cmd |= CMD_ACT_R;
	else if(act=='l'||act=='L')
		cmd |= CMD_ACT_L;
	else	cmd |= CMD_ACT_U;
	cmd |= (qT & 0x0000007F)<<22;
	cmd |= cT;
	return cmd;
}

int cmd_cmp(const void *a, const void *b)
/*Use it for sorting cmds in qsort. */
{
	uint32_t x, y;
	x = *(uint32_t*)a - 1;	
	y = *(uint32_t*)b - 1;	
	if(x>y)
		return 1;
	else if(x<y)
		return -1;
	else
		return 0;
}

cmd_t *prog; /*List of instructions (sorted after feeding)*/
cmd_t **p_cache; /*p_cache[i] = first cmd w/ qF = i or NULL if no such*/
int prog_size=0, prog_len=0;

struct
{
	uint8_t *f, *b;
	int f_alloced, b_alloced;
	int f_used, b_used;
	int pos;
} data;

uint8_t data_read(void)
{
	int *alloced;
	uint8_t **bank;
	int pos;
	int *used;
	if(data.pos<0)
		pos=-data.pos, alloced = &data.b_alloced, bank = &data.b, used=&data.b_used;
	else	pos= data.pos, alloced = &data.f_alloced, bank = &data.f, used=&data.f_used;

	if(*bank == NULL)
	{
		*bank = malloc(128);
		*alloced = 128;	
	}
/*	else if(pos>=alloced)
	{
		*alloced+=128;
		*bank = realloc(*bank, *alloced);
	} */

	if(pos>=*used)
		return (uint8_t) ' ';
	else
		return (*bank)[pos];
}

void data_write(uint8_t v)
{
	int *alloced;
	uint8_t **bank;
	int pos;
	int *used;
	if(data.pos<0)
		pos=-data.pos, alloced = &data.b_alloced, bank = &data.b, used=&data.b_used;
	else	pos= data.pos, alloced = &data.f_alloced, bank = &data.f, used=&data.f_used;

	if(*bank == NULL)
	{
		*bank = malloc(128);
		*alloced = 128;	
	}
	else if(pos>=*alloced)
	{
		*alloced+=128;
		*bank = realloc(*bank, *alloced);
	}

	(*bank)[pos] = v;
	if(pos>*used)
		*used=pos;
}

void print_line(void)
{
	if(data.b)
	{
		int i;
		printf("Offset:-%i\n", data.b_used);
		for(i=0;i<data.b_used;i++)
			printf("%c",data.b[data.b_used-i]);
	}
	else printf("Offset:0\n");
	if(data.f)
		printf("%s", data.f);

	printf("\n");
}

void feed_program(FILE *in)
{
	int qF, qT;	
	char act, cF, cT;
	if(prog) free(prog);
	if(p_cache) free(p_cache);
	prog = calloc(256,sizeof(cmd_t));
	p_cache = calloc(128,sizeof(void*));
	prog_len = 0; prog_size = 256;
	/*(1) read em*/
	while(/*!feof(in) &&*/ fscanf(in,"%i,%c,%c,%c,%i",&qF,&cF,&cT,&act,&qT)==5)
	{
		cmd_t cmd = cmd_build(qF,cF,act,qT,cT);
		if(prog_len >= prog_size)
			prog_size+=256, prog = realloc(prog,prog_size*sizeof(cmd_t));
		prog[prog_len++]=cmd;
	}
	/*(2) sort em*/
	qsort(prog,prog_len,sizeof(cmd_t), cmd_cmp);
	/*(3) make cache*/
	{
		int i;
		qF=300;
		for(i=0;i<prog_len;i++)
			if(cmd_qF(prog[i])!=qF)
				qF = cmd_qF(prog[i]),
				p_cache[qF] = &prog[i];   
	}
}

void run(void)
{
	uint8_t c;
	int q=0;
	int st=0;
	int it=0;
	cmd_t *cptr;
	while(1)
	{
		it++;
		c = data_read();
		if(p_cache[q]==NULL)
		{
			st=1;
		       	break;
		}
		cptr=p_cache[q];
		while(cmd_qF(*cptr)==q && cmd_cF(*cptr)!=c)cptr++;

		if(cmd_qF(*cptr)==q)
		{
			q=cmd_qT(*cptr);
			data_write(cmd_cT(*cptr));
			if(cmd_act(*cptr)==CMD_ACT_S)
				break;
			else if(cmd_act(*cptr)==CMD_ACT_L)
				data.pos--;
			else if(cmd_act(*cptr)==CMD_ACT_R)
				data.pos++;
		}
		else 
		{
			st=1;
		       	break;
		}
	}
	if(st==1)
		printf("ERROR: No command. Q = %i, C = `%c`\n", q, c);
	else
		printf("Done.\n");
	printf("Count of iterations:%i\n", it);
}

int main(int argc, char **argv)
{
	int i;
	int in_offset=0;
	char *in_fill=NULL;
	FILE *pfile;
	pfile = stdin;
	for(i=1;i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
			if(argv[i][1]=='p')
				pfile = fopen(argv[++i],"r");
			else if(argv[i][1]=='i' && argv[i][2]=='o')
				sscanf(argv[++i], "%i", &in_offset);
			else if(argv[i][1]=='i' && argv[i][2]==0)
				in_fill=argv[++i];
			else if(argv[i][1]=='h')
			{
				printf(USAGE_MSG);
				return 0;
			}
		}
	}
	if(in_fill)
	{
		data.pos = in_offset;
		while(*in_fill)
		{
			data_write(*in_fill);
			in_fill++;
			data.pos++;
		}
	}
	if(!pfile)
	{
		printf("Cant open program file.\n");
		return 0;
	}
	feed_program(pfile);
	run();
	print_line();
	return 0;
}
