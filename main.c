#include <assert.h>     // assert
#include <fcntl.h>      // O_RDWR, O_CREAT
#include <stdbool.h>    // bool
#include <stdio.h>      // printf, getline
#include <stdlib.h>     // calloc
#include <string.h>     // strcmp
#include <unistd.h>     // execvp
#include <sys/wait.h>       // wait

#define MAX_LINE 80 /* The maximum length command */
int readline(char** buffer) {
  size_t len;
  int number_of_chars = getline(buffer, &len, stdin);
  if (number_of_chars > 0) {
     // get rid of \n
     (*buffer)[number_of_chars - 1] = '\0';
  }
  return number_of_chars;
}
int tokenize(char* line, char** tokens, char *criteria) {
  // http://www.cplusplus.com/reference/cstring/strtok/
  char* pch;
  pch =  strtok(line, criteria);
  int num = 0;
  while (pch != NULL) {
     //printf ("Token: %s\n",pch);
     tokens[num] = pch;
     ++num;
     pch =  strtok(NULL, criteria);
  }
  return num;
}
void runCommand(char *args[], bool waitForIt) {
  int child_pid = fork();
  if (child_pid == 0) {
    //sleep(2);
    int success = execvp(args[0], args);
    // only gets here if execvp failed
    printf("execvp for %s wailed with %d\n", *args, success);
    exit(EXIT_FAILURE);
  }
  if (waitForIt) {
    printf("Parent waiting on %d\n", child_pid);
    //int status;
    //int completed_pid = wait(&status);
    ///wait(NULL);
    //printf("Completed %d with return value %d\n", completed_pid, status);
  }
}
int main(void) {
  char *args[MAX_LINE/2 + 1];
  int shouldRun = 1;
  
  while (shouldRun) {
   
    printf("osh> ");
    fflush(stdout);
    
    char *cmdline = (char *) malloc(MAX_LINE * sizeof(char));
    
    int len = readline(&cmdline);
    // validate command
    if (len <= 0)
      break;
    if (strcmp(cmdline, "") == 0)
      continue;
    if (strcmp(cmdline, "exit") == 0)
      break;
  
    // clear out args
    for (int i = 0; i < MAX_LINE/2 + 1; ++i)
      args[i] = NULL;
    int num_of_tokens = tokenize(cmdline, args, ";");
    // for testing only
    //for (int i = 0; i < num_of_tokens; ++i)
    //    printf("%d. %s\n", i, args[i]);
    
    
    // args[i] is a command separed by;
    //pid_t pid[num_of_tokens];
    for (int i = 0; i < num_of_tokens; i++) {
      pid_t pid = fork();
      //pid[i] = fork();
      if (pid < 0) {
        perror("fork() failed\n");
      } else if (pid == 0) {
        // process each command args[i]
        //printf("executing: %s ", args[i]);
        char *sub_args[MAX_LINE/2 + 1];
        for (int j = 0; j < MAX_LINE/2 + 1; j++)
          sub_args[j] = NULL;
        int sub_num_of_tokens = tokenize(args[i], sub_args, " &");
        pid_t sub_pid[sub_num_of_tokens];
        for (int k = 0; k < sub_num_of_tokens; k++) {
          //runCommand(sub_args, false);
          runCommand(sub_args, false);
        }
        
      } else {
        // parent wait until child finish
        //runCommand(args[i], true);
        //int status;
        //wait(&status);
      }
    }
    
    
    
    free(cmdline);
  }
  
  printf("Exiting shell\n");
  return 0;
}
