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

#define commands_size 10
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /*initialize stream*/
  command_stream_t stream = (command_stream_t) checked_malloc(sizeof(struct command_stream)); /*allocate stream size*/
  stream->commands = (command_t*) checked_malloc(commands_size*sizeof(command_t)); /*allocate commands with arbitrary command size (should be "robust")*/
  stream->commandNum = 0;
  stream->index = 0;
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
