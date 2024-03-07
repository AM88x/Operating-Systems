#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define MAX_INPUT_SIZE 256
#define MAX_CMD_SIZE 64
#define MAX_ARGS 16

// Function prototypes
void on_child_exit(int signo);
void setup_environment();
void shell();
void parse_input(char *input, char **command, char ***args, int *bg);
void evaluate_expression(char *input, char **command, char ***args, int *bg);
void execute_shell_builtin(char *command, char **args);
void execute_command(char *command, char **args, int bg);

// Global variables
char Current_Working_Directory[256]; // Change the size accordingly
int child_pid = 0;

// Signal handler for child exit
void on_child_exit(int signo) {
    int status;
    waitpid(child_pid, &status, 0);
    write_to_log_file("Child terminated");
}

// Function to setup environment
void setup_environment() {
    chdir(Current_Working_Directory);
}

// Function to write to log file
void write_to_log_file(const char *message) {
    // Implementation of writing to log file
    // Replace this with your own implementation
}

// Function to parse input
void parse_input(char *input, char **command, char ***args, int *bg) {
    // Implementation of parsing input
    // You need to tokenize input and fill command, args, and bg accordingly
}

// Function to evaluate expression
void evaluate_expression(char *input, char **command, char ***args, int *bg) {
    parse_input(input, command, args, bg);
    // Additional evaluation logic if needed
}

// Function to execute shell builtin commands
void execute_shell_builtin(char *command, char **args) {
    // Implementation of executing shell builtin commands
    
}

// Function to execute external commands
void execute_command(char *command, char **args, int bg) {
    child_pid = fork();

    if (child_pid == 0) { // Child process
        execvp(command, args);
        perror("Error");
        exit(EXIT_FAILURE);
    } else if (child_pid > 0) { // Parent process
        if (!bg) {
            waitpid(child_pid, NULL, 0);
        }
    } else { // Fork error
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
}

// Main shell function
void shell() {
    char input[MAX_INPUT_SIZE];
    char *command;
    char **args;
    int bg;

    do {
        printf("$ ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // Remove trailing newline

        if (strcmp(input, "exit") == 0) {
            break;
        }

        evaluate_expression(input, &command, &args, &bg);

        if (command != NULL) {
            if (strcmp(command, "cd") == 0) {
                chdir(args[1]);
            } else if (strcmp(command, "echo") == 0 || strcmp(command, "export") == 0) {
                execute_shell_builtin(command, args);
            } else {
                execute_command(command, args, bg);
            }
        }

    } while (1);
}

int main() {
    // Register signal handler for child exit
    signal(SIGCHLD, on_child_exit);

    // Set up environment
    setup_environment();

    // Start the shell
    shell();

    return 0;
}
