#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define MAX_INPUT_SIZE 256
#define MAX_CMD_SIZE 64
#define MAX_ARGS 16

void on_child_exit(int signo);
void setup_environment();
void shell();
void parse_input(char *input, char *command, char *args[], int *bg);
void evaluate_expression(char *input, char *command, char *args[], int *bg);
void execute_shell_builtin(char *command, char *args[]);
void execute_command(char *command, char *args[], int bg);

char Current_Working_Directory[256]; // Change the size accordingly
int child_pid = 0;

// Signal handler for child exit
void on_child_exit(int signo) {
    int status;
    waitpid(child_pid, &status, 0);
}

//struct to store environment variables
typedef struct {
    char *name;
    char *value;
} EnvVariable;

EnvVariable environment[MAX_ARGS]; //assume a maximum of MAX_ARGS environment variables

//function to setup environment
void setup_environment() {
    //change the Current_Working_Directory to the desired initial directory
    chdir(Current_Working_Directory);
}

int find_env_variable(char *name) {
    //find the index of the environment variable with the given name
    for (int i = 0; environment[i].name != NULL; i++) {
        if (strcmp(environment[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void parse_input(char *input, char *command, char *args[], int *bg) {
    char *token;
    int i = 0;
    for (int j = 0; j < MAX_ARGS; ++j) { //intialize args to NULL
        args[j] = NULL;
    }

    token = strtok(input, " "); //seperate input by space
    //printf("token: %s\n", token);
    while (token != NULL) {
        if (i == 0) {
            strcpy(command, token); //typically first word is command
            // args[i] = strdup(token); //store command in args array
        } else if (i == 1 && strcmp(token, "&") == 0) { //if second word is & then it is a background process
            *bg = 1; // 1 for background
        } else {
            args[i - 1] = strdup(token); //args are stored in args array 
            // printf("\nargs[%d]: %s\n", i - 1, args[i - 1]); //debug   
            }
        token = strtok(NULL, " "); //continue to next word
        i++;
    }
    args[i] = NULL; //last argument is NULL
}

void evaluate_expression(char *input, char *command, char *args[], int *bg) {
    parse_input(input, command, args, bg);

    for (int i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '$') {
            int var_index = find_env_variable(args[i] + 1);
            if (var_index == -1) {
                fprintf(stderr, "Variable not found: %s\n", args[i] + 1);
                continue;
            }
            if (environment[var_index].value != NULL) {
                free(args[i]); // Free the old argument
                char *value = strdup(environment[var_index].value);
                char *token = strtok(value, " ");
                int j = i;
                while (token != NULL) {
                    args[j++] = strdup(token);
                    token = strtok(NULL, " ");
                }
                args[j] = NULL;
                printf("Expanded args[%d-%d]: ", i, j - 1);
                for (int k = i; k < j; k++) {
                    printf("%s \n", args[k]);
                }
            }
        }
    }
}

void execute_shell_builtin(char *command, char *args[]) {
    if (strcmp(command, "cd") == 0) {
        if (args[0] != NULL) {
            if (chdir(args[0]) != 0) {
                perror("cd");
            }
        } else {
            fprintf(stderr, "cd: missing argument\n");
        }
    } else if (strcmp(command, "echo") == 0) {
        int i = 0;
        while (args[i] != NULL) {
            if (args[i][0] == '$') {
                int var_index = find_env_variable(args[i] + 1);
                if (var_index != -1 && environment[var_index].value != NULL) {
                    printf("%s ", environment[var_index].value);
                }
            } else {
                printf("%s ", args[i]);
            }
            i++;
        }
        printf("\n");
    } else if (strcmp(command, "export") == 0) {
        int i = 0;
        while (args[i] != NULL) {
            char *env_var = strtok(args[i], "=");
            char *value = strtok(NULL, "");

            if (env_var != NULL && value != NULL) {
                int var_index = find_env_variable(env_var);
                if (var_index == -1) {
                    // Variable doesn't exist, add it to the environment
                    for (var_index = 0; environment[var_index].name != NULL; var_index++);
                    environment[var_index].name = strdup(env_var);
                }

                // Concatenate the entirety of value before checking for double quotes
                while (args[i + 1] != NULL) {
                    strcat(value, " ");
                    strcat(value, args[++i]);
                    if (args[i][strlen(args[i]) - 1] == '\"') {
                        break;
                    }
                }

                // If the value starts and ends with a double quote, remove them
                if (value[0] == '\"' && value[strlen(value) - 1] == '\"') {
                    value++;
                    value[strlen(value) - 1] = '\0';
                }

                // Store the value
                environment[var_index].value = strdup(value);
            } else {
                fprintf(stderr, "export: invalid argument: %s\n", args[i]);
            }

            i++;
        }
    }
}



void execute_command(char *command, char *args[], int bg) {
    printf("command: %s\n", command);
    child_pid = fork();
    if (child_pid == 0) { // Child process
        if (strcmp(command, "mkdir") == 0) {
            // If the command is mkdir, use execlp to pass arguments separately
            execlp(command, command, args[0], (char *)NULL);
            perror("Error");
            exit(EXIT_FAILURE);
        } else {
            // For other commands, use execvp as before
            execvp(command, args);
            perror("Error");
            exit(EXIT_FAILURE);
        }
    } else if (child_pid > 0) { // Parent process
        if (!bg) {
            waitpid(child_pid, NULL, 0);
        }
        else {
            printf("Background process started with PID %d\n", child_pid);
        }
    } else { // error case
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
}


void shell() {
    char input[MAX_INPUT_SIZE];
    char command[MAX_CMD_SIZE];
    char *args[MAX_ARGS];
    int bg;

    do {
        printf(">> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Remove trailing newline

        if (strcmp(input, "exit") == 0) {
            break;
        }
        else if (strcmp(input, "") == 0) {
            continue;
        }else{
        evaluate_expression(input, command, args, &bg);

        if (command[0] != '\0') {
            if (strcmp(command, "cd") == 0 || strcmp(command, "echo") == 0 || strcmp(command, "export") == 0) {
                execute_shell_builtin(command, args);
            } else {
                execute_command(command, args, bg);
                }
            }
        }
    } while (1);
}

int main() {

    signal(SIGCHLD, on_child_exit);
    setup_environment();
    shell();
    return 0;
}
