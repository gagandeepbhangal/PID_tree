#include <stdio.h>
// Add your system includes here.
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "ptree.h"

// Defining the constants described in ptree.h
const unsigned int MAX_PATH_LENGTH = 1024;
int err = 0;

// If TEST is defined (see the Makefile), will look in the tests
// directory for PIDs, instead of /proc.
#ifdef TEST
    const char *PROC_ROOT = "tests";
#else
    const char *PROC_ROOT = "/proc";
#endif

/*
 * Creates a PTree rooted at the process pid. The root of the tree is
 * placed in root. The function returns 0 if the tree was created
 * successfully and 1 if the tree could not be created or if at least
 * one PID was encountered that could not be found or was not an
 * executing process.
 */
int create_children(struct TreeNode **, pid_t);
int get_files(struct TreeNode **, pid_t, FILE **);

int get_files(struct TreeNode **root, pid_t pid, FILE **children){
  struct stat buf;
  char proc_children[MAX_PATH_LENGTH + 1];
  if (snprintf(proc_children, MAX_PATH_LENGTH + 1, "%s/%d/task/%d/children", PROC_ROOT, pid, pid) < 0) {
    (*root)->child_procs = NULL;
  }
  else{
    if (lstat(proc_children, &buf) != 0){
      (*root)->child_procs = NULL;
      err = 1;
      return 2;
    }
    *children = fopen(proc_children, "r");
    if (*children == NULL){
      err = 1;
      return 1;
    }
  }
  return 0;
}

int set_name(struct TreeNode *root, pid_t pid){
  struct stat buf;
  char proc_cmd[MAX_PATH_LENGTH + 1];
  if (snprintf(proc_cmd, MAX_PATH_LENGTH + 1, "%s/%d/cmdline", PROC_ROOT, pid) < 0) {
      root->name = NULL;
  }
  else{
    if (lstat(proc_cmd, &buf) != 0){
      root->name = NULL;
      err = 1;
      return 0;
    }
    FILE *cmd = fopen(proc_cmd, "r");
    if (cmd == NULL){
      err = 1;
      return 1;
    }
    root->name = malloc((sizeof(char) * MAX_PATH_LENGTH)+1);
    char n[MAX_PATH_LENGTH + 1];
    if (fscanf(cmd, "%s", n) != 1){
      root->name = NULL;
    }
    else{
      char *name;
	    char *token = strtok(n, "/");
	    name = token;
	    while (token != NULL){
        name = token;
        token = strtok(NULL, "/");
      } 
      strncpy(root->name, name, MAX_PATH_LENGTH+1);
    }

    if (fclose(cmd) != 0){
      err = 1;
      return 1;
    }
  }
  return 0;
}

int generate_ptree(struct TreeNode **root, pid_t pid){
  char procfile[MAX_PATH_LENGTH + 1];
  if (snprintf(procfile, MAX_PATH_LENGTH + 1, "%s/%d/exe", PROC_ROOT, pid) < 0) {
    err = 1;
    return err;
  }
  struct stat buf;
  if (lstat(procfile, &buf) != 0){
    err = 1;
    return err;
  }
  //Create space for the first node then send it into create_children to
  //generate the next child
  *root = malloc(sizeof(struct TreeNode));
  if ((*root) == NULL){
    err = 1;
    return err;
  }
  return create_children(root, pid);
}

int create_children(struct TreeNode **root, pid_t pid) {

  char procpid[MAX_PATH_LENGTH + 1];
  if (snprintf(procpid, MAX_PATH_LENGTH + 1, "%s/%d", PROC_ROOT, pid) < 0){
    *root = NULL;
    free(*root);
    err = 1;
    return err;
  }
  char procfile[MAX_PATH_LENGTH + 1];
  if (snprintf(procfile, MAX_PATH_LENGTH + 1, "%s/%d/exe", PROC_ROOT, pid) < 0) {
    *root = NULL;
    free(*root);
    err = 1;
    return err;
  }
  struct stat buf;
  if (lstat(procpid, &buf) != 0){
    *root = NULL;
    free(*root);
    err = 1;
    return err;
  }
  if (lstat(procfile, &buf) != 0){
    *root = NULL;
    free(*root);
    err = 1;
    return err;
  }
  (*root)->pid = pid;
  (*root)->child_procs = NULL;
  (*root)->next_sibling = NULL;
  int error = set_name((*root), pid);
  if (error == 1){
    err = 1;
    return err;
  }
  FILE *children = malloc(sizeof(*children));
  int errorr = get_files(root, pid, &children);
  if (errorr == 1){
    err = 1;
    return err;
  }
  if (errorr == 2){
    return 1;
  }
  char child_pid[MAX_PATH_LENGTH];
  struct TreeNode *temp;
  while(fscanf(children, "%s", child_pid) != EOF){
    if ((*root)->child_procs == NULL){
      //create space for the child
      (*root)->child_procs = malloc(sizeof(struct TreeNode));
      if ((*root)->child_procs == NULL){
        //Failed to allocate memory on heap
        err = 1;
        return err;
      }
      //recurse into the next child
		  create_children(&(*root)->child_procs, strtol(child_pid, NULL, 10));
      //keeps track of the last nodes child so we can recurse into its siblings
      temp = (*root)->child_procs;
	  }
    else{
        while (temp->next_sibling != NULL){
          //loop until we reach the sibling that has no sibling
          //this is so we don't overwrite any previously created siblings
          temp = temp->next_sibling;
        }
        //create space for the new sibling
        temp->next_sibling = malloc(sizeof(struct TreeNode));
        if (temp->next_sibling == NULL){
          //Failed to allocate memory on heap
          err = 1;
          return err;
        }
        //recurse into the new sibling and make its children (if any)
		    create_children(&temp->next_sibling, strtol(child_pid, NULL, 10));
	  }
  }
  if (fclose(children) != 0){
    err = 1;
    return err;
  }
  return err;
}

/*
 * Prints the TreeNodes encountered on a preorder traversal of an PTree
 * to a specified maximum depth. If the maximum depth is 0, then the
 * entire tree is printed.
 */
void print_ptree(struct TreeNode *root, int max_depth) {
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    if (root != NULL){
      // Your implementation goes here.
      if (root->name != NULL){
        for (int i = 0; i < depth; i++){
          printf("  ");
        }
        printf("%d: %s\n", root->pid, root->name);
      }
      else{
        if (root->pid){
          for (int i = 0; i < depth; i++){
            printf("  ");
          }
          printf("%d\n", root->pid);
        }
      }
      if (max_depth == 0){
        if (root->child_procs != NULL){
          //recurse into the next child and increment depth by 1
          depth += 1;
          print_ptree(root->child_procs, max_depth);
          //decrement the depth
          depth --;
        }
      }
      else if (depth < max_depth - 1){
        if (root->child_procs != NULL){
          //recurse into the next child and increment depth by 1
          depth += 1;
          print_ptree(root->child_procs, max_depth);
          //decrement the depth
          depth --;
        }
      }
      //This is run when there is no more children for the current root
      if (root->next_sibling != NULL){
        //Now we recurse into its sibling and print out its children
        print_ptree(root->next_sibling, max_depth);
      }
    }
}
