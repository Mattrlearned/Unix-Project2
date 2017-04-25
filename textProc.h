void removeChar(char *str, char toRemove) {
	char *old, *new;
	for (old = new = str; *old != '\0'; old++) {
		*new = *old;
		if (*new != toRemove)
			new++;
	}
	*new = '\0';
}

void newlineToSpace(char* str) {
	while (*str != '\0') {
		if (*str == '\n') {
			*str = ' ';
			str++;
		} else {
			str++;
		}
	}
}

int numDelims(char* str, char* delim) {
	int num = 0;
	char* c;
	for (c = delim; *delim; ++delim) {
		while (*str) {
			if (*str == *c)
				num++;
			str++;
		}
	}
	return num;
}

void splitLine(char* line, char*** cmdStr, char* delims) {
	*cmdStr = malloc(sizeof(char*) * (numDelims(line, delims) + 2));
	if (*cmdStr) {
		int i = 0;
		char* token = strtok(line, delims);
		while (token != NULL) {
			if (!strstr(token, "|")) {
				(*cmdStr)[i++] = strdup(token);
			}
			token = strtok(NULL, delims);
		}
		(*cmdStr)[i] = '\0';
	}
}


void getRedir(char* line, char*** cmdStr, char* delims, char** fileIn, char** fileOut, int in, int out) {
	*cmdStr = malloc(sizeof(char*) * (numDelims(line, delims) + 2));
	if (*cmdStr) {
		int i = 0;
                int inDir =0;
                int outDir =0;
		char* token = strtok(line, delims);
		while (token != NULL) {
                   if(strstr(token, "<")){
                      *fileIn= strtok(NULL, delims);
                      in =1;
                    }
                    else if(strstr(token, ">")){
                       *fileOut = strtok(NULL, delims);
                       in =2;
                    }
                    else if (!strstr(token, "<") || !strstr(token, ">")) {
				(*cmdStr)[i++] = strdup(token);
			}
                     else{}
		     token = strtok(NULL, delims);
		}
		(*cmdStr)[i] = '\0';
	}
        
}

