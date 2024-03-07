#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// void shell();
// function parent_main()
//     register_child_signal(on_child_exit())
//     setup_environment()
//     shell()
char* parse_input();
void shell();
int evaluate_command(char* command);
void execute_shell_builtin(char* command);
void execute_command(char* command);
char* read_input(void);
char command[100];
int main() {
    printf("Welcome to the terminal\n");
    printf("Type 'exit' to exit the terminal\n");
    printf(">> ");
    shell();
    return 0;
}

char* read_input(void){
    char* input = (char*)malloc(100);
    scanf("%s", input);
    return input;
}

// parse_input function
char* parse_input(char *input){
    //parsing input through spaces
    return strtok(input, " ");
}

//function to evaluate_commands
int evaluate_command(char* command){
    if (strcmp(command,"export")) return 0; //representing a builtin command
    if (strcmp(command,"cd")) return 0;
    if (strcmp(command,"echo")) return 0;
    return 1; //representing an executable command
}

void execute_shell_builtin(char* command){
    if (strcmp(command, "export") == 0){
        //export function

    }
    if (strcmp(command, "cd") == 0){
        //cd function
    }
    if (strcmp(command, "echo") == 0){
        //echo function
    }
}

//function to execute command
void execute_command(char* command){
    //forking the process
    int pid = fork();
    if (pid == 0){
        //child process
        execvp(command, NULL);
    }
    else{
        //parent process
        wait(NULL);
    }
}
//shell function
void shell(void){
    do
    {
        char *expression = parse_input(read_input());
        switch (evaluate_command(expression))
        {
        case 0: //0 for built_in command
            execute_shell_builtin(expression);
            break;
        case 1:// 1 for executable or error
            execute_command(expression);
            break;
        }
    } while (strcmp(command, "exit")== 1);
    return;
}