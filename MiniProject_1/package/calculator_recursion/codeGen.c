#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"


int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                break;
            case INT:
                retval = atoi(root->lexeme);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                for(int i = 0;i<3;i++)
                {
                        if(strcmp(root->left->lexeme, table[i].name) == 0)
                        {
                            printf("MOV r%d %d\n",i ,retval);
                            break;
                        }
                }
                //printf("MOV r3 %d\n", retval);
                //printf("MOV [")

                break;
            case ADDSUB:
            case MULDIV:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                } else if (strcmp(root->lexeme, "/") == 0)
                {
                    if (rv == 0)S
                    {
                        error(DIVZERO);
                    }
                    retval = lv / rv;
                }
                break;
            case AND:
            case OR:
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "&") == 0) {
                    retval = lv & rv;
                } else if (strcmp(root->lexeme, "|") == 0) {
                    retval = lv | rv;
                } else if (strcmp(root->lexeme, "^") == 0) {
                    retval = lv ^ rv;
                }
                break;
            case INCDEC:
                if(strcmp(root->lexeme,"++")==0) int ww = setval(root->left->lexeme, getval(root->left->lexeme)+1);
                else int ww = setval(root->left->lexeme, getval(root->left->lexeme)-1);
                retval = evaluateTree(root->left);
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}
int putid(int initreg){
    if(r0==0){
      printf("MOV r0 [%d]",initreg);
      printf("\n");
        r0=1;
      return 0;//回傳現在在哪兒
    }
    else if(r1==0){
      printf("MOV r1 [%d]",initreg);
         printf("\n");
        r1=1;
      return 1;
    }
      else{
      printf("MOV r2 [%d]",initreg);
           printf("\n");
        r2=1;
      return 2;
    }
}
int putint(int val){
    if(r0==0){
        r0=1;
        printf("MOV r0 %d",val);
         printf("\n");
        return 0;
    }else if(r1==0){
        r1=1;
        printf("MOV r1 %d",val);
         printf("\n");
        return 1;
    }else{
        r2=1;
        printf("MOV r2 %d",val);
         printf("\n");
        return 2;
    }

}
