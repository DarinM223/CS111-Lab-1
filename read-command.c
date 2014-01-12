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
int dealWithOperator(stackOp *op_stack, stackCom *com_stack, int op_type);
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

const int END_SUBSHELL_COMMAND = SUBSHELL_COMMAND+1;
const int LEFT_REDIRECT = SUBSHELL_COMMAND+2, RIGHT_REDIRECT = SUBSHELL_COMMAND+3;

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  int curByte;
  command_stream_t stream;

   /*builds the stack*/

  stackOp *_opStack = (stackOp*) checked_malloc(sizeof(stackOp));
  stackCom *_comStack = (stackCom*) checked_malloc(sizeof(stackCom));

  /*initialize stacks*/
  initStackOp(_opStack);
  initStackCom(_comStack);

  char prevChar = 0;
  int str_size;
  char *str;
  str = (char*)checked_malloc(str_size*sizeof(char));

  /* read every byte */
  for (curByte = get_next_byte(get_next_byte_argument); curByte != EOF ; ) {
      if (isValidWordChar(curByte)) { /*if the character is a word character*/
          if (prevChar == '&') { /*if the previous character was '&' */
              /*print error*/
          }
          if (prevChar == '|') { /*if the previous character was '|' */
               /*its a pipeline*/
               command_t* com = createSimpleCommand(str);
               commandPush(_comStack, com);
               dealWithOperator(_opStack, _comStack, PIPE_COMMAND);
               prevChar = 0;
          }
          append(curByte, &str, &str_size); /*add the word to a temporary string (will resize if necessary)*/
          prevChar = 0; /*reset any previous operator chars since there is a correct word char after the operator*/
      } else if (strchr("&|();", c)) { /*if the character is an operator*/
          if (c == '&') {
              if (prevChar == '&') {
                  /*its a double and*/
                  command_t* com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, AND_COMMAND);
                  prevChar = 0;
              } else 
                  prevChar = '&';
          }  else if (c == '|') {
              if (prevChar == '|') {
                  /*its a double or*/
                  command_t* com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, OR_COMMAND);
                  prevChar = 0;
              } else
                  prevChar = '|';
          } else if (prevChar == '&' || prevChar == '|')  { /* if the character isn't '&' or '|' but the previous character was*/
              /*print error*/
          } else if (c == ';') { /* if the character is the ';' character */
                  command_t* com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, SEQUENCE_COMMAND);
                  prevChar = 0;
          } else if (c == '(') {
                  if (prevChar != 0) { /* if there is already another subshell operator or operator without a space in between */
                          /*print error*/
                  }
                  command_t* com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, SUBSHELL_COMMAND);
                  prevChar = '(';
          } else if (c == ')') {
                  if (prevChar != 0) { /*if there is already another subshell operator or operator without a space in between */
                          /*print error*/
                  }
                  command_t* com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, END_SUBSHELL_COMMAND);
                  prevChar = '(';
          } else if (c == '<') {
                  command_t* com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, LEFT_REDIRECT);
                  prevChar = 0;
          } else if (c == '>') {
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, RIGHT_REDIRECT);
                  prevChar = 0;
          }
      } else if (c == '\n') { /*if c is a newline*/
              /*add the command tree to the command stream*/
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

/*danger!!!! seg fault!!! needs debugging*/
command_t* createSimpleCommand(char *str) { /* don't forget to break words up! Also resize!!*/
      int size = 0, capacity = 10;
      command_t *com = (command_t*) checked_malloc(sizeof(command_t)); /* create new command */ 
      (*com)->u.word = (char**) checked_malloc(capacity*sizeof(char*)); /*allocate capacity amount of words*/
      char *tempStr = strtok(str, " "); /* get first token divided by " " */
      if (tempStr == NULL) { /* if no tokens */
              /*No spaces, so no new words*/
      } else {
       /*break up char into words*/
              do {
                      if (size < capacity) {
                              (*com)->u.word[size] = (char**) checked_malloc((strlen(tempStr) + 1) * sizeof(char*));
                              /*add the string to the word array*/
                              strcpy((*com)->u.word[size], tempStr); 
                              size++;
                      } else {
                              capacity *= 2; /*realloc double the capacity*/
                              char **newWord = (char**)checked_realloc((*com)->u.word, capacity*(sizeof(char*)));
                              if (newWord == NULL) { return NULL; } /*if realloc failed return NULL*/
                              else {
                                     (*com)->u.word = newWord; /*otherwise set the word array to the realloced array*/
                              }

                              (*com)->u.word[size] = (char**) checked_malloc((strlen(tempStr) + 1) * sizeof(char*)); 
                              /*add the string to the word array*/
                              strcpy((*com)->u.word[size], tempStr);
                              size++;
                      }
              }  while ((tempStr = strtok(NULL, " ") != NULL)); /*keep getting new words*/    
              if (size < capacity) { /* try to add the NULL at the end*/
                     (*com)->u.word[size] = NULL;
              } else { /*if no more space*/
                     capacity += 1; /* realloc one more space just for the NULL */
                     char **newWord = checked_realloc((*com)->u.word, capacity*(sizeof(char*)));
                     if (newWord == NULL) { return NULL; }
                     else {
                              (*com)->u.word = newWord;
                     }

                     (*com)->u.word[size] = NULL; /*set that space to NULL*/
              } 
      }                
      return com;
}

/* appends a character to the string, and resizes if necessary 
 * str_size is the capacity 
 * returns 0 if it works, -1 if it doesn't */
int append(int ch, char** str, int *str_size) { /*tested and works*/
      int size = strlen(*str); /*get the size of the string*/
      if ((size+1) < *str_size) { /*if the size (including the zero byte) is less than the capacity*/
              (*str)[size+1] = (*str)[size]; /*set the space after the previous zero byte to a zero byte*/
              (*str)[size] = ch; /*set the previous zero byte space to the character*/
      } else {
              /*resize the string*/
              *str_size *= 2; /*double the capacity*/
              char *newStr = (char*)checked_realloc(*str,*str_size * sizeof(char));
              if (newStr == NULL) {return -1;}
              else {
                      *str = newStr;
              }
              
              (*str)[size+1] = (*str)[size]; /*set the space after the previous zero byte to a zero byte*/
              (*str)[size] = ch; /*set the previous zero byte space to the character*/
      }
      return 0;
}

/* ************************* *
 * Operation stack functions *
 * ************************* */
/*op stack functions tested and worked*/
void initStackOp(stackOp *s) {
        s->capacity = 10;
        s->size = 0;
        s->_operators = (int*) checked_malloc(s->capacity*sizeof(int));
}
void freeStackOp(stackOp *s) {
        int i;
        free(s->_operators);
        free(s);
}
int opPop(stackOp *s) {
        if (s->size == 0) return -1;
        int op = s->_operators[s->size - 1];
        s->_operators[s->size - 1] = 0;
        s->size--;
        return op;
}
int opPeek(stackOp *s) {
        if (s->size == 0) return -1;
        return s->_operators[s->size - 1];
}        
int opPush(stackOp *s, int op_type) {
        if (s->size < s->capacity) {
                s->_operators[s->size] = op_type;
                s->size++;
        } else {
                s->capacity *= 2;
                int *tempOp = (int*) checked_realloc(s->_operators, s->capacity*sizeof(int));
                if (tempOp == NULL) {return -1;}
                else {
                        s->_operators = tempOp;
                }
                s->_operators[s->size] = op_type;
                s->size++;
        }
        return 0;
}


/* *********************** *
 * Command stack functions * 
 * *********************** */

/*command stack function still untested (difficult to test command_t pointers)*/

/* initializes command stack */
void initStackCom(stackCom *s) {
        s->capacity = 10;
        s->size = 0;
        s->_commands = (command_t**) checked_malloc(s->capacity*sizeof(command_t*));
}

void freeStackCom(stackCom *s) {
        int i;
        for (i = 0; i < s->size; i++) {
                free(s->_commands[i]);
        }
        free(s->_commands);
        free(s);
}

command_t* commandPop(stackCom *s) {
        if (s->size == 0) return NULL;
        command_t *comm = s->_commands[s->size - 1];
        s->_commands[s->size - 1] = NULL;
        s->size--;
        return comm;
}
command_t* commandPeek(stackCom *s) {
        if (s->size == 0) return NULL;
        return s->_commands[s->size - 1];
}
int commandPush(stackCom *s, command_t* c) {
        if (s->size < s->capacity) {
                s->_commands[s->size] = c;
                s->size++;
        } else {
                s->capacity *= 2;
                command_t **tempComm = (command_t**) checked_realloc(s->_commands, s->capacity*sizeof(command_t*));
                if (tempComm == NULL) { return -1; }
                else {
                        s->_commands = tempComm;
                }

                s->_commands[s->size] = c;
                s->size++;
        }
        return 0;
}

