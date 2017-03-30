#include <stdlib.h>
#include <stdio.h>


void mysh_loop(void)
{
	char *line;
	char **args;
	int status;

	do {
		printf("$ \n");
		line = mysh_read_line();
		args = mysh_split_line(line);
		status = lsh_execute(args);
		free(line);
		free(args);
	} while (status);
}

// Implement read line, split line, execute....


int main(int argc, char const *argv[])
{
	// Load any config files

	// Run command loop
	mysh_loop();

	// Perform shutdown/cleanup

	return EXIT_SUCCESS;
}