#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

   // strcpy(table[sbcount].name, str);
   // table[sbcount].val = 0;
   // sbcount++;

    printf("EXIT 1");
    exit(0);
    return 0;
}

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// factor := INT | ADDSUB INT | //ok
//		   	 ID  | ADDSUB ID  |//ok
//		   	 ID ASSIGN expr |//ok
//		   	 LPAREN expr RPAREN |//ok
//		   	 ADDSUB LPAREN expr RPAREN//ok
//by me: incdec ID
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        left = makeNode(ID, getLexeme());
        advance();
        if (!match(ASSIGN))
        {
            retp = left;
        }
        else
        {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = expr();
        }
    }
    else if (match(ADDSUB))
    {
        retp = makeNode(ADDSUB, getLexeme());
        retp->left = makeNode(INT, "0");
        advance();
        if (match(INT))
        {
            retp->right = makeNode(INT, getLexeme());
            advance();
        }
         else if (match(ID))
        {
            retp->right = makeNode(ID, getLexeme());
            advance();
        }
        else if (match(LPAREN))
        {
            advance();
            retp->right = assign_expr();
            if (match(RPAREN))
                advance();
            else
                error(MISPAREN);
        }
        else
        {
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        advance();
        retp = expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    }
     else if(match(INCDEC)) //by me incdec id
     {
        if(strcmp(getLexeme(),"--")==0) retp = makeNode(INCDEC, "--");
        else retp = makeNode(INCDEC, "++");
        //retp->right = makeNode(INT, "1");
        advance();
        if (match(ID))
        {
            retp->left = makeNode(ID, getLexeme());
            //if(strcmp(retp->lexeme,"+")==0) int ww = setval(retp->left->lexeme, getval(retp->left->lexeme)+1);
            //else int ww = setval(retp->left->lexeme, getval(retp->left->lexeme)-1);
            advance();
        }
        else
        {
            error(NOTNUMID);
        }
     }
     else
     {
        error(NOTNUMID);
     }
    return retp;
}
//
//unary_expr       := ADDSUB unary_expr | factor
BTNode *unary_expr()
{
    if(match(ADDSUB))
    {
        BTNode *node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = makeNode(INT, "0");
        node->right = unary_expr();
        return node;
    }
    else
    {
        return factor();
    }
}

// term := factor term_tail //muldiv_expr
BTNode *term(void)
{
    BTNode *node = unary_expr();
    return term_tail(node);
}

// term_tail := MULDIV factor term_tail | NiL //muldiv_expr_tail
BTNode *term_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return term_tail(node);
    } else {
        return left;
    }
}

// expr := term expr_tail //addsub_expr
BTNode *expr(void) {
    BTNode *node = term();
    return expr_tail(node);
}

// expr_tail := ADDSUB term expr_tail | NiL //addsub_expr_tail
BTNode *expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = term();
        return expr_tail(node);
    } else {
        return left;
    }
}
//by me and_expr and_expr_tail
BTNode *and_expr(void)
{
    BTNode *node = expr();
    return and_expr_tail(node);
}
BTNode *and_expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(AND))
    {
        node = makeNode(AND, getLexeme());
        advance();
        node->left = left;
        node->right = expr();
        return and_expr_tail(node);
    } else {
        return left;
    }
}
//by me xor_expr xor_expr_tail
BTNode *xor_expr(void)
{
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}
BTNode *xor_expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(XOR))
    {
        node = makeNode(XOR, getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    } else {
        return left;
    }
}
//by me or_expr or_expr_tail
BTNode *or_expr(void)
{
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}
BTNode *or_expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(OR))
    {
        node = makeNode(OR, getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else
    {
        return left;
    }
}
//by me assign_expr //ID ASSIGN assign_expr | or_expr
BTNode *assign_expr()
{
    if(match(ID))
    {
        BTNode *left = makeNode(ID, getLexeme());
        advance();
        if(match(ASSIGN))
        {
            BTNode *node = makeNode(ASSIGN, getLexeme());
            advance();
            node->left = left;
            node->right = assign_expr();
            return node;
        }
        else
        {
            if(match(OR)) return or_expr_tail(left);
            else if(match(XOR)) return xor_expr_tail(left);
            else if(match(AND)) return and_expr_tail(left);
            else if(match(ADDSUB)) return expr_tail(left);
            else if(match(MULDIV)) return term_tail(left);
            else return left;
        }
    }
    else return or_expr();
}
// statement := ENDFILE | END | assign_expr END
void statement(void) {
    BTNode *retp = NULL;

    if (match(ENDFILE))
    {
        for(int i = 0;i<3;i++)
        {
            printf("MOV r%d %d\n", i, table[i].val); //µ²§ô«e¿é¥X
        }
        printf("EXIT 0");
        exit(0);
    }
    else if (match(END))
    {
        //printf(">> ");
        advance();
    }
    else
    {
        retp = assign_expr();
        if (match(END))
        {
          int c =  evaluateTree(retp);
            //printf("%d\n", evaluateTree(retp));
            //printf("Prefix traversal: ");
            //printPrefix(retp);
            //printf("\n");
            freeTree(retp);
            //printf(">> ");
            advance();
        }
        else
        {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    printf("EXIT 1");
    exit(0);
}
