#include <assert.h>     // assert
#include <fcntl.h>      // O_RDWR, O_CREAT
#include <stdbool.h>    // bool
#include <stdio.h>      // printf, getline
#include <stdlib.h>     // calloc
#include <string.h>     // strcmp
#include <unistd.h>     // execvp
#include <sys/wait.h>       // wait

#define MAX_LINE 80 /* The maximum length command */
const int BUF_SIZE = 4096;
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
int PipeOrRedirection(char *str) {
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == '>' || str[i] == '<' || str[i] == '|')
      return i;
  }
  return -1;
}

void executeSubCmd(char *groupcmd) {
  // separate commands by &
  // e.g ls -all & ps becomes [0]: ls -all, [1]: ps
  int pid = fork();
  int parentstatus;
  if (pid == 0) {
    int status;
    char *args[MAX_LINE/2 + 1];
    for (int i = 0; i < MAX_LINE/2 + 1; ++i)
      args[i] = NULL;
    int nums_of_tokens = tokenize(groupcmd, args, "&");
    int pids[nums_of_tokens];
    for (int i = 0; i < nums_of_tokens; i++) {
      int pos = PipeOrRedirection(args[i]);
      if (pos != -1) {
        char *temp[MAX_LINE/2 + 1];
        for (int j = 0; j < MAX_LINE/2 + 1; ++j)
          temp[j] = NULL;
        // contains | <  > in command
        char ch = args[i][pos];
        char *criteria;
        if (ch == '>')
          criteria = ">";
        else if (ch == '<')
          criteria = "<";
        else if (ch == '|')
          criteria = "|";
        //printf("%c\n", ch);
        int l = tokenize(args[i], temp, criteria);
//        for (int q=0;q < l; q++)
//          printf("%s\n", temp[q]);
//        printf("%d", l);
//        printf("%c", ch);
        if (strcmp(criteria, ">") == 0) {
          
          char *singlecmd[MAX_LINE/2 + 1];
          for (int j = 0; j < MAX_LINE/2 + 1; ++j)
            singlecmd[j] = NULL;
          // ls -all become [0]: ls, [1]: -all
          //printf("%s\n",temp[0]);
          int lll= tokenize(temp[0], singlecmd, " ");
          //printf("%s\n", temp[1]);
          //printf("%d\n", lll);
//          printf("singlecmd: ");
//          for (int k=0;k<lll;k++)
//            printf("%s\n", singlecmd[k]);
          int pipefd[2];
          pipe(pipefd);
          pids[i] = fork();
          if (pids[i] < 0) {
            perror("fork() failed\n");
            //exit(EXIT_FAILURE);
          } else if (pids[i] == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], 1);
            close(pipefd[1]);
            execvp(singlecmd[0], singlecmd);
            exit(EXIT_SUCCESS);
          } else {
            char buf[BUF_SIZE];
            close(pipefd[1]);
            int y=read(pipefd[0], buf, BUF_SIZE);
            // remove extra space in filename
            char *filename[2];
            tokenize(temp[1], filename, " ");
            printf("%s\n", filename[0]);
            FILE *file;
            file = fopen(filename[0], "w");
            
            
           
            // this is for testing
            for (int q=0; q < y;q++) {
              //printf("%c", buf[q]);
              fputc(buf[q], file);
            }
            fclose(file);
            wait(&status);
          }
          
        } else if (strcmp(criteria, "<") == 0) {
          
        } else if (ch == "|") {
          
        }
//        pids[i] = fork();
//        if (pids[i] < 0) {
//          perror("fork() failed\n");
//        } else if (pids[i] == 0) {
//          execvp(singlecmd[0], singlecmd);
//          exit(EXIT_SUCCESS);
//        } else {
//          wait(&status);
//        }
//        //continue;
      } else {
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
          wait(&status);
        }
        // execute cmd
        //runCommand(singlecmd[i], false);
      
      }
    }
  } else {
    wait(&parentstatus);
    exit(EXIT_SUCCESS);
  }
}
int main(void) {
  char *args[MAX_LINE/2 + 1];
  int shouldRun = 1;
 
  while (shouldRun) {
    
    printf("osh>");
    fflush(stdout);
    
    // read cmd from console
    char *cmdline = (char *) malloc(MAX_LINE * sizeof(char));
    int len = readline(&cmdline);
    
    // validate command
    if (len <= 0)
      break;
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
    for (int i = 0; i < num_of_tokens; i++)
      executeSubCmd(args[i]);
    // free memory
    free(cmdline);
  }
  
  printf("Exiting shell\n");
  return 0;
}
