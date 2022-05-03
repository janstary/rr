#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <err.h>
#include "rr.h"

int qflag = 0;
int vflag = 0;

void
usage()
{
	warnx("usage: rr [-qv] inst.rm");
}

struct inst*
mkinst(char* line)
{
	int r;
	char i;
	struct inst* inst = NULL;
	if (sscanf(line, "%c%d", &i, &r) == 2) switch (i) {
		case 'i':
		case 'd':
		case 'j':
			break;
		default:
			warnx("'%c' is not an instruction", i);
			return NULL;
	} else {
		warnx("cannot parse '%s'", line);
		return NULL;
	}
	if ((inst = calloc(1, sizeof(struct inst))) == NULL)
		err(1, NULL);
	inst->i = i;
	inst->r = r;
	return inst;
}

void
prinst(struct inst* inst)
{
	if (inst)
		printf("%c%d\n", inst->i, inst->r);
}

int
addinst(struct rm* rm, char* line)
{
	struct inst* inst;
	if (rm == NULL)
		return -1;
	if ((inst = mkinst(line)) == NULL)
		return -1;
	rm->inst[rm->numi++] = *inst;
	free(inst);
	return 0;
}

void
print(struct rm* rm)
{
	unsigned i;
	if (rm) {
		for (i = 0; i < rm->numi; i++)
			prinst(&rm->inst[i]);
		for (i = 0; i < rm->numr; i++)
			printf("%u ", rm->regs[i]);
		putchar('\n');
	}
}

struct rm*
parse(const char* file)
{
	FILE* f;
	ssize_t len;
	size_t size = 0;
	char* line = NULL;
	struct rm* rm = NULL;
	if ((rm = calloc(1, sizeof(struct rm))) == NULL)
		err(1, NULL);
	if ((f = fopen(file, "r")) == NULL)
		err(1, "%s", file);
	while ((len = getline(&line, &size, f)) != -1) {
		if (len == 1)
			continue;
		if (addinst(rm, line) == -1) {
			warnx("Error adding instruction: %s", line);
			goto bad;
		}
	}
	fclose(f);
	free(line);
	return rm;
bad:
	fclose(f);
	free(rm);
	return NULL;
}

int
init(struct rm* rm, char* line)
{
	char *n = NULL;
	const char *e = NULL;
	if (rm == NULL || line == NULL)
		return -1;
	rm->numr = 1;
	rm->regs[0] = 0;
	while ((rm->numr < 1024) && (n = strsep(&line, " \t\n"))) {
		rm->regs[rm->numr++] =  strtonum(n, 0, 0xffffffffU, &e);
           	if (e) {
			warnx("%s is %s", n, e);
			return -1;
		}
	}
	if (rm->numr == 1024) {
		warnx("1024 registers exhausted");
		return -1;
	}
	return 0;
}

/*
void
debug(char* expr, char* here, struct rule* rule)
{
	char* c;
	char* tail;
	if (expr == NULL)
		return;
	if (here && rule) {
		for (c = expr; *c && c < here ; )
			putchar(*c++);
		tail = c + rule->llen;
		if (isatty(1)) {
			printf(
				"\033[4m%s\033[24m",
				*rule->left ? rule->left : "_");
		} else {
			if (*rule->left) {
				for (c = rule->left; *c ; c++)
					printf("%c%c%c", *c, 0x08, '_');
			} else {
				printf("%c%c%c", '_', 0x08, '_');
			}
		}
		for (c = tail; *c; )
			putchar(*c++);
		printf("\t%s", *rule->right ? rule->right : "_");
	} else {
		printf("%s", expr);
	}
	putchar('\n');
}
*/

void
run(struct rm* rm)
{
	int j = 0, p = 0;
	if (rm == NULL)
		return;
	while (1) {
		if (vflag) {
			/* FIXME */
		}
		switch (rm->inst[p].i) {
			case 'i':
				rm->regs[rm->inst[p].r]++;
				p += 1;
				break;
			case 'd':
				if (rm->regs[rm->inst[p].r] > 0) {
					rm->regs[rm->inst[p].r]--;
					p += 2;
				} else {
					p += 1;
				}
				break;
			case 'j':
				j = rm->inst[p].r;
				if (p + j >= rm->numi ||  p + j < 0)
					return;
				p += j;
				break;
		}
		if (p >= rm->numi)
			return;
	}
}

int
main(int argc, char** argv)
{
	int c;
	ssize_t len;
	size_t size = 0;
	char* line = NULL;
	struct rm* rm;

	while ((c = getopt(argc, argv, "qv")) != -1) switch (c) {
		case 'q':
			qflag = 1;
			vflag = 0;
			break;
		case 'v':
			qflag = 0;
			vflag = 1;
			break;
		default:
			usage();
			return 1;
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage();
		return 1;
	}

	if ((rm = parse(*argv)) == NULL)
		return 1;

	while ((len = getline(&line, &size, stdin)) != -1) {
		line[--len] = '\0'; /* newline */
		if (*line == '\0')
			continue;
		if (init(rm, line) == -1) {
			warnx("cannot init registers");
			continue;
		}
		run(rm);
		if (qflag == 0)
			printf("%u\n", rm->regs[0]);
	}

	free(line);
	free(rm);
	return 0;
}
