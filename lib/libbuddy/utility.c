#include <utility.h>
#include <stdio.h>
#include <string.h>

static FILE *pDebugFile = NULL;

/** @brief opens a debug file that will be written to by subsequent
 *			debug_write calls.
 *  @param output_file null terminated string for file to open
 *  @return 0 on sucess, -1 on failure.
*/
int debug_init(char *output_file)
{
	pDebugFile = fopen(output_file, "w");

	if (!pDebugFile) {
		return -1;
	}

	return 0;
}

/** @brief writes a debug message to the file previously open with debug_init
 *			function.
 *  @param message null terminated string with message to be written to the
 *			debug file.
 *  @return Void.
*/
void debug_write(char *message)
{
	fwrite(message, sizeof(char), strlen(message), pDebugFile);
	fflush(pDebugFile);
}

/** @brief closes the debug file handle.
 *  @return Void.
*/
void debug_cleanup(void)
{
	fclose(pDebugFile);
}