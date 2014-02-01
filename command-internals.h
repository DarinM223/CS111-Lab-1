// UCLA CS 111 Lab 1 command internals

enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or 0 if none.
  char *input;
  char *output;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
};

struct _fileNode;
struct _dependencyNode;
struct _commandTreeNode;

struct _fileNode {
        char *file;
        struct _fileNode *next;
};

struct _dependencyNode {
        struct _commandTreeNode* dependency;
        struct _dependencyNode* next;
}; 

struct _commandTreeNode {
        struct command *comm;

        int numDependencies;
        struct _dependencyNode* dependencyList;

        struct _fileNode* inputList;
        struct _fileNode* outputList;

        int pid;
        struct _commandTreeNode* next;
};

