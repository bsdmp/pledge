#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <limits.h>

void
usage(int i)
{
	fprintf(stderr, "%s %i: [-p path mode ...] <-P \"promises\"> "
	    "<program> [arguments]\n", getprogname(), i);
	exit(1);
}

int
main(int argc, char **argv)
{
	int i, j, k, Pflag = 0, pflag = 0;
	char promises[1024] = { 0 };
	char **paths = NULL;
	char **modes = NULL;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-P") == 0) {
			if (Pflag)
				usage(0);
			Pflag = 1;
			strlcpy(promises, argv[++i], sizeof(promises));
			break;
		}
		if (strcmp(argv[i], "-p") == 0) {
			if (pflag)
				usage(0);
			pflag = 1;
			if (Pflag) {
				fprintf(stderr, "%s: -P flag must be before "
				    "-p\n", getprogname());
				exit(1);
			}
			i++;
			j = 0;
			while (argv[i] != NULL && strcmp(argv[i], "-P") != 0) {
				paths = reallocarray(paths, j+1,
				    sizeof(*paths));
				modes = reallocarray(modes, j+1,
				    sizeof(*modes));
				//printf("argv[%i]=%s\n", i, argv[i]);
				paths[j] = strdup(argv[i++]);
				//printf("paths[%i]=%s\n", j, paths[j]);
				//printf("argv[%i]=%s\n", i, argv[i]);
				/* We've advanced 'i', check if it's not EOL */
				if (argv[i] == NULL)
					usage(1);
				modes[j++] = strdup(argv[i++]);
				//printf("modes[%i]=%s\n", j, modes[j-1]);
				//printf("===\n");
			}
			/* If we have no args after -p */
			if (j == 0)
				usage(2);
			/* Last cycle advanced i to -P, shift it back */
			i--;
			continue;
		}
	}

	if (Pflag == 0)
		usage(3);

	//printf("---\n");
	if (pledge("stdio exec unveil", promises) == -1)
		err(1, "pledge");

	if (pflag) {
		for (k = 0; k < j; k++) {
			//printf("paths[%i]=%s\n", k, paths[k]);
			//printf("modes[%i]=%s\n", k, modes[k]);
			if (unveil(paths[k], modes[k]) == -1)
				err(1, "unveil: %s mode %s", paths[k],
				    modes[k]);
		}
		unveil(NULL, NULL);
	}

	char *progname = argv[++i];
	char **progargs = &argv[i];
	if (execvp(progname, progargs) == -1)
		err(1, "execvp: %s", progname);

	return (0);
}
