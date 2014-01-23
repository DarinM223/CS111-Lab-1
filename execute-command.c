// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>  
#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

typedef struct _used_file_list {
        char **_files;
        int size;
        int capacity;
} used_file_list;
void initFileList(used_file_list *list);
void addFile(used_file_list *list, char *file);
int searchForFile(used_file_list *list, char *file) ;

void execute(command_t comm);
void executeAnd(command_t comm);
void executeOr(command_t comm);

/*creates another process if TIMETRAVEL == 1 only if the input/output stuff is correct*/
void executeSequence(command_t comm); 

void executePipe(command_t comm); /*use the C pipe() function*/
void executeSimple(command_t comm);
void dealWithRedirection(command_t comm); /*use the C open() function*/

int TIMETRAVEL=0; /*Time travel is a preset option before running the program so it can be global?*/

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  /*error (1, 0, "command execution not yet implemented");*/
        TIMETRAVEL=time_travel;
        execute(c);
}
void execute(command_t comm ) {
        switch (comm->type) {
                case AND_COMMAND:
                    executeAnd(comm );
                    break;  
                case OR_COMMAND:
                    executeOr(comm );
                    break;
                case SEQUENCE_COMMAND:
                    executeSequence(comm );
                    break;
                case PIPE_COMMAND:
                    executePipe(comm );
                    break;
                case SIMPLE_COMMAND:
                    executeSimple(comm );                    
                    break;
                case SUBSHELL_COMMAND:
                    execute(comm->u.subshell_command);
                    break;
        }
}

void executeAnd(command_t comm) {
        /*first execute first command*/
        execute(comm->u.command[0]);
        if (comm->u.command[0]->status == 0) { /*if first command completed successfully*/
                /*execute 2nd command and set status to 2nd command*/
                execute(comm->u.command[1]);
                comm->status = comm->u.command[1]->status;
        } else {
                /*otherwise set status to 1st command status*/
                comm->status = comm->u.command[0]->status;
        }
}
/*Executes or command (or command executes first command and the second one if the first one didn't return 0) */
void executeOr(command_t comm) {
        /*first execute first command*/
        execute(comm->u.command[0]);
        if (comm->u.command[0]->status != 0) { /*if first command didn't complete*/
                /*execute 2nd command and set status to 2nd command*/
                execute(comm->u.command[1]);
                comm->status = comm->u.command[1]->status;
        } else {
                /*otherwise set status to 1st command status*/
                comm->status = comm->u.command[0]->status;
        }

}
void executeSequence(command_t comm ) {
        if (TIMETRAVEL == 0) {
                execute(comm->u.command[0]);
                execute(comm->u.command[1]);
                comm->status = comm->u.command[1]->status;
        } else {
                /* TODO: Implement timetravel stuff */
        }
}

void executeSimple(command_t comm) {
        /*TODO: deal with IO first*/
        int stat1 = fork();
        if (stat1 < 0) return;
        else if (stat1 > 0) {
                /*its a parent*/
                /*wait for child process to finish*/
                int stat2;
                wait(&stat2);
        } else if (stat1 == 0) {
                /*its a child*/
                /*use exec to execute (terminated by NULL)*/
                /*executes command*/
                execvp(*comm->u.word, comm->u.word);
        }
}
void executePipe(command_t comm) {
        int pc[2];
        if (pipe(pc) < 0) return;
        int pid = fork();
        if (pid == 0) {
                dup2(pc[1], 1 ); /*make stdout come to write area to pipe*/
                close(pc[0]);
                executeSimple(comm->u.command[0]); /*read from left hand command*/
                close(pc[1]);
        } else {
                dup2(pc[0], 0); /*make stdin come to read area of pipe*/
                close(pc[1]);
                executeSimple(comm->u.command[1]); /*write to right hand command*/
                close(pc[0]);
                int status;
                wait(&status);
        }
}
