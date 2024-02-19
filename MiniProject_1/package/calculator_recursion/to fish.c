#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef __LEX__
#define __LEX__

#define MAXLEN 256
int cccount[9]={0};//QAQ
// Token types
typedef enum {
    UNKNOWN, END, ENDFILE,
    INT, ID,
    ADDSUB, MULDIV,
    ASSIGN,
    LPAREN, RPAREN,
    INCDEC, AND,
    OR, XOR
} TokenSet;

// Test if a token matches the current token
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__

#ifndef __PARSER__
#define __PARSER__

#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left;
    struct _Node *right;
    int r;
} BTNode;

// The symbol table
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the value of a variable
extern int getval(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern BTNode *factor(void);
extern BTNode *term(void);
extern BTNode *term_tail(BTNode *left);
extern BTNode *expr(void);
extern BTNode *expr_tail(BTNode *left);
extern void statement(void);
//by me
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *assign_expr(void);
//by me
// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__

// This package is a calculator
// It works like a Python interpretor
// Example:
// >> y = 2
// >> z = 2
// >> x = 3 * y + 4 / (2 * z)
// It will print the answer of every line
// You should turn it into an expression compiler
// And print the assembly code according to the input

// This is the grammar used in this package
// You can modify it according to the spec and the slide
// statement  :=  ENDFILE | END | expr END
// expr    	  :=  term expr_tail
// expr_tail  :=  ADDSUB term expr_tail | NiL
// term 	  :=  factor term_tail
// term_tail  :=  MULDIV factor term_tail| NiL
// factor	  :=  INT | ADDSUB INT |
//		   	      ID  | ADDSUB ID  |
//		   	      ID ASSIGN expr |
//		   	      LPAREN expr RPAREN |
//		   	      ADDSUB LPAREN expr RPAREN

int main() {
    //test
    //freopen("input.txt", "w", stdout);
    //
    initTable();
    //printf(">> ");
    while (1) {
        statement();
        for(int i=0;i<8;i++)cccount[i] = 0;
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
int check = 0;
BTNode *assign_expr()
{
    if(match(ID))
    {
        BTNode *left = makeNode(ID, getLexeme());
        advance();
        if(match(ASSIGN))
        {
            check = 1;
            BTNode *node = makeNode(ASSIGN, getLexeme());
            advance();
            node->left = left;
            node->right = assign_expr();
            return node;
        }
        else if(check ==1)
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
            printf("MOV r%d %d\n", i, table[i].val); //結束前輸出
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
        check = 0;
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');
    //ID(¦n¤@¥b)  //INCDEC //and or xor(OK)
    if (isdigit(c))
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        lexeme[0] = c;
        c = fgetc(stdin);
        if(c==lexeme[0])
        {
            lexeme[1] = c;
            lexeme[2] = '\0';
            return INCDEC;
        }
        else ungetc(c, stdin);
        lexeme[1] = '\0';
        return ADDSUB;
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    //by me
    else if(c=='&')
    {
       strcpy(lexeme, "&");
        return AND;
    }
    else if(c=='|')
    {
        strcpy(lexeme, "|");
        return OR;
    }
    else if(c=='^')
    {
       strcpy(lexeme, "^");
        return XOR;
    }
    //
    else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c)) { //by me
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c)||isdigit(c)||c=='_') && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    } else if (c == EOF) {
        return ENDFILE;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int evaluateTree(BTNode *root) {//2owo
    int retval = 0, lv = 0, rv = 0;
    //for(int i=0;i<8;i++)printf("i=%d cccount[i]=%d\n",i,cccount[i]);
    if (root != NULL) {
        int num = 0;
        switch (root->data) {
            case ID:
                //retval = getval(root->lexeme);//?owo
                //printf("one of ID is %c i=%d\n ",root->lexeme[0],cccount);
                for(int i=0;i<8;i++){
                  if(cccount[i]==0){
                    root->r = i;
                    cccount[i]=1;
                    break;
                  }
                }//QAQ
                for(int i=0;i<sbcount;i++)if(strcmp(root->lexeme,table[i].name)==0)num = i*4;//?owo

                printf("MOV r%d [%d]\n",root->r,num);
                break;
            case INT:
                retval = atoi(root->lexeme);//轉換為字符串參數str為整數（int型
                //printf("one of INT is %d i=%d\n",retval,cccount);
                for(int i=0;i<8;i++){
                  if(cccount[i]==0){
                    root->r = i;
                    cccount[i]=1;
                    break;
                  }
                }//QAQ
                printf("MOV r%d %d\n",root->r,retval);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                //printf("one of ASSIGN is %c  %c\n",root->lexeme[0],root->left->lexeme[0]);
                num = 0;
                for(int i=0;i<sbcount;i++)if(strcmp(root->left->lexeme,table[i].name)==0)num = i*4;//?owo
                printf("MOV [%d] r%d\n",num,root->right->r);
                break;
            case ADDSUB:
            case MULDIV:
            case AND:
            case OR:
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    //retval = lv + rv;
                    printf("ADD r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                } else if (strcmp(root->lexeme, "-") == 0) {
                    //retval = lv - rv;
                    printf("SUB r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                } else if (strcmp(root->lexeme, "*") == 0) {
                    //retval = lv * rv;
                    printf("MUL r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (root->right==NULL || root->right->lexeme[0]=='0')
                        error(DIVZERO);
                    //retval = lv / rv;
                    printf("DIV r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                } else if (strcmp(root->lexeme, "&") == 0) {
                    //retval = lv & rv;
                    printf("AND r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                } else if (strcmp(root->lexeme, "|") == 0) {
                    //retval = lv | rv;
                    printf("OR r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                } else if (strcmp(root->lexeme, "^") == 0) {
                    //retval = lv ^ rv;
                    printf("XOR r%d r%d\n",root->left->r,root->right->r);
                    cccount[root->right->r]=0;//QAQ
                    root->r = root->left->r;//QAQ
                }
                //printf("one of +-*/  is %c\n",root->lexeme[0]);
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
