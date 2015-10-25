/* Normalized Algorithmes of Markov intrepreter*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#define INFO_STR "Normal Algos of Markov interpreter\n-h  Show this help\n-f  Set rules path (Default is rules.txt)\n-i  Set input string\n"
typedef struct
{
	char from[16];
	char to[15];
	char act;
} rule_t;

rule_t rules[64];
#define MAX_RULES 62
int   rules_l = 0;
char *buffer;

void feed_rules(FILE *fp)
{
	char s1[16], s2[16], a;
	int i=0;
	while(fscanf(fp,"%16s %c> %16s", s1, &a, s2)>1 && s1[0])
	{
		if(i<MAX_RULES)
		{
			char *t;
			if(t = strstr(s2,"λ"))
				*t = 0;
			strcpy(rules[i].from, s1);
			strcpy(rules[i].to, s2);
			rules[i++].act = a;
		}
		else
		{
			printf("ERROR: Too much rules.");
			exit(0);
		}
	}
	rules_l = i;
}

void run(void)
{
	printf("%s\n", buffer);
	while(1==1)
	{
		int i;
		for(i=0;i<rules_l;i++)
		{
			char *tmp, *tmp2;
			if((tmp=strstr(buffer, rules[i].from)))
			{

				*tmp = 0;
				tmp2 = strdup(&tmp[strlen(rules[i].from)]);
			       	tmp = strdup(buffer);
				printf(" --> %s\033[32m%s\033[0m%s\n", tmp, rules[i].to, tmp2);
				snprintf(buffer, 80, "%s%s%s%c", tmp, rules[i].to, tmp2,0);
				free(tmp);
				free(tmp2);
				if(rules[i].act == '=')
					return;
				goto _c;
			}
		}
		break;
	_c:
		continue;
	}
}

int main(int argc, char **argv)
{
	int i;
	char *fpath=NULL;
	FILE *fp;
	buffer = malloc(81);
	for(i=1; i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
			if(argv[i][1]=='h')
			{
				printf("%s", INFO_STR);
				return 0;
			}
		else	if(argv[i][1]=='i')
			{
				strncpy(buffer, argv[++i], 80);
			}
		else	if(argv[i][1]=='f')
			{
				fpath = argv[++i];
			}
		}
	}
	if(fpath == NULL)
		fpath = "rules.txt";
	fp = fopen(fpath, "r");
	feed_rules(fp);
	fclose(fp);
	run();
	free(buffer);
	return 0;
}
