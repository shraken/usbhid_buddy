#include <utility.h>
#include <stdio.h>
#include <string.h>

static FILE *pDebugFile = NULL;

int debug_init(char *output_file)
{
	pDebugFile = fopen(output_file, "w");

	if (!pDebugFile) {
		return -1;
	}

	return 0;
}

void debug_write(char *message)
{
	fwrite(message, sizeof(char), strlen(message), pDebugFile);
	fflush(pDebugFile);
}

void debug_cleanup(void)
{
	fclose(pDebugFile);
}