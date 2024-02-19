#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef __LEX__
#define __LEX__
#define MAXLEN 256
int used[9]={0};
int mem[270]={0};
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

int main()
{
    initTable();
    for(int i =0;i<8;i++) printf("MOV r%d 0\n", i);
    //freopen("input.txt", "w", stdout);
    printf("MOV r0 [0]\nMOV r1 [4]\nMOV r2 [8]\n");
    printf("MOV [0] r0\nMOV [4] r1\nMOV [8] r2\n");
    while (1)
    {
        statement();
        for(int i=0;i<8;i++)
        {
            used[i] = 0;
        }

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
int check=0;
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        retp=left = makeNode(ID, getLexeme());
        advance();
        if(check==1)
        {
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
        advance();
        if (match(ID))
        {
            retp->left = makeNode(ID, getLexeme());
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
BTNode* assign_expr_tail(BTNode *left);
BTNode* assign_expr(void)
{
    BTNode *left = or_expr();
    if(left->data!=ID&&left->data!=INCDEC) err(1);
    BTNode *node = assign_expr_tail(left);
    node->left = left;
    check=1;
    node->right = or_expr();
    return node;
}
BTNode* assign_expr_tail(BTNode *left)
{
    BTNode *node = NULL;
    if (match(ASSIGN))
    {
        node = makeNode(ASSIGN, getLexeme());
        advance();
        node->left = left;
        advance();
        BTNode *first =
        if()
        node->right = or_expr();
        if(node->right->data==ID) advance();
        if(match(ASSIGN)) assign_expr_tail(node);
        else return node;
    }
    else return left;
}
// statement := ENDFILE | END | assign_expr END
void statement(void) {
    BTNode *retp = NULL;

    if (match(ENDFILE))
    {
        for(int i = 0;i<3;i++)
        {
            printf("MOV r%d [%d]\n", i, i*4);
        }
        printf("EXIT 0");
        exit(0);
    }
    else if (match(END))
    {
        advance();
    }
    else
    {
        check = 0;
        retp = assign_expr();
        if (match(END))
        {
            int c =  evaluateTree(retp);
            freeTree(retp);
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
int is_there_id;
void check_tree_id(BTNode *root)
{
    if(root!=NULL)
    {
        if(root->data ==ID) is_there_id = 1;
        else
        {
            check_tree_id(root->right);
            check_tree_id(root->left);
        }
    }
}
char *getLexeme(void) {
    return lexeme;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int ww, bowshit, bowshit2;
int evaluateTree(BTNode *root)
{
    int retval = 0, lv = 0, rv = 0;
    if (root != NULL) {
        int num = 0;
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                for(int i=0;i<8;i++)
                {
                  if(used[i]==0)
                  {
                    root->r = i;
                    used[i]=1;
                    break;
                  }
                }
                for(int i=0;i<sbcount;i++)if(strcmp(root->lexeme,table[i].name)==0)num = i*4;

                printf("MOV r%d [%d]\n",root->r,num);
                break;
            case INT:
                retval = atoi(root->lexeme);
                for(int i=0;i<8;i++)
                {
                  if(used[i]==0)
                  {
                    root->r = i;
                    used[i]=1;
                    break;
                  }
                }
                printf("MOV r%d %d\n",root->r,retval);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                retval = setval(root->left->lexeme, rv);
                num = 0;
                for(int i=0;i<sbcount;i++)if(strcmp(root->left->lexeme,table[i].name)==0)num = i*4;
                BTNode* curr = root->right;
                while(curr->right!=NULL&&curr->data==ASSIGN)
                {
                    curr = curr->right;
                }
                printf("MOV [%d] r%d\n",num, curr->r);
                root->r = root->right->r;
                break;
            case ADDSUB:
            case MULDIV:
            case AND:
            case OR:
            case XOR:
                lv = evaluateTree(root->left);
                rv = evaluateTree(root->right);
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    printf("ADD r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                    printf("MUL r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv==0)
                    {
                        if(root->right->data==INT)
                        {
                           error(DIVZERO);
                        }
                        else
                        {
                            is_there_id = 0;
                            check_tree_id(root->right);
                            if(!is_there_id)error(DIVZERO);
                            retval =0;
                        }
                    }
                    else retval = lv / rv;
                    printf("DIV r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                } else if (strcmp(root->lexeme, "&") == 0) {
                    retval = lv & rv;
                    printf("AND r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                } else if (strcmp(root->lexeme, "|") == 0) {
                    retval = lv | rv;
                    printf("OR r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                } else if (strcmp(root->lexeme, "^") == 0) {
                    retval = lv ^ rv;
                    printf("XOR r%d r%d\n",root->left->r,root->right->r);
                    used[root->right->r]=0;
                    root->r = root->left->r;
                }
                break;
            case INCDEC:
                for(int i = 0;i<sbcount;i++) if(strcmp(root->left->lexeme, table[i].name)==0) num = i*4;
                for(int i=0;i<8;i++)
                {
                    if(used[i]==0)
                    {
                        bowshit = i;
                        used[i]=1;
                        break;
                    }
                }
                for(int i=0;i<8;i++)
                {
                    if(used[i]==0)
                    {
                        bowshit2 = i;
                        used[i]=1;
                        break;
                    }
                }
                if(strcmp(root->lexeme,"++")==0)
                {
                    printf("MOV r%d 1\n", bowshit);
                    printf("MOV r%d [%d]\n",bowshit2 ,num);
                    printf("ADD r%d r%d\n", bowshit2, bowshit);
                     printf("MOV [%d] r%d\n",num ,bowshit2);
                    ww = setval(root->left->lexeme, getval(root->left->lexeme)+1);
                }
                else
                {
                    printf("MOV r%d 1\n", bowshit);
                    printf("MOV r%d [%d]\n",bowshit2 ,num);
                    printf("SUB r%d r%d\n", bowshit2, bowshit);
                    printf("MOV [%d] r%d\n",num ,bowshit2);
                    ww = setval(root->left->lexeme, getval(root->left->lexeme)-1);
                }
                retval = ww;
                root->r = bowshit2;
                used[bowshit] = 0;
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL)
    {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
