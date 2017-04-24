char* procSubshells(const char* line) {
	size_t originSize = strlen(line);
	char* newCommand = malloc(sizeof(char) * (originSize + 1));
	strcpy(newCommand, line);

	char* cursor;
	while (cursor = strstr(newCommand, "$(")) {
		char* subcmd = cursor + 2;
		*cursor = '\0';

		if (!(cursor = strstr(subcmd, ")"))) {
			fprintf(stderr, "error: non-terminated subshell");
			return 0;
		}

		cursor += 1;
		*(cursor - 1) = '\0';

		size_t subSize = strlen(subcmd);

		char* subshellDump = 0;
		getExecOutput(subcmd, &subshellDump);
		newlineToSpace(subshellDump);

		size_t tempSize = (originSize - subSize + strlen(subshellDump));
		char* temp = malloc(sizeof(char) * (tempSize + 1));
		sprintf(temp, "%s%s%s", newCommand, subshellDump, cursor);

		free(newCommand);
		newCommand = temp;

	}
	printf("%s\n", newCommand);
	return newCommand;
}

int execSubshell(char **args) {
	pid_t pid, wpid;
	int status;
	char** args1 = NULL;
	splitLine(args, &args1, " ");

	pid = fork();
	if (pid == 0) {
		char *progPath = getenv(args1[0]);
		if (!progPath) {
			progPath = args1[0];
		}
		if (execvp(progPath, args1) == -1) {
			printf("fail\n");
			exit(EXIT_FAILURE);
		}
		exit(1);
	} else if (pid < 0) {
		printf("Error, fork\n");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	fflush(stdout);
	fflush(stdin);

	return 1;
}

int getExecOutput(char* cmd, char** output) {
	char** command;
	splitLine(cmd, &command, " ");

	// save output by using pipe
	int fd[2];
	int status;
	pid_t cpid;

	if (pipe(fd) == -1) {
		perror("pipe error");
		return 0;
	}

	cpid = fork();
	if (cpid < 0) {
		perror("fork error\n");
		return 1;
	}

	if (cpid == 0) {
		dup2(fd[STDOUT_FILENO], STDOUT_FILENO);
		close(fd[STDIN_FILENO]);
		close(fd[STDOUT_FILENO]);
		char *progPath = getenv(command[0]);
		if (!progPath) {
			progPath = command[0];
		}
		if (execvp(progPath, command) == -1) {
			fprintf(stderr, "exec: failed to execute \"%s\"\n", cmd);
			exit(EXIT_FAILURE);
		}
		exit(1);
	}

	else if (cpid > 0) {
		close(fd[1]);
		waitpid(cpid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
			return 1;
		}
		*output = malloc(sizeof(char) * (4096 + 1));

		int numBytes = read(fd[0], *output, 4096);
		if (numBytes == 4096) {
			fprintf(stderr, "Buffer overflow\n");
			return 1;
		}
		(*output)[numBytes] = '\0';
	}

	close(fd[STDIN_FILENO]);
	close(fd[STDOUT_FILENO]);
	fflush(stdout);
	fflush(stdin);
}

