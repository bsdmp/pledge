#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

void
usage(void)
{
	fprintf(stderr, "%s: [-p path mode ...] <-P \"promises\"> "
	    "<program> [arguments]\n", getprogname());
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
				usage();
			Pflag = 1;
			strlcpy(promises, argv[++i], sizeof(promises));
			break;
		}
		if (strcmp(argv[i], "-p") == 0) {
			if (pflag)
				usage();
			pflag = 1;
			i++;
			j = 0;
			while (argv[i] != NULL && strcmp(argv[i], "-P") != 0) {
				paths = reallocarray(paths, j+1,
				    sizeof(*paths));
				modes = reallocarray(modes, j+1,
				    sizeof(*modes));
				paths[j] = strdup(argv[i++]);
				/* We've advanced 'i', check if it's not EOL */
				if (argv[i] == NULL)
					usage();
				modes[j++] = strdup(argv[i++]);
			}
			/* If we have no args after -p */
			if (j == 0)
				usage();
			/* Last cycle advanced i to -P, shift it back */
			i--;
			continue;
		}
	}

	/* -P is mandatory */
	if (Pflag == 0)
		usage();

	if (pledge("stdio exec unveil", promises) == -1)
		err(1, "pledge");

	if (pflag) {
		for (k = 0; k < j; k++) {
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
