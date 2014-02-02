// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>  
#include <error.h>
#include <fcntl.h>
#include <string.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

/* ******************** *
 * Timetravel functions *
 * ******************** */
fileNode_t initFileList(char *word);
void addFileToList(fileNode_t list, char *word);
void addCommTreeDependencies(commandTreeNode_t tree, command_t comm);
void createDependency(commandTreeNode_t source, commandTreeNode_t dependency);
void findDependencies(commandTreeNode_t treeOne, commandTreeNode_t treeTwo);
command_t execute_time_travel(command_stream_t s);

void execute(command_t comm);
void executeAnd(command_t comm);
void executeOr(command_t comm);

/*creates another process if TIMETRAVEL == 1 only if the input/output stuff is correct*/
void executeSequence(command_t comm); 

void executePipe(command_t comm); /*use the C pipe() function*/
void executeSimpleOrSubshell(command_t comm);

void printCommandError(char* msg) {
      /* fprintf(stderr, ":( Your PC ran into a problem and needs to restart. Your hard drive will be wiped out shortly. If you like to know more you can search online for this error: HAL9000_IM_SORRY_DAVE \n");*/ 
      perror(msg);
      exit(1); /*comment this out to debug*/
}


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
                    executeSimpleOrSubshell(comm );                    
                    break;
                case SUBSHELL_COMMAND:
                    executeSimpleOrSubshell(comm );
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

void executeSimpleOrSubshell(command_t comm) {
	/* deal with redirection */
	int stdin_dup, stdout_dup;
	if ((stdin_dup = dup(STDIN_FILENO)) == -1) printCommandError("dup");
	if ((stdout_dup = dup(STDOUT_FILENO)) == -1) printCommandError("dup");

	int fd0, fd1;
        if (comm->input != NULL)
        {
                if ((fd0 = open(comm->input, O_RDONLY)) == -1) 
		{	
			perror(comm->input); 
			comm->status = 1;
		        close(stdin_dup);
        		close(stdout_dup);
			return;
		}
                if (dup2(fd0, STDIN_FILENO) == -1) printCommandError("dup2");
                close(fd0);
        }
        if (comm->output != NULL)
        {
                if ((fd1 = open(comm->output, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		{
			perror(comm->output);
			comm->status = 1;
		        close(stdin_dup);
        		close(stdout_dup);
			return;
		}
                if (dup2(fd1, STDOUT_FILENO) == -1) printCommandError("dup2");
                close(fd1);
        }

        int stat1;
	if ((stat1 = fork()) < 0)  printCommandError("fork");
        if (stat1 > 0) {
                /*its a parent*/
                /*wait for child process to finish*/
                int stat2;
                waitpid(stat1, &stat2, 0);
		if (WIFEXITED(stat2))
    			comm->status =  WEXITSTATUS(stat2);
        } else if (stat1 == 0) {
                /*it's a child*/
		/* simple command */
		if (comm->type == SIMPLE_COMMAND)
		{
                	/*use exec to execute (terminated by NULL)*/
                	/*executes command*/
			int status;
			if (strcmp(comm->u.word[0], "exec") == 0) /* special case exec */
			{	
				if(strcmp(comm->u.word[1], "exec") == 0) 
					fprintf(stderr,"exec: exec: command not found\n");
				else
					status = execvp(comm->u.word[1], comm->u.word+1); /* use second word as filename */
				if (status == -1)
					fprintf(stderr,"%s: command not found\n", comm->u.word[1]);
			}
			else
			{
                		status = execvp(*comm->u.word, comm->u.word);
				if (status == -1)
					fprintf(stderr,"%s: command not found\n", comm->u.word[0]);
			}
			_exit(1);
		}
		else /* subshell */
		{
			execute(comm->u.subshell_command);
                	exit(comm->u.subshell_command->status);
		}
        }

	/* restore STDIN, STDOUT */
        if (dup2(stdin_dup, 0) == -1) printCommandError("dup2");
        if (dup2(stdout_dup, 1) == -1) printCommandError("dup2");
	close(stdin_dup);
	close(stdout_dup);
}

void executePipe(command_t comm) {
	/* deal with redirection */
        int stdin_dup, stdout_dup;
        if ((stdin_dup = dup(STDIN_FILENO)) == -1) printCommandError("dup");
        if ((stdout_dup = dup(STDOUT_FILENO)) == -1) printCommandError("dup");

        int pc[2];
        if (pipe(pc) < 0) printCommandError("pipe");
        int pid = fork();
        if (pid == 0) {
                if (dup2(pc[1], 1 ) == -1) printCommandError("dup2"); /*make stdout come to write area to pipe*/
                close(pc[0]);
                execute(comm->u.command[0]); /*read from left hand command*/
                close(pc[1]);
		exit(comm->u.command[0]->status);
        } else if (pid > 0) {
		int status;
		if(wait(&status) == -1) printCommandError("wait");
                if (dup2(pc[0], 0) == -1) printCommandError("dup2"); /*make stdin come to read area of pipe*/
                close(pc[1]);
                execute(comm->u.command[1]); /*write to right hand command*/
                close(pc[0]);
                comm->status = comm->u.command[1]->status;
        } else {
                printCommandError("fork");
        }

        /* restore STDIN, STDOUT */
        if (dup2(stdin_dup, 0) == -1) printCommandError("dup2");
        if (dup2(stdout_dup, 1) == -1) printCommandError("dup2");
        close(stdin_dup);
        close(stdout_dup);

}


fileNode_t initFileList(char *file) {
        fileNode_t newFileList = (fileNode_t) checked_malloc(sizeof (fileNode_t));
        newFileList->file = file; 
        newFileList->next = NULL;
        return newFileList;
}
void addFileToList(fileNode_t list, char *file) {
        //if file is already in list, do nothing
        if (strcmp(list->file, file) == 0) {
                return;
        } else if (list->next == NULL) { //if reached the end
                list->next = (fileNode_t) checked_malloc(sizeof(fileNode_t)); //create a new file node
                list->next->file = file;
                list->next->next = NULL;
        } else { //otherwise keep going down the list
                addFileToList(list->next, file);
        }
}
void addCommTreeDependencies(commandTreeNode_t  tree, command_t comm) {
       //add the command inputs and outputs into the input/output lists
       if (comm->input != NULL) {
               if (tree->inputList == NULL) {
                       tree->inputList = initFileList(comm->input);
               } else {
                       addFileToList(tree->inputList, comm->input);
               }
       }
       if (comm->output != NULL) {
               if (tree->outputList == NULL) {
                       tree->outputList = initFileList(comm->output);
               } else {
                       addFileToList(tree->outputList, comm->output);
               }
       } 

       
       switch (comm->type) {
               //if &&, ||, ;, or | recurse and add the dependencies of the other commands
               case AND_COMMAND:
               case OR_COMMAND:
               case SEQUENCE_COMMAND:
               case PIPE_COMMAND:
                       addCommTreeDependencies(tree, comm->u.command[0]);
                       addCommTreeDependencies(tree, comm->u.command[1]);
                       break;
               //if subshell add the subshell's dependencies
               case SUBSHELL_COMMAND:
                       addCommTreeDependencies(tree, comm->u.subshell_command);
                       break;
               //add the input file arguments of the simple command
               case SIMPLE_COMMAND:
                       {
                               //doesn't deal with flags very well like sort -u something... 
                               //well better safe than sorry I guess...
                               int i;
                               for (i = 1; comm->u.word[i] != NULL; i++) {
                                       if (tree->inputList == NULL)
                                               tree->inputList = initFileList(comm->u.word[i]);
                                       else
                                               addFileToList(tree->inputList, comm->u.word[i]);
                               }

                       }
                       break;
       }
}

void createDependency(commandTreeNode_t source, commandTreeNode_t dependency) {
        dependencyNode_t dependList = source->dependencyList;
        dependencyNode_t last = dependList;
        //find last dependency
        for (;dependList != NULL;dependList = dependList->next) {
                last = dependList;
        }
        //create new node
        dependencyNode_t newNode = (dependencyNode_t)checked_malloc(sizeof(dependencyNode_t));
        newNode->dependency = dependency;
        newNode->next = NULL;
        if (last != NULL) {
                last->next = newNode;
        } else {
                source->dependencyList = newNode;
        }
}
//treeOne is the current command tree, treeTwo is the previous command tree
void findDependencies(commandTreeNode_t treeOne, commandTreeNode_t treeTwo) {
        fileNode_t currOutput = treeOne->outputList;
        fileNode_t currInput = treeTwo->inputList;

        //loop through the output list of tree one and the input list of tree two
        for (;currOutput != NULL;currOutput = currOutput->next) {
                for (;currInput != NULL;currInput = currInput->next) {
                        //if tree one outputs to a file that tree two reads from
                        if (strcmp(currInput->file, currOutput->file) == 0) {
                                //the current tree (treeOne) is a dependency of the previous tree (treeTwo)
                                treeOne->numDependencies++;
                                createDependency(treeTwo, treeOne);
                                return;
                        }
                }
        }

        currOutput = treeTwo->outputList;
        currInput = treeOne->inputList;

        //loop through the output list of tree two and the input list of tree one
        for (;currOutput != NULL;currOutput = currOutput->next) {
                for (;currInput != NULL;currInput = currInput->next) {
                        //if tree two outputs to a file that tree one reads from 
                        if (strcmp(currInput->file, currOutput->file) == 0) {
                                //the current tree (treeOne) is a dependency of the previous tree (treeTwo)
                                treeOne->numDependencies++;
                                createDependency(treeTwo, treeOne);
                                return;
                        }
                }
        }

        //tl;dr if there is ANY conflict between files of the two trees, the current tree will be dependent
        //on the previous tree to finish first
}
