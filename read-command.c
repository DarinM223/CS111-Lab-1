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

int isValidWordChar(char c) { /*checks if character is valid word character*/
    return (isascii(c) && (isalum(c) || strchr("!%+,-./:@^_",c)));
}


#define commands_size 10
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  int curByte;
  command_stream_t stream;
  command_t currCommand;

  /*initialize stream*/
  stream = (command_stream_t) checked_malloc(sizeof(struct command_stream)); /*allocate stream size*/
  stream->commands = (command_t*) checked_malloc(commands_size*sizeof(command_t)); /*allocate commands with arbitrary command size (should be "robust")*/
  stream->commandNum = 0;
  stream->index = 0;
  /*builds the stack*/
  char prevChar = 0;
  int str_size;
  char *str;
  robustAlloc(str, str_size);
  /* read every byte */
  for (curByte = get_next_byte(get_next_byte_argument); curByte != EOF ; ) {
      if (isValidWordChar(curByte)) { /*if the character is a word character*/
          if (prevChar == '&' || prevChar == '|') { /*if the previous character was '&' or '|'*/
              /*print error*/
          }
          append(curByte, &str, &str_size); /*add the word to a temporary string (will resize if necessary)*/
          /*is there a better way to append to string??*/
      } else if (strchr("&|();", c)) { /*if the character is an operator*/
          if (c == '&') {
              if (prevChar == '&') {
                  /*its a double and*/
                  /*push str to command stack*/
                  command_t c = createSimpleCommand(str);
                  addCommand(c);
                  /*peek at operator stack*/
                  if (opPeek() != -1) {
                       /*create command with the current operator and 2 most current commands*/
                       command_t prevCom1 = commandPop();
                       command_t prevCom2 = commandPop();
                       command_t c = createOperatorCommand(opPop(), prevCom1, prevCom2);
                       /*push command onto command stack*/
                       addCommand(c);
                  }
                  /*push and to operator stack*/
                  addOp(AND_COMMAND);
                  prevChar = 0;
              } else 
                  prevChar = '&';
          }  else if (c == '|') {
              if (prevChar == '|') {
                  /*its a double or*/
                  /*push str to command stack*/
                  command_t c = createSimpleCommand(str);
                  addCommand(c);
                  /*peek at operator stack*/
                  if (opPeek() != -1) {
                       /*create command with the current operator and 2 most current commands*/
                       command_t prevCom1 = commandPop();
                       command_t prevCom2 = commandPop();
                       command_t c = createOperatorCommand(opPop(), prevCom1, prevCom2);
                       /*push command onto command stack*/
                       addCommand(c);
                  }

                  /*push or to operator stack*/
                  addOp(OR_COMMAND);
                  prevChar = 0;
              } else
                  prevChar = '|';
          } else if (prevChar == '&' || prevChar == '|') { /* if the character isn't '&' or '|' but the previous character was*/
              /*print error*/
          }
      } else {
          /*print error*/
      }
  }
 
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  command_t comm;
  /* if s is NULL or you are at end of stream return NULL */
  if ((s == NULL) || (s->index >= s->commandNum)) { return NULL; }
  /*set the command to the command at the current index of the command stream*/
  comm = s->commands[s->index];
  s->index++; /*increment current index counter*/
  return comm;
}
