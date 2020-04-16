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
    int status;
    int completed_pid = wait(&status);
    //wait(NULL);
    //printf("Completed %d with return value %d\n", completed_pid, status);
  }
}
void executeSubCmd(char *groupcmd) {
  // separate commands by &
  // e.g ls -all & ps becomes [0]: ls -all, [1]: ps
  char *args[MAX_LINE/2 + 1];
  for (int i = 0; i < MAX_LINE/2 + 1; ++i)
    args[i] = NULL;
  int nums_of_tokens = tokenize(groupcmd, args, "&");
  int pids[nums_of_tokens];
  for (int i = 0; i < nums_of_tokens; i++) {
    // process each single command in parallel
    char *singlecmd[MAX_LINE/2 + 1];
    for (int j = 0; j < MAX_LINE/2 + 1; ++j)
      singlecmd[j] = NULL;
    // ls -all become [0]: ls, [1]: -all
    tokenize(args[i], singlecmd, " ");
    
    pids[i] = fork();
    if (pids[i] < 0) {
      perror("fork() failed\n");
    } else if (pids[i] == 0) {
      execvp(singlecmd[0], singlecmd);
      exit(EXIT_SUCCESS);
    } else {
    }
    // execute cmd
    //runCommand(singlecmd[i], false);
    
  }
}
int main(void) {
  char *args[MAX_LINE/2 + 1];
  int shouldRun = 1;
 
  while (shouldRun) {
    
    printf("osh> ");
    fflush(stdout);
    
    // read cmd from console
    char *cmdline = (char *) malloc(MAX_LINE * sizeof(char));
    int len = readline(&cmdline);
    
    // validate command
    if (len <= 0)
      continue;
    if (strcmp(cmdline, "") == 0)
      continue;
    if (strcmp(cmdline, "exit") == 0) {
      printf("\n");
      break;
    }
      
  
    // separate commands by ;
    for (int i = 0; i < MAX_LINE/2 + 1; ++i)
      args[i] = NULL;
    int num_of_tokens = tokenize(cmdline, args, ";");
    
    // for testing only
    //for (int i = 0; i < num_of_tokens; ++i)
    //    printf("%d. %s\n", i, args[i]);
    for (int i = 0; i < num_of_tokens; i++) {
      int pid = fork();
      if (pid == 0) {
        executeSubCmd(args[i]);
        exit(EXIT_SUCCESS);
      } else {
        int status;
        wait(&status);
        if (status == 0) {
          printf("\ndone\n");
        }
      }
    }
    // free memory
    free(cmdline);
  }
  
  printf("Exiting shell\n");
  return 0;
}
