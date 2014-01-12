// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
struct command_stream {
        command_t* commands; /*array of commands*/
        int commandNum; /*number of commands*/
        int index; /*current index of commands*/
};


/************************************************************
 ** functions either implemented or want to be implemented **
 ************************************************************/

/*string functions*/
int append(int ch, char** str, int *str_size);

/*command functions*/
int isValidWordChar(char c); /*implemented*/
int precedence(int op_type);
int dealWithOperator(int op_type);
command_t* createSimpleCommand(char *str); /* don't forget to break words up! Also resize!!*/

typedef struct _stackCom {
    int capacity;
    int size;
    command_t** _commands;
} stackCom;
/*command stack functions*/
void freeStackCom(stackCom *s);
void initStackCom(stackCom *s);
command_t* commandPop(stackCom *s);
command_t* commandPeek(stackCom *s);
int commandPush(stackCom *s, command_t *c);

typedef struct _stackOp {
    int capacity;
    int size;
    int* _operators;
} stackOp;
/*operator stack functions*/
void freeStackOp(stackOp *s);
void initStackOp(stackOp *s);
int opPop(stackOp *s);
int opPeek(stackOp *s);
int opPush(stackOp *s, int op_type);

/*command tree functions*/
int sizeOfTree(command_t* commandTree);
command_stream_t buildStream(command_t* commandTree);


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  int curByte;
  command_stream_t stream;

   /*builds the stack*/
  char prevChar = 0;
  int str_size;
  char *str;
  robustAlloc(str, str_size);
  /* read every byte */
  for (curByte = get_next_byte(get_next_byte_argument); curByte != EOF ; ) {
      if (isValidWordChar(curByte)) { /*if the character is a word character*/
          if (prevChar == '&') { /*if the previous character was '&' */
              /*print error*/
          }
          if (prevChar == '|') { /*if the previous character was '|' */
               /*its a pipeline*/
               command_t* com = createSimpleCommand(str);
               commandPush(com);
               dealWithOperator(PIPE_COMMAND);
               prevChar = 0;
          }
          append(curByte, &str, &str_size); /*add the word to a temporary string (will resize if necessary)*/
          /*is there a better way to append to string??*/
      } else if (strchr("&|();", c)) { /*if the character is an operator*/
          if (c == '&') {
              if (prevChar == '&') {
                  /*its a double and*/
                  command_t* com = createSimpleCommand(str);
                  commandPush(com);
                  dealWithOperator(AND_COMMAND);
                  prevChar = 0;
              } else 
                  prevChar = '&';
          }  else if (c == '|') {
              if (prevChar == '|') {
                  /*its a double or*/
                  command_t* com = createSimpleCommand(str);
                  commandPush(com);
                  dealWithOperator(OR_COMMAND);
                  prevChar = 0;
              } else
                  prevChar = '|';
          } else if (prevChar == '&' || prevChar == '|')  { /* if the character isn't '&' or '|' but the previous character was*/
              /*print error*/
          } else if (c == ';') { /* if the character is the ';' character */
                  addSimpleCommand(str);
                  addOp(SEQUENCE_COMMAND);
                  prevChar = 0;
          }
      } else {
          /*print error*/
      }
  }
  /*get the command tree*/
  command_t *command_tree = commandPop();
  return buildStream(command_tree);


  /*get the size of the tree to allocate*/
  commands_size = sizeOfTree(command_tree);
  /*initialize stream*/
  stream = (command_stream_t) checked_malloc(sizeof(struct command_stream)); /*allocate stream size*/
  stream->commands = (command_t*) checked_malloc(commands_size*sizeof(command_t)); /*allocate commands with arbitrary command size (should be "robust")*/
  stream->commandNum = 0;
  stream->index = 0;
  /*build the stream from the command tree*/
  return *buildStream(stream, command_tree);
}


/*WTF????? Needs changing*/
command_t
read_command_stream (command_stream_t s)
{
  command_t comm;
  /* if s is NULL or you are at end of stream return NULL */
  if ((s == NULL) || (s.index >= s.commandNum)) { return NULL; }
  /*set the command to the command at the current index of the command stream*/
  comm = s.commands[s.index];
  s.index++; /*increment current index counter*/
  return comm;
}



/****************************************
 ** Implementation of wanted functions **
 ****************************************/


int isValidWordChar(char c) { /*checks if character is valid word character*/
    return (isascii(c) && (isalum(c) || strchr("!%+,-./:@^_",c)));
}


/* returns 0 if an operator command was created, 1 if the only thing that happened was the command was pushed on the stack*/
int addSimpleCommand(char *str) {
      /*push str to command stack*/
      command_t *c = createSimpleCommand(str);
      commandPush(c);
      /*peek at operator stack*/
      if (opPeek() != -1 && commandSize() >= 2) {
            /*create command with the current operator and 2 most current commands*/
            command_t *prevCom1 = commandPop();
            command_t *prevCom2 = commandPop();
            command_t *c = createOperatorCommand(opPop(), prevCom1, prevCom2);
            /*push command onto command stack*/
            commandPush(c);
            return 0;
      }
      return 1;
}


command_stream_t* buildStream(command_t* commandTree) {
  /*get the size of the tree to allocate*/
  commands_size = sizeOfTree(command_tree);
  /*initialize stream*/
  command_stream_t *stream = (command_stream_t*) checked_malloc(sizeof(struct command_stream)); /*allocate stream size*/
  stream->commands = (command_t*) checked_malloc(commands_size*sizeof(command_t)); /*allocate commands with arbitrary command size (should be "robust")*/
  stream->commandNum = 0;
  stream->index = 0;
  
  /* do code that builds the command stream from the tree using the operators */
  return stream;
}

command_t* createSimpleCommand(char *str) { /* don't forget to break words up! Also resize!!*/
      int size = 0, capacity = 10;
      command_t *com = (command_t*) checked_malloc(sizeof(command_t)); /* create new command */ 
      com->word = (char**) checked_malloc(capacity*sizeof(char*)); /*allocate capacity amount of words*/
      char *tempStr = strtok(str, " "); /* get first token divided by " " */
      if (tempStr == NULL) { /* if no tokens */
              /*No spaces, so no new words*/
      } else {
       /*break up char into words*/
              do {
                      if (size < capacity) {
                              com->word[size] = tempStr; /*add the string to the word array*/
                              size++;
                      } else {
                              capacity *= 2; /*realloc double the capacity*/
                              char **newWord = checked_realloc(capacity*(sizeof(char*)));
                              if (newWord == NULL) { return NULL; } /*if realloc failed return NULL*/
                              else {
                                     com->word = newWord; /*otherwise set the word array to the realloced array*/
                              }

                              com->word[size] = tempStr; /*add the string to the word array*/
                              size++;
                      }
              }  while ((tempStr = strtok(NULL, " ") != NULL); /*keep getting new words*/    
              if (size < capacity) { /* try to add the NULL at the end*/
                     com->word[size] = NULL;
              } else { /*if no more space*/
                     capacity += 1; /* realloc one more space just for the NULL */
                     char **newWord = checked_realloc(capacity*(sizeof(char*)));
                     if (newWord == NULL) { return -1; }
                     else {
                             com->word = newWord;
                     }

                     com->word[size] = NULL; /*set that space to NULL*/
              } 
      }                
      return com;
}

