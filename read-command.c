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
command_t* createSimpleCommand(char *str);
command_t* createOperatorCommand(int op_type, command_t* prevCom1, command_t* prevCom2);
int addSimpleCommand(char *str); /*implemented*/

/*command stack functions*/
command_t* commandPop();
command_t* commandPeek();
int commandPush(command_t *c);

/*operator stack functions*/
int opPop();
int opPeek();
int opPush(int op_type);

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
               addSimpleCommand(str);
               addOp(PIPE_COMMAND);
               prevChar = 0;
          }
          append(curByte, &str, &str_size); /*add the word to a temporary string (will resize if necessary)*/
          /*is there a better way to append to string??*/
      } else if (strchr("&|();", c)) { /*if the character is an operator*/
          if (c == '&') {
              if (prevChar == '&') {
                  /*its a double and*/
                  addSimpleCommand(str);
                  /*push and to operator stack*/
                  addOp(AND_COMMAND);
                  prevChar = 0;
              } else 
                  prevChar = '&';
          }  else if (c == '|') {
              if (prevChar == '|') {
                  /*its a double or*/
                  addSimpleCommand(str);
                  /*push or to operator stack*/
                  addOp(OR_COMMAND);
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
