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
        int size; /*current size*/
        int capacity; /*current capacity*/
        int index;
};

/************************************************************
 ** functions either implemented or want to be implemented **
 ************************************************************/

/*string functions*/
int append(int ch, char** str, int *str_size); /*works*/

/*command functions*/
int isValidWordChar(char c); /*implemented*/
int isValidWord(char *c); /*works*/
int precedence(int op_type); 
int dealWithOperator(stackOp *op_stack, stackCom *com_stack, int op_type);
int createCommandTree(stackOp* op_stack, stackCom* com_stack, int com_type);
char** breakIntoWords(char* str); /*works*/
command_t createSimpleCommand(char *str); /* don't forget to break words up! Also resize!!*/ /*works*/

typedef struct _stackCom {
    int capacity;
    int size;
    command_t* _commands;
} stackCom;
/*command stack functions*/
void freeStackCom(stackCom *s); /*works*/
void initStackCom(stackCom *s);/*works*/
command_t commandPop(stackCom *s);/*works*/
command_t commandPeek(stackCom *s); /*works*/
int commandPush(stackCom *s, command_t c);/*works*/

typedef struct _stackOp {
    int capacity;
    int size;
    int* _operators;
} stackOp;
/*operator stack functions*/
void freeStackOp(stackOp *s); /*works*/
void initStackOp(stackOp *s); /*works*/
int opPop(stackOp *s); /*works*/
int opPeek(stackOp *s); /*works*/
int opPush(stackOp *s, int op_type);/*works*/

const int END_SUBSHELL_COMMAND = SUBSHELL_COMMAND+1;
const int LEFT_REDIRECT = SUBSHELL_COMMAND+2, RIGHT_REDIRECT = SUBSHELL_COMMAND+3;

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  int curByte;
  command_stream_t stream;

  /*initialize stream*/
  stream->capacity = 10;
  stream->size = 0;
  stream->index = 0;
  stream = (command_stream_t) checked_malloc(sizeof(struct command_stream)); /*allocate stream size*/
  stream->commands = (command_t*) checked_malloc(stream->capacity*sizeof(command_t)); /*allocate commands with arbitrary command size (should be "robust")*/

   /*builds the stack*/

  stackOp *_opStack = (stackOp*) checked_malloc(sizeof(stackOp));
  stackCom *_comStack = (stackCom*) checked_malloc(sizeof(stackCom));

  /*initialize stacks*/
  initStackOp(_opStack);
  initStackCom(_comStack);

  char prevChar = 0;
  int str_size = 10;
  char *str;
  str = (char*)checked_malloc(str_size*sizeof(char));
  int subshellFlag = 0;
  int opFlag = -1;
  /*if op flag is set
   * if a word char or a beginning subshell is next readed char then reset op flag
   */
  /* read every byte */
  for (curByte = get_next_byte(get_next_byte_argument); curByte != EOF ; ) {
      if (isValidWordChar(curByte) || strchr(" \t", curByte)) { /*if the character is a word character or space*/
          if (isValidWordChar(curByte) && opFlag != -1) opFlag = -1; /*if op flag is set and the char is a word char unset the flag*/ 
          if (prevChar == '&') { /*if the previous character was '&' */
              /*print error*/
          }
          if (prevChar == '|') { /*if the previous character was '|' */
               /*its a pipeline*/
               command_t com = createSimpleCommand(str);
               commandPush(_comStack, com);
               dealWithOperator(_opStack, _comStack, PIPE_COMMAND);
               opFlag = PIPE_COMMAND;
               prevChar = 0;
          }
          append(curByte, &str, &str_size); /*add the word to a temporary string (will resize if necessary)*/
          prevChar = 0; /*reset any previous operator chars since there is a correct word char after the operator*/
      } else if (strchr("&|();<>", c)) { /*if the character is an operator*/
          if (opFlag != -1 && curByte == '(') { /*if the op flag is set and the char is '(' unset the flag*/
                  opFlag = -1;
          } else {/*if there is an operator flag and the current char is a operator that is not '('*/
                  /*print error*/
          }
          if (c == '&') {
              if (prevChar == '&') {
                  /*its a double and*/
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, AND_COMMAND);
                  opFlag = AND_COMMAND;
                  prevChar = 0;
              } else 
                  prevChar = '&';
          }  else if (c == '|') {
              if (prevChar == '|') {
                  /*its a double or*/
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, OR_COMMAND);
                  opFlag = OR_COMMAND;
                  prevChar = 0;
              } else
                  prevChar = '|';
          } else if (prevChar == '&' || prevChar == '|')  { /* if the character isn't '&' or '|' but the previous character was*/
              /*print error*/
          } else if (curByte == ';') { /* if the character is the ';' character */
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, SEQUENCE_COMMAND);
                  opFlag = SEQUENCE_COMMAND;
                  prevChar = 0;
          } else if (curByte == '(') {
                  if (opFlag == '<' || opFlag == '>') { /*if the operator flag is < or > */
                          /*print error*/
                  } else if (opFlag == -1) { /*if the operator flag hasn't been set*/
                          /*print error*/
                  }
                  /*DO NOT ADD THE LAST COMMAND*/
                  /*Only things possible are:
                   * a op (blah)
                   * (blah) */
                  /*command_t com = createSimpleCommand(str);*/
                  /*commandPush(_comStack, com);*/
                  dealWithOperator(_opStack, _comStack, SUBSHELL_COMMAND);
                  subshellFlag++; /*increase number of subshells*/
                  prevChar = 0;
          } else if (curByte == ')') {
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, END_SUBSHELL_COMMAND);
                  subshellFlag--;
                  prevChar = 0;
          } else if (curByte == '<') {
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, LEFT_REDIRECT);
                  opFlag = LEFT_REDIRECT;
                  prevChar = 0;
          } else if (curByte == '>') {
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, RIGHT_REDIRECT);
                  opFlag = RIGHT_REDIRECT;
                  prevChar = 0;
          }
      } else if (curByte == '\n' || curByte == '#') { /*if c is a newline or comment*/
              if (curByte == '#') { /*if c is a comment loop then ignore until a newline is reached*/
                     do {
                             curByte = get_next_byte(get_next_byte_argument);
                     } while (curByte != '\n' && curByte != EOF);
              }
              /*act as if the character is a newline*/
              if (opFlag == -1 && subshellFlag == 0) { /*if there is no operator flag and the number of subshells is zero*/
                  /*add the command tree to the command stream*/
                  /*add the last command*/
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  int __op;
                  while ((__op = opPop(_opStack)) != -1) { /*while there are still operators in the stack*/
                        createCommandTree(_opStack, _comStack, __op); /*create the tree from the stacks*/
                  }
                  command_t command_tree = commandPop(_comStack);
                  /*add command tree to stream*/

                  if (stream->size < stream->capacity) {
                        stream->_commands[stream->size] = command_tree;
                        stream->size++;
                  } else {
                        stream->capacity *= 2;
                        command_t* tempComStream = (command_t*) checked_realloc(stream->_commands, stream->capacity * sizeof(command_t));
                        if (tempComStream == NULL) { /*print error*/ }
                        else {
                              stream->_commands = tempComStream;
                        }

                        stream->_commands[stream->size] = command_tree;
                        stream->size++;
                  }
              } else if (subshellFlag != 0) { /*if there is at least one subshell*/
                  /* add a ';' */
                  command_t com = createSimpleCommand(str);
                  commandPush(_comStack, com);
                  dealWithOperator(_opStack, _comStack, SEQUENCE_COMMAND);
                  opFlag = SEQUENCE_COMMAND;
                  prevChar = 0;
              } else {/* if there is an operator flag*/
                      /*do nothing*/
              }
      } else {
          /*print error*/
      }
  }


}

command_t
read_command_stream (command_stream_t s)
{
  command_t comm;
  /* if s is NULL or you are at end of stream return NULL */
  if ((s == NULL) || (s->index >= s->size)) { return NULL; }
  /*set the command to the command at the current index of the command stream*/
  comm = s->commands[s->index];
  s->index++; /*increment current index counter*/
  return comm;
}

/****************************************
 ** Implementation of wanted functions **
 ****************************************/


int isValidWordChar(char c) { /*checks if character is valid word character*/
    return (isascii(c) && (isalum(c) || strchr("!%+,-./:@^_",c)));
}
/*returns 0 if not valid, 1 if valid*/
int isValidWord(char *c) {
       if (strlen(c) == 0) return 0;
       int i;
       for (i = 0; i < strlen(c); i++) {
               if (!isValidWordChar(c[i])) {
                     return 0;
               } 
       }
       return 1;
}

/*breaks a string into an array of strings ending in NULL
 *if its an empty string it returns an empty array of strings
 *if the allocation failed then it returns NULL
 * */
/*tested and works*/
char** breakIntoWords(char* str) {
        int arr_capacity = 10;
        int arr_size = 0;
        char **strArr = (char**)checked_malloc(arr_capacity*sizeof(char*));
        if (strlen(str) == 0) {
                strArr[0] = NULL;
                return strArr;
        }
        int str_size = 10;
        int modified_str_size = str_size;
        char *tempStr = (char*)checked_malloc(str_size*sizeof(char));
        tempStr[0] = '\0';
        int i = 0;
        while (1) {
                if (str[i] == ' ' || str[i] == '\0') {
                        if (isValidWord(tempStr) != 0) { /*add to strings only if it is valid*/
                                /*add tempStr to the array of c strings*/
                                if (arr_size < arr_capacity) {
                                        strArr[arr_size] = (char*)checked_malloc((strlen(tempStr)+1) * sizeof(char));
                                        strcpy(strArr[arr_size], tempStr);
                                        arr_size++;
                                } else {
                                        arr_capacity *= 2;
                                        char **tempStrArr = (char**)checked_realloc(strArr, arr_capacity*sizeof(char*));
                                        if (tempStrArr == NULL) {return NULL;}
                                        else {
                                                strArr = tempStrArr;
                                        }
                                        strArr[arr_size] = (char*) checked_malloc((strlen(tempStr)+1) * sizeof(char));
                                        strcpy(strArr[arr_size], tempStr);
                                        arr_size++;
                                }
                        }
                        /*reset tempStr*/
                        free(tempStr);
                        tempStr = NULL;
                        if (str[i] == '\0') break;
                        tempStr = (char*)checked_malloc(str_size*sizeof(char));
                        tempStr[0] = '\0';
                        modified_str_size = str_size;
                } else {
                        /*append the character to the temporary string*/
                        append(str[i], &tempStr, &modified_str_size);
                }
                i++;
        }
        free(tempStr);
        tempStr = NULL;
        /*try to add the NULL at the end*/
        if (arr_size < arr_capacity) {
                strArr[arr_size] = NULL;
        } else {
                arr_capacity += 1;
                char **tempStrArr = (char**) checked_realloc(strArr, arr_capacity*sizeof(char*));
                if (tempStrArr == NULL) {return NULL;}
                else {
                        strArr = tempStrArr;
                }
                strArr[arr_size] = NULL;
        }
        return strArr;
}
/*tested and works*/
command_t createSimpleCommand(char *str) { /* don't forget to break words up! Also resize!!*/
      command_t com = checked_malloc(sizeof(struct command));
      com->type = SIMPLE_COMMAND;
      char **a = breakIntoWords(str);
      com->u.word = a;
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
        s->_commands = (command_t*) checked_malloc(s->capacity*sizeof(command_t));
}

void freeStackCom(stackCom *s) {
        int i;
        for (i = 0; i < s->size; i++) {
                free(s->_commands[i]);
        }
        free(s->_commands);
        free(s);
}

command_t commandPop(stackCom *s) {
        if (s->size == 0) return NULL;
        command_t comm = s->_commands[s->size - 1];
        s->_commands[s->size - 1] = NULL;
        s->size--;
        return comm;
}
command_t commandPeek(stackCom *s) {
        if (s->size == 0) return NULL;
        return s->_commands[s->size - 1];
}
int commandPush(stackCom *s, command_t c) {
        if (s->size < s->capacity) {
                s->_commands[s->size] = c;
                s->size++;
        } else {
                s->capacity *= 2;
                command_t *tempComm = (command_t*) checked_realloc(s->_commands, s->capacity*sizeof(command_t));
                if (tempComm == NULL) { return -1; }
                else {
                        s->_commands = tempComm;
                }

                s->_commands[s->size] = c;
                s->size++;
        }
        return 0;
}

/* modify command tree given an operator */
/* return 1 if successful, 0 otherwise */
int createCommandTree(stackOp *op_stack, stackCom *com_stack, int com_type)
{
    int flag = 1; /* return value */
    
    switch (com_type) {
        case END_SUBSHELL_COMMAND:
            command_t subshell = (command_t)checked_malloc(sizeof(struct command));
            subshell->type = SUBSHELL_COMMAND;
            subshell->status = -1;
            subshell->input = subshell->output = 0;
            /* create tree for subcommand */
            while (flag && op_stack->size && opPeek(op_stack) != SUBSHELL_COMMAND)
            {
                int popped_op = opPop(op_stack);
                flag = createCommandTree(op_stack, com_stack, popped_op);
            }
            if (opPeek(op_stack) != SUBSHELL_COMMAND) /* error */
                return 0;
            else
            {
                opPop(op_stack);
                subshell->u.subshell_command = commandPop(com_stack); /* top command of com_stack is subcommand */
                commandPush(com_stack, subshell);
            }
            break;
            /* remember to free simple command */
        case LEFT_REDIRECT:
        case RIGHT_REDIRECT:
            command_t redir = commandPop(com_stack); /* input/output */
            command_t popped_com = commandPop(com_stack); /* incident command */
            if (redir && popped_com && redir->type == SIMPLE_COMMAND
                && redir->u.words[1] == NULL) /* input/output has to be 1-word SIMPLE_COMMAND */
            {
                if (com_type == LEFT_REDIRECT)
                    popped_com->input = redir->u.words[0];
                else
                    popped_com->output = redir->u.words[0];
                commandPush(com_stack, popped_com);
            }
            else
                return 0;
            break;
        case PIPE_COMMAND:
        case AND_COMMAND:
        case OR_COMMAND:
        case SEQUENCE_COMMAND:
            command_t binary = (command_t)checked_malloc(sizeof(command));
            binary->type = com_type;
            binary->status = -1;
            binary->input = binary->output = 0;
            command_t com2 = commandPop(com_stack); /* second half */
            command_t com1 = commandPop(com_stack); /* first half */
            if (com1 && com2)
            {
                binary->u.command[0] = com1;
                binary->u.command[1] = com2;
                commandPush(com_stack, binary);
            }
            else
                return 0;
            break;
        default:
            break;
    }
    return flag;
}

/* take one operator, rearrange the stack if necessary */
/* return 1 if successful, 0 otherwise */
int dealWithOperator(stackOp *op_stack, stackCom *com_stack, int op_type)
{
    int flag = 1; /* return value */
    switch (op_type)
    {
        case SUBSHELL_COMMAND:
            opPush(op_stack, op_type); /* push anyways */
            break;
        case END_SUBSHELL_COMMAND:
            flag = createCommandTree(op_stack, com_stack, op_type); /* pop everything before ( */
            break;
        case LEFT_REDIRECT:
        case RIGHT_REDIRECT:
            int top = opPeek(op_stack);
            while (flag && (top == LEFT_REDIRECT) || (top == RIGHT_REDIRECT))
            {
                int popped_op = opPop(op_stack);
                flag = createCommandTree(op_stack, com_stack, popped_op);
                top = opPeek(op_stack);
            }
            opPush(op_stack, op_type);
            break;
        case PIPE_COMMAND:
            int top = opPeek(op_stack);
            while (flag && (top == LEFT_REDIRECT) ||
                   (top == RIGHT_REDIRECT) || (top == PIPE_COMMAND))
            {
                int popped_op = opPop(op_stack);
                flag = createCommandTree(op_stack, com_stack, popped_op);
                top = opPeek(op_stack);
            }
            opPush(op_stack, op_type);
            break;
        case AND_COMMAND:
        case OR_COMMAND:
            int top = opPeek(op_stack);
            while (flag && (top == LEFT_REDIRECT) || (top == RIGHT_REDIRECT)
                   || (top == PIPE_COMMAND) || (top == AND_COMMAND) || (top == OR_COMMAND))
            {
                int popped_op = opPop(op_stack);
                flag = createCommandTree(op_stack, com_stack, popped_op);
                top = opPeek(op_stack);
            }
            opPush(op_stack, op_type);
            break;
        case SEQUENCE_COMMAND:
            int top = opPeek(op_stack);
            while (flag && (top == LEFT_REDIRECT) || (top == RIGHT_REDIRECT)
                   || (top == PIPE_COMMAND) || (top == AND_COMMAND) || (top == OR_COMMAND)
                   || (top != SEQUENCE_COMMAND))
            {
                int popped_op = opPop(op_stack);
                flag = createCommandTree(op_stack, com_stack, popped_op);
                top = opPeek(op_stack);
            }
            opPush(op_stack, op_type);
            break;
        default:
            break;
    }
    return flag;
}




