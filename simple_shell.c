#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"

char* read_line() {
	size_t size = 20;
	size_t length = 0;
	int ch;
	char* str;
	str = malloc(sizeof * str * size);
	if (!str) {
		fprintf(stderr, "shell: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		ch = getchar();

		if (ch == '\n' || ch == EOF) {
			str[length++] = '\0';
			return str;
		}
		else {
			str[length++] = ch;
			if (length == size) {
				str = realloc(str, sizeof * str * (size *= 2));
				if (!str) {
					fprintf(stderr, "shell: memory allocation failed\n");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}

char** parse_line(const char* str, FILE* errf) {
	size_t num_args = 5; // Set default size
	char** args = malloc(sizeof * args * num_args);
	size_t i = 0;

	if (!args) {
		fprintf(errf, "shell: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	int space = first_unquoted_space(str);
	int prev_space = -1;

	while (space != -1) {
		// Copy contents into corresponding argument
		args[i] = malloc(sizeof(char) * space + 1);
		if (!args[i]) {
			fprintf(errf, "shell: memory allocation failed\n");
			exit(EXIT_FAILURE);
		}
		int j;
		for (j = 0; j < space; ++j) {
			args[i][j] = str[j + prev_space + 1];
		}
		args[i++][j] = NULL;

		if (i == num_args) {
			args = realloc(args, sizeof * args * (num_args *= 2));
			if (!args) {
				fprintf(errf, "shell: memory allocation failed\n");
				exit(EXIT_FAILURE);
			}
		}

		space = space + prev_space + 1;
		prev_space = space;
		space = first_unquoted_space(str + prev_space + 1);
	}

	// Copy the last portion of the string
	size_t rem_len = strlen(str) - prev_space;
	args[i] = malloc(sizeof(char) * rem_len + 2);
	if (!args[i]) {
		fprintf(errf, "shell: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	int j = 0;
	while (str[j + prev_space + 1] != NULL) {
		args[i][j] = str[j + prev_space + 1];
		++j;
	}
	args[i++][j] = NULL;

	if (i == num_args) {
		args = realloc(args, sizeof * args * (num_args *= 2));
		if (!args) {
			fprintf(stderr, "shell: memory allocation failed\n");
			exit(EXIT_FAILURE);
		}
	}
	args[i++] = NULL;

	// Clean up the arguments
	char** clean_args = malloc(sizeof * clean_args * i);
	i = 0;
	while (args[i]) {
		clean_args[i] = unescape(args[i], stderr);
		free(args[i++]);
	}
	clean_args[i] = NULL;
	free(args);
	printf("\n");

	return clean_args;
}

// int mode: 0 -- executing from prompt, 1 -- from file
int execute_command(char** args, FILE* errf, int mode) {
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(errf, "shell: fork() failed\n");
		return 1;
	}
	else if (pid == 0) {
		// Child process
		if (execvp(args[0], args) < 0) {
			fprintf(errf, "shell: execute failed\n");
		}

		_exit(1);
	}
	else {
		// Parent process (edit code)
		int status;
		wait(&status);
		if (WIFEXITED(status)) {
			int returned = WEXITSTATUS(status);
			if (returned == 1 && mode == 1) {
				exit(1);
			}
			else if (returned == 1 && mode == 0) {
				return 1;
			}
		}
	}
	return 0;
}

int run_prompt() {
	int done = 0;
	const char* exit_str = "exit";
	while (!done) {
		printf("user> ");
		char* str = read_line();
		char** args = parse_line(str, stderr);

		// If user enters the exit command with no arguments, exit
		if (!strcmp(args[0], exit_str) && !args[1]) {
			done = 1;
		}

		if (!done) {
			execute_command(args, stderr, 0);
		}

		int i = 0;
		while (args[i]) {
			free(args[i++]);
		}
		free(args);
		free(str);
		printf("\n");
	}
	printf("Shell exiting...\n");

	return 0;
}

int run_from_file(const char* fName) {
	FILE *fp = fopen(fName, "r");
	if (!fp) {
		fprintf(stderr, "shell: can't open file\n");
		exit(EXIT_FAILURE);
	}
	// Read commands from file
	size_t size = 20;
	size_t length = 0;
	int ch;
	char *str;
	str = malloc(sizeof *str * size);
	if (!str) {
		fprintf(stderr, "shell: memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	// Process the file
	while ((ch = fgetc(fp)) != EOF) {
		if (ch == '\n') {
			// Parse
			str[length++] = '\0';
			char** args = parse_line(str, stderr);

			// Execute
			execute_command(args, stderr, 1);

			////////////////////////
			//int i = 0;
			//while (args[i]) {
			//	free(args[i]);
			//}
			//free(args);
			////////////////////////

			free(str);
            		str = malloc(sizeof *str * size);
			if (!str) {
				fprintf(stderr, "shell: memory allocation failed\n");
				exit(EXIT_FAILURE);
			}
			length = 0;
		}
        	else {
			str[length++] = ch;
			if (length == size) {
				str = realloc(str, sizeof *str * (size*=2));
				if (!str) {
					fprintf(stderr, "shell: memory allocation failed\n");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	// Cleanup
	free(str);
	fclose(fp);
	return 0; // Success
}


int main(int argc, char **argv) {
	if (argc == 1) {
		run_prompt();
	}
	else if (argc == 2) {
		run_from_file(argv[1]);
	}
	else {
		fprintf(stderr, "shell: must be run with zero or one arguments\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
