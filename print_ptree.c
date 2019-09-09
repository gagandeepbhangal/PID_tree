#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptree.h"

int check_for_int(int yo, char *temp){
  return 0;
}
int main(int argc, char **argv) {
    // TODO: Update error checking and add support for the optional -d flag
    // printf("Usage:\n\tptree [-d N] PID\n");

    // NOTE: This only works if no -d option is provided and does not
    // error check the provided argument or generate_ptree. Fix this!
    int pid = 0;
    int depth = 0;
    if (argc == 2){
      //Make sure we have an integer
      for (int i = 0; i < strlen(argv[1]); i++){
        if (argv[1][i]>= '0' && argv[1][i] <= '9'){
          pid = strtol(argv[1], NULL, 10);
        }
        else{
          fprintf(stderr, "%s\n", "Usage:\n\tptree [-d N] PID\n");
          return 1;
        }
        pid = strtol(argv[1], NULL, 10);
      }
    }
    else if (argc == 4){
      char *temp = argv[1];
      if (temp[0] == '-' && temp[1] == 'd' && strlen(temp) == 2){
        for (int i = 0; i < strlen(argv[2]); i++){
          if (argv[2][i]>= '0' && argv[2][i] <= '9'){
            pid = strtol(argv[2], NULL, 10);
          }else{
            fprintf(stderr, "%s\n", "Usage:\n\tptree [-d N] PID\n");
            return 1;
          }
          depth = strtol(argv[2], NULL, 10);
        }
        for (int i = 0; i < strlen(argv[3]); i++){
          if (argv[3][i]>= '0' && argv[3][i] <= '9'){
            pid = strtol(argv[3], NULL, 10);
          }else{
            fprintf(stderr, "%s\n", "Usage:\n\tptree [-d N] PID\n");
            return 1;
          }
          pid = strtol(argv[3], NULL, 10);
        }
      }
      else{
        fprintf(stderr, "%s\n", "Usage:\n\tptree [-d N] PID\n");
      	return 1;
      }
    }
    else{
      fprintf(stderr, "%s\n", "Invalid number of arguments\n");
    	return 1;
    }

    struct TreeNode *root = NULL;
    if (generate_ptree(&root, pid) == 1){
      print_ptree(root, depth);
      return 2;
    }
    print_ptree(root, depth);
    return 0;
}
