/* Author: Blake Rude
 *
 * A shell I wrote in 2019
 * which has similar command structure
 * as BASH
 * read the readme to operate.
 * 
 * V 1.0 - 29 March 2019
 */

/*
 * ; for multiple commands: working               e.g.    ls ; ps
 * mshrc for file input:  working                 e.g.    mshrc
 * aliasing: currently only saves aliases to file e.g.    alias ll="ls -l"
 * PATH expand: I do not know how to do this
 * history: working                               e.g.    history
 * piping: working                                e.g.    ls -al | more
 * color: working                                 e.g.    color -> r/g/b
 */

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#define S_SIZE 512 // String malloc size
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"

int multiflag = 0, pipeflag = 0;

/* Function Prototypes  */
char* userip();
int makearg(char* s, char*** args, char*** tempargs);
int run_process(char** args);
void userui();
int mem(char*** args);
void printargs(char** args, int argc);
void multiples(int n, char*** args, char*** tempargs);
int PipeCommand(char** pre, char** post);

typedef struct node {
  int number;
  char* s;
  struct node* next;
}hist_node;
void hnodeadd(int number, char* str, hist_node* now);
void print_node(hist_node* now);

void main(int nargs, char* argv[]){
  /* Variable declarations  */
  int i = 0, j = 0, k = 0, shell = 1, number = 1;
  char c;
  char **args, **tempargs;
  char *str;
  int argc;
  FILE* aliasfile;
  FILE* readfile;
  str = (char*)malloc(S_SIZE*sizeof(char*));
  
  hist_node* head = NULL;
  head = malloc(sizeof(hist_node));
  head->number = 1;
  head->s = "msh";
  head->next = NULL;
  
  /* While loop so shell continues to run after user input  */
  while( shell ){
    userui();
    str = userip();
    number++;
    hnodeadd(number, str, head);
    
    /* makearg() function call */
    argc = makearg(str, &args, &tempargs);
    if( argc == -1){
      printf("fatal error");
      return;
    }
    /* Shell stops running when user inputs "exit"  */
    if( args[0][0] == 'e' &&
	args[0][1] == 'x' &&

	args[0][2] == 'i' &&
	args[0][3] == 't'   )
      {
	printf("Exiting.\n");
	//shell = 0;
	return;
      }
    else if( args[0][0] == 'm' &&
	     args[0][1] == 's' &&
	     args[0][2] == 'h' &&
	     args[0][3] == 'r' &&
	     args[0][4] == 'c'   ){
      printf("Executing commands in mshrc.\n");
      mem(&args);
      mem(&tempargs);
      readfile = fopen("mshrc", "r");
      if( readfile == NULL ){
	perror("mshrc not found\n");	
      }
      else{
	argc = 0;
	while((str[i] = fgetc(readfile)) != EOF)
	  i++;
	i = 0;
        while(str[i] != '\0'){
	  if(str[i] != '\n'){
	    args[j][k] = str[i];
	    i++;
	    k++;
	  }
	  else{
	    argc++;
	    args[j][k] = '\0';
	    i++;
	    j++;
	    k = 0;
	  }
	}
	int n = 0;
	args[j][k-1] = '\0';
	argc++;
	printf("%i\n", argc);
	printargs(args, argc);
	for(; n < argc; n++){
	  multiples(n, &args, &tempargs);
	}
	//run_process(args);
      }
    }
    else if(args[0][0] == 'c' &&
	    args[0][1] == 'o' &&
	    args[0][2] == 'l' &&
	    args[0][3] == 'o' &&
	    args[0][4] == 'r' 
	    ){
      char pick[1];
      printf("Red (r), Green (g), or Blue (b)\n");
      pick[0] = getchar();
      if (pick[0] == 'r'){
	//printf("red chosen");
	printf("%sred\n", RED);
      }
      if (pick[0] == 'g'){
	//printf("green chosen");
	printf("%sgreen\n", GREEN);
      }
      if (pick[0] == 'b'){
	//printf("blue chosen");
	printf("%sblue\n", BLUE);
      }
    }
    else if(args[0][0] == 'h' &&
	    args[0][1] == 'i' &&
	    args[0][2] == 's' &&
	    args[0][3] == 't' &&
	    args[0][4] == 'o' &&
	    args[0][5] == 'r' &&
	    args[0][6] == 'y'){
      printf("History lesson\n");
      print_node(head);
    }
    else if(args[0][0] == 'a' &&
	    args[0][1] == 'l' &&
	    args[0][2] == 'i' &&
	    args[0][3] == 'a' &&
	    args[0][4] == 's'){
      char* a_name, * s;
      a_name = (char*)malloc(sizeof(char)*32);
      aliasfile = fopen("aliasrc","a+");
      if(aliasfile == NULL){
	perror(".aliasrc didn't open");
	break;
      }
      fprintf(aliasfile, "%s\n", str);
    }
    else if(pipeflag != 0){
      PipeCommand(args, tempargs);
    }
    else if(multiflag != 0){
      run_process(args);
      run_process(tempargs);
    }
    else /* or, it runs their command */{
      printargs(args, argc);
      run_process(args);
    }
  }
}
/* End of main()  */

int makearg(char* s, char*** args, char*** tempargs){
  /* Variable delcarations for makearg()  */
  int i = 0, j = 0, k = 0, temp = 0, length = 1, nWords = 1;
  multiflag = 0;
  /* While loop finds length of s[], for loop finds how many words in s[]  */
  while (s[i] != '\0'){
    i++;
    length++;
  }
  for( i = 0; i < length; i++){
    if(s[i] == ' ') nWords++;
    if(s[i] == ';') nWords++;
  }
  
  /* Allocate memory for both dimensions of args, return -1 if a row of args is NULL  */
  (*args) = (char**)malloc(sizeof(char*)*S_SIZE); //nwords
  for( i = 0; i < nWords; i++){
    (*args)[i] = (char*)malloc(sizeof(char)*S_SIZE);
  }
  
  //  for(i = 0; i < length; i++){
  //     printf("%c", s[i]);
  //  }
  i = 0;
  /* If s[i] isn't NULL Character, or a space, load into args, if it is a space, loads in NULL Character  */
  while(s[i] != '\0'){
    if(s[i] != ' ' && s[i] != ';' && s[i] != '|'){
      (*args)[j][k] = s[i];
      i++;
      k++;
    }
    else if(s[i] == ';'){
      multiflag++;
      for(temp = i; temp < length; temp++){
	if(s[temp] != ' ' &&  s[temp] != ';'){
	  (*args)[j][k] = s[temp];
	  temp;
	  i = temp;
	  break;
	}
      }
    }
    else if(s[i] == '|'){
      pipeflag++;
      for(temp = i; temp < length; temp++){
        if(s[temp] != ' ' &&  s[temp] != '|'){
          (*args)[j][k] = s[temp];
          temp;
          i = temp;
          break;
        }
      }
    }
    else {
      (*args)[j][k] = '\0';
      i++;
      j++;
      k = 0;
    }
  }
  if(multiflag > 0){
    //printf("multiflag\n");
    //fflush(stdout);
    (*tempargs) = (char**)malloc(sizeof(char*)*S_SIZE);
    for( i = 0; i < nWords; i++){
      (*tempargs)[i] = (char*)malloc(sizeof(char)*S_SIZE);
    }

    j = 1;
    k = 0;
    for(; k < 4; k++){
      if((*args)[j][k] != '\0'){
	(*tempargs)[j-1][k] = (*args)[j][k];
	(*args)[j][k] = '\0';
      }
      else
	break;
    } 
    (*args)[1] = NULL;
    (*tempargs)[1] = NULL;
  }
  if(pipeflag > 0){
    printf("pipe was flagged\n");
    (*tempargs) = (char**)malloc(sizeof(char*)*S_SIZE);
    for( i = 0; i < nWords; i++){
      (*tempargs)[i] = (char*)malloc(sizeof(char)*S_SIZE);
    }
    (*tempargs)[0] = "more";
    (*args)[2] = NULL;
  }
  return nWords;
}
/* End of makearg()  */

int run_process(char** args){
  /* Variable declarations for run_process()  */
  pid_t pid, wt;
  int child;
  
  /* Call fork(), if successful call execvp() with args[]  */
  pid = fork();
  if( pid == 0 ){
    if( execvp(args[0], args) == -1){
      perror("execvp failure.");
      return -1;
    }
  }
  /* If fork() returns -1, it failed.  */
  else if( pid == -1 ){
    perror("Forking Error.");
  }
  else{
    /* Parent needs to wait for the child, so we don't create a zombie  */
    do{
      wt = waitpid(pid, &child, WUNTRACED);
    } while (!WIFEXITED(child) && !WIFSIGNALED(child));
  }
  return 1;
} 
/* End of run_process()  */

char* userip(){
  /* Variable declarations for userip()  */
  int i = 0, c;
  char* s = malloc(S_SIZE*sizeof(char));

  /* Get user input  */
  while(1){
    c = getchar();
    /* If end of a string or newline, set null bit  */
    if( c == EOF || c == '\n' ){
      s[i] = '\0';
      return s;
    }
    /* if not, set equal  */
    else{
      s[i] = c;
    }
    i++;
  }
}
/* End of userip()  */

void userui(){
  printf(">: ");
  return;
}
/* End of userui()  */

int mem(char*** args){
  /* Variable delcarations for mem)  */
  int i = 0, nWords = 10;
  
  /* Allocate memory for both dimensions of args, return -1 if a row of args is NULL  */
  (*args) = (char**)malloc(sizeof(char*)*S_SIZE);
  for( i = 0; i < nWords; i++){
    (*args)[i] = (char*)malloc(sizeof(char)*S_SIZE);
    if(args[i] == NULL){
      perror("");
      return -1;
    }
  }
}
/* end of mem() */

void printargs(char** args, int argc){
  int i;
  printf("args:\n");
  for( i = 0; i < argc; i++){
    printf("%s\n", args[i]);
  }
}
  /* end of printargs() */

void multiples(int n, char*** args, char*** tempargs){
  int i;
  for(i = 0; (*args)[n][i] != '\0'; i++){
    (*tempargs)[0][i] = (*args)[n][i];
  }
  if( (*tempargs)[0][i] != '\0'){
    //printf("test");
    int j = i, go = 1;
    while(go){
      if( (*tempargs)[0][j] != '\0'){
	(*tempargs)[0][j] = '\0';
	j++;
      }
      else
	go = 0;
    }
  }
  (*tempargs)[1] = NULL;
  printargs((*tempargs), 1);
  run_process((*tempargs));
}
/* end of multiples */

void hnodeadd(int number, char* str, hist_node* now){
  hist_node* new = now;
  while(new->next != NULL){
    new = new->next;
  }
  new->next = malloc(sizeof(hist_node*));
  new->next->number = number;
  new->next->s = str;
  new->next->next = NULL;
}
/* end of hnodeadd() */

void print_node(hist_node* now){
  hist_node* current = now;

  while (current != NULL){
    printf("%d   ", now->number);
    printf("%s\n", now->s);
    now = now->next;
  }
}
/* end of print_node() */

int PipeCommand(char** pre, char ** post){
  int status;
  int fd[2];
  int result;
  pid_t child1, child2;
  
  child1 = fork();

  /* parent */
  if(child1 != 0){
    waitpid(child1, &status, 0);
  }
  /* child */
  else{
    if((pipe(fd)) < 0){
      perror("error with fd\n");
    }
    child2 = fork();

    /* parent */
    if(child2 != 0){
      close(fd[1]);
      close(0);
      dup(fd[0]);
      close(fd[0]);

      if(execvp(post[0], post) < 0){
	perror("execvp\n");
      }
    }
    /* child */
    else{
      close(fd[0]);
      close(1);
      dup(fd[1]);
      close(fd[1]);

      if(execvp(pre[0], pre) < 0){
	perror("execvp\n");
      }
    }
  }
}
/* end of PipeCommand() */
