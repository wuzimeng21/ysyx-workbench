/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
// add
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum
{
    TK_NOTYPE = 256,

    /* TODO: Add more token types */
    ZUOK = 10,
    YOUK = 11,
    MUL = 31,
    DIV = 32,
    ADD = 41,
    SUB = 42,
    LEQ = 61,
    GEQ = 62,
    LESS = 63,
    GET = 64,
    TK_EQ = 71,
    TK_NOT_EQ = 72,

    AND = 110,
    OR = 120,

    TK_NUM = 1010,
    HEX = 1011,
    REG = 1012,
    DEREF = 1013,
};

static struct rule
{
    const char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", ADD},      // plus
    {"\\==", TK_EQ},   // equal
    {"\\-", SUB},      // sub
    {"\\*", MUL},
    {"\\/", DIV},
    {"\\(", ZUOK},
    {"\\)", YOUK},
    {"[0-9]*", TK_NUM},
    {"0[xX][[0-9a-fA-F]]+", HEX},
    {"\\!\\=", TK_NOT_EQ},
    {"\\<\\=", LEQ},
    {"\\>\\=", GEQ},
    {"<", LESS},
    {">", GET},
    {"\\&\\&", AND},
    {"\\|\\|", OR},
    {"$[a-zA-Z][0-9]", REG},
    // {"", PONITER},

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
    int i;
    char error_msg[128];
    int ret;
    // printf("NR_REGEX: %d\n", NR_REGEX);

    for (i = 0; i < NR_REGEX; i++)
    {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        // printf("init: %s   %s\n", rules[i].regex, re[i]);
        if (ret != 0)
        {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token
{
    int type;
    char str[32];
} Token;

static Token tokens[128] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;
int zero = 0;

int num_to_str(int x, char e[], int mark)
{
    // init the str
    memset(e, 0, strlen(e));
    int i = 0;
    // int l = 0;
    int tmp = x;
    int k = 1;
    if (mark == 1)
    {
        ;
    }
    else
    {
        e[i++] = '-';
    }

    while (tmp)
    {
        // l ++;
        tmp = tmp / 10;
        k = k * 10;
    }
    k = k / 10;

    while (k != 0)
    {
        // printf("k : %d\n", k);
        int a = x / k;
        x = x % k;
        // printf("a: %d\n", a);
        e[i++] = a + '0';
        k = k / 10;
    }

    return i;
}

static bool make_tokens(char *e)
{
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;
    zero = 0;

    while (e[position] != '\0')
    {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++)
        {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
            {
                 char *substr_start = e + position;

                int substr_len = pmatch.rm_eo;
                bool flag = true;
                if (e[position] == '<')
                {
                    flag = false;
                    if (e[position + 1] == '=')
                    {
                        substr_len = 2;
                        tokens[nr_token].type = LEQ;
                    }
                    else
                    {
                        substr_len = 1;
                        tokens[nr_token].type = LESS;
                    }
                }
                else if (e[position] == '>')
                {
                    flag = false;
                    if (e[position + 1] == '=')
                    {
                        substr_len = 2;
                        tokens[nr_token].type = GEQ;
                    }
                    else
                    {
                        substr_len = 1;
                        tokens[nr_token].type = GET;
                    }
                }
                else if (e[position] == '&')
                {
                    flag = false;
                    substr_len = 2;
                    tokens[nr_token].type = AND;
                }
                else if (e[position] == '|')
                {
                    flag = false;
                    substr_len = 2;
                    tokens[nr_token].type = OR;
                }
                else if (e[position] == '!')
                {
                    flag = false;
                    substr_len = 2;
                    tokens[nr_token].type = TK_NOT_EQ;
                }
                else if (e[position] == '=')
                {
                    flag = false;
                    substr_len = 2;
                    tokens[nr_token].type = TK_EQ;
                }
                else if (e[position] == '0' && (e[position + 1] == 'x' || e[position + 1] == 'X'))
                {
                    flag = false;
                    // substr_len = 2;
                    tokens[nr_token].type = HEX;
                    int l_hex = 0;
                    while (((e[position + l_hex] - '0' <= 9 && e[position + l_hex] - '0' >= 0) ||
                            ((e[position + l_hex] - '0' <= 'Z' - '0' && e[position + l_hex] - '0' >= 'A' - '0') ||
                             (e[position + l_hex] - '0' <= 'z' - '0' && e[position + l_hex] - '0' >= 'a' - '0'))) &&
                           e[position + l_hex] != ' ')
                    {
                        l_hex++;
                    }
                    substr_len = l_hex;
                }
                else if (e[position] == '$')
                {
                    printf("REG cmp OK..\n");
                    flag = false;
                    substr_len = 3;
                    tokens[nr_token].type = REG;
                }

                position += substr_len;

                if (flag == false)
                {
                    strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                    //printf("tokens: %s\n", tokens[nr_token].str);
                    nr_token++;
                }

                 Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                  i, rules[i].regex, position, substr_len, substr_len, substr_start);

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */
                switch (rules[i].token_type)
                {
                case ADD:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case SUB:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case MUL:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case DIV:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case YOUK:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case ZUOK:
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                case TK_NUM:
                    if (flag == true)
                    {
                        tokens[nr_token].type = rules[i].token_type;
                        strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                        nr_token++;
                    }
                    else
                    {
                        ;
                    }
                    break;
                case TK_NOTYPE:
                    break;
                case HEX:
                    tokens[nr_token].type = rules[i].token_type;
                    strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                    nr_token++;
                    break;
                case REG:
                    break;
                default: // TODO();
                    printf("err: at the position %d\n", position);
                    printf("op is %d\n", rules[i].token_type);
                    break;
                }

                break;
            }
        }

        if (i == NR_REGEX)
        {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    return true;
}

// stack
typedef struct {
    int items[10000];
    int top;
} Stack;

void initStack(Stack *s) {
    s->top = -1;
}

void push(Stack *s, int item) {
    if (s->top < 10000 - 1) {
        s->items[++(s->top)] = item;
    }
}

bool isEmpty(Stack *s) {
    return s->top == -1;
}

int pop(Stack *s) {
    if (!isEmpty(s)) {
        return s->items[(s->top)--];
    }
    return -1; // empty
}

bool check_parentheses(int p, int q)
{
    if (tokens[p].type != ZUOK || tokens[q].type != YOUK)
        return false;
    int l = p, r = q;
    // for(int i = l; i <= r; i ++) {
    //     printf("+%s", tokens[i].str);
    // }
    // printf("\n");

    // check out "(expression)" like this
    while (l < r)
    {

        if (tokens[l].type == ZUOK)
        {
            Stack stack;
            initStack(&stack);
            int idx = -1;
            // printf("l r: %d %d\n", l, r);
            for(int i = l; i <= r; i ++) {
                if(tokens[i].type == ZUOK) {
                    // printf("push zuok\n");
                    push(&stack, i);
                }
                else if(tokens[i].type == YOUK) {
                    // Stack is not empty && have ( in Stack
                    if(isEmpty(&stack) != true) {
                        // printf("pop zuok\n");
                        int tmp = pop(&stack);
                        // printf("tmp: %d\n", tmp);
                        if(tmp == l) {
                            idx  = i;
                        }
                    }
                }
            }
            // printf("idx (: %d\n", idx);
            // It is necessary to determine whether this ')' matches with this '('.
            if (tokens[r].type == YOUK)
            {
                if(idx == r){
                    return true;
                }
                else {
                    return false;
                }
            }
            else
            {
                r--;
            }
            // printf("pos: %d %d\n", l, r);
        }
        else if (tokens[l].type == YOUK)
        {
            return false;
        }
        else
        {
            l++;
        }
    }

    return true;
}

int find_op(int p, int q)
{
    int res = 0;
    int pos1 = p;

    for (int i = p; i <= q; i++)
    {
        int tmp = 0;
        if (tokens[i].type == ZUOK)
        {
            tmp++;
            while (tmp != 0)
            {
                i++;
                if (tokens[i].type == ZUOK)
                {
                    tmp++;
                }
                else if (tokens[i].type == YOUK)
                {
                    tmp--;
                }
            }
        }
        else if (tokens[i].type != ZUOK && tokens[i].type != TK_NUM && tokens[i].type != TK_NOTYPE && tokens[i].type != HEX && tokens[i].type != REG)
        {
            int tmp_res = tokens[i].type / 10;
            if (res <= tmp_res)
            {
                res = tmp_res;
                pos1 = i;
            }
        }
    }

    return pos1;
}

uint32_t eval(int p, int q)
{
    // printf("p and q: %d %d\n", p, q);
    int op = 0;
    if (p > q)
    {
        printf("\nATTENTION: p and q: %d %d, p > q\n\n\n", p, q);
        // assert(0);
        return INT_MAX;
    }
    else if (p == q)
    {
        return atoi(tokens[p].str);
    }
    else if (p < q && check_parentheses(p, q) == true)
    {
        // printf("ENTER TRUE\n");
        // deal with 1 + ()
        if(p + 1 == q) {
            // deal with ()
            // printf("enter p + 1 == q\n");
            return 0;
        }
        return eval(p + 1, q - 1);
    }
    else if (p < q && check_parentheses(p, q) == false)
    {
        // printf("enter false\n");
        op = find_op(p, q);
        int add_cnt = 0, sub_cnt = 0;
        while(tokens[p].type == ADD || tokens[p].type == SUB) {
            if(tokens[p].type == ADD) add_cnt++;
            else if(tokens[p].type == SUB) sub_cnt++;
            p ++;
        }

        uint32_t val1 = 0;
        uint32_t val2 = 0;
        if(add_cnt || sub_cnt) {
            val2 = 0;
            for(int i = p; i <= q; i ++) {
                printf("%s", tokens[i].str);
            }

            val1 = eval(p, q);
        }
        else {
            val1 = eval(p, op - 1);
            val2 = eval(op + 1, q);
        }
        printf("op: %d\t val1: %d\t val2: %d\n", tokens[op].type, val1, val2);
        int flag = 1;
        if(sub_cnt % 2 == 1){
            flag *= -1;
        }
        // if(val1 < 0) {
        //     printf("val1 < 0\n");
        //     // flag *= -1;
        //     val1 *= -1;
        //     for(int i = p; i <= q; i ++) {
        //         printf("%s", tokens[i].str);
        //     }
        //     printf("\n");
        //     // for(int i = p; i <= q; i ++) {
        //     //     tokens[i + 1] =  tokens[i];
        //     // }
        //     // tokens[p].type = SUB;
        // }
        // if(val2 < 0) {
        //     flag *= -1;
        //     val2 *= -1;
        // }
        
        switch (tokens[op].type)
        {
        case ADD:
            return flag* (val1 + val2);
        case SUB:
            return flag * (val1 - val2);
        case MUL:
            return flag * val1 * val2;
        case DIV:
            if (val2 == 0)
            {
                zero = 1;
            }
            if (zero == 0)
            {
                // printf("num::::%d\n", -val1 / val2);
                // printf("num::::%d\n", -(-val1 / val2));
                return val1 / val2;
            }
            else
            {
                printf("\nATTENTION: divide 0!!!\n\n\n");
                // assert(0);
                return INT_MAX;
            }
        case AND:
            return val1 && val2;
        case OR:
            return val1 || val2;
        case TK_EQ:
            return val1 == val2;
        case TK_NOT_EQ:
            return val1 != val2;
        case LEQ:
            return val1 <= val2;
        case GEQ:
            return val1 >= val2;
        case LESS:
            return val1 < val2;
        case GET:
            return val1 > val2;
        default:
            printf("No type match...\n");
            assert(0);
            break;
        }
    }

    return 0;
}

void hex_to_num()
{
    for (int i = 0; i < nr_token; i++)
    {
        if (tokens[i].type == HEX)
        {
            int tmp_hex = strtol(tokens[i].str, NULL, 16);
            // printf("tmp_hex: %d\n", tmp_hex);
            if (tmp_hex >= 0)
                num_to_str(tmp_hex, tokens[i].str, 1);
            else
                num_to_str(-tmp_hex, tokens[i].str, 0);
            tokens[i].type = TK_NUM;
        }
    }
    return;
}

void reg_to_num()
{
    for (int i = 0; i < nr_token; i++)
    {
        if (tokens[i].type == REG)
        {
            bool suc_reg_to_num = false;
            printf("REG: %s\n", tokens[i].str);
            word_t tmp_num = isa_reg_str2val(tokens[i].str, &suc_reg_to_num);
            if (suc_reg_to_num == false)
            {
                printf("transfer reg value to num value err..\n");
                assert(0);
            }
            if (tmp_num >= 0)
                num_to_str(tmp_num, tokens[i].str, 1);
            else
                num_to_str(-tmp_num, tokens[i].str, 0);
            tokens[i].type = TK_NUM;
        }
    }
    return;
}

word_t expr(char *e, bool *success)
{
    char *clr = "  ";
    for (int i = 0; i < 128; i++)
    {
        strcpy(tokens[i].str, clr);
        tokens[i].type = TK_NOTYPE;
    }
    printf("expression: %s\n", e);
    // printf("Before make_tokens\n");
    if (!make_tokens(e))
    {
        *success = false;
        printf("make_tokens fails..\n");
        return 0;
    }

    printf("before deal with HEX/TK_NUM/REG/DEREF nr_token is %d.\n", nr_token);

    // deal with expression "()"
    if(nr_token == 2 && tokens[0].type == ZUOK && tokens[1].type == YOUK) {
        printf("\nATTENTION: please do not input expression like (). \n\n");
        // assert(0);
        return INT_MAX;
    }

    /* HEX to TK_NUM*/
    hex_to_num();

    /* REG to TK_NUM*/
    reg_to_num();

    // find -NUM
    // int cnt = 0;
    // for (int i = 0; i < nr_token; i++)
    // {
    //     if (tokens[i].type == SUB && i > 0)
    //     {
    //         if (tokens[i - 1].type != TK_NOTYPE && tokens[i - 1].type != YOUK && tokens[i - 1].type != TK_NUM)
    //         {
    //             tokens[i].type = TK_NUM;
    //             int num_f = strtol(tokens[i + 1].str, NULL, 10);
    //             num_to_str(num_f, tokens[i].str, 0);
    //             for (int j = i + 1; j < nr_token - 1; j++)
    //             {
    //                 tokens[j].type = tokens[j + 1].type;
    //                 strcpy(tokens[j].str, tokens[j + 1].str);
    //             }

    //             cnt++;
    //         }
    //     }
    // }

    // nr_token = nr_token - cnt;

    /* TODO: Implement code to evaluate the expression. */
    // rules: (* HEX) || (* REG) || (* TK_NUM) if i != 0  // need use ()
    // 		else:
    // 			i == 0 && (* HEX || *REG || * TK_NUM)  /// no () is OK
    // for (int i = 0; i < nr_token; i++)
    // {
    //     printf("see see: %d %s\n", tokens[i].type, tokens[i].str);
    // }

   for (int i = 0; i < nr_token; i++)
    {
        if (tokens[i].type == MUL && (i == 0 || tokens[i - 1].type == ZUOK))
        {
            tokens[i].type = DEREF;
            assert(tokens[i + 1].type == TK_NUM);
            // calculate (* xxx )
            int tmp_jie1 = atoi(tokens[i + 1].str);
	    printf("tmp_jie1: %d\n", tmp_jie1);
	   //int *ptr = &tmp_jie1;
	   intptr_t ptr_value = (intptr_t)tmp_jie1;
	   printf("OK..Now we have ptr_value...But maybe we can not get value from it..\n");
	   int * ptr = (int *)ptr_value;
	   int res_jie = *ptr;
	   printf("The value we get from the pointer: %d\n", res_jie);

	   if (res_jie >= 0)
	      num_to_str(res_jie, tokens[i].str, 1);
	   else
	      num_to_str(-res_jie, tokens[i].str, 0);
	   tokens[i].type = TK_NUM;
	   printf("The value we get from the pointer is already in the tokens.\n");
	   // strcpy(tokens[i].str, str_tmp_jie);
           
            if (i == 0)
            {		
                // HEX || TK_NUM || REG
                tokens[i + 1].type = TK_NOTYPE;
                strcpy(tokens[i + 1].str, clr);

                nr_token -= (1);
            }
            else if (tokens[i - 1].type == ZUOK)
            {
                // '('
                tokens[i - 1].type = TK_NOTYPE;
                strcpy(tokens[i - 1].str, clr);

                // HEX || TK_NUM || REG
                tokens[i + 1].type = TK_NOTYPE;
                strcpy(tokens[i + 1].str, clr);

                // ')'
                tokens[i + 2].type = TK_NOTYPE;
                strcpy(tokens[i + 2].str, clr);
                nr_token -= (3);
            }
        }
    }

    uint32_t ans = 0;

    printf("nr_token: %d\n", nr_token);
    // for (int i = 0; i < nr_token; i++)
    // {
    //     printf("see see: %d %s\n", tokens[i].type, tokens[i].str);
    // }

    /* TODO: Insert codes to evaluate the expression. */
    // TODO();
    ans = eval(0, nr_token - 1);
    printf("\ncalculate answer is %u\n\n", ans);

    return ans;
}

void test_expr()
{

    bool su2 = true;
    bool su3 = true;
    bool su4 = true;
    bool su5 = true;
    bool su6 = true;
    bool su7 = true;
    bool su8 = true;

    char *expr2 = "100/2+(10-3)*2";
    char *expr3 = "(1*(2*2 + 3) - 2*1 )*2";
    char *expr4 = "2*8 + 1 + 27 +27 /9 *3";
    char *expr5 = "1<= 2 + 12+ (2*7)";
    char *expr6 = "7 != 2 + 23 >= 2";
    char *expr7 = "0xA * 2 + 1";
    char *expr8 = "0xA * 2 + (-10)";

    word_t res2 = 0;
    word_t res3 = 0;
    word_t res4 = 0;
    word_t res5 = 0;
    word_t res6 = 0;
    word_t res7 = 0;
    word_t res8 = 0;

    res2 = expr(expr2, &su2);
    assert(su2 != false);
    res3 = expr(expr3, &su3);
    assert(su3 != false);
    res4 = expr(expr4, &su4);
    assert(su4 != false);
    res5 = expr(expr5, &su5);
    assert(su5 != false);
    res6 = expr(expr6, &su6);
    assert(su6 != false);
    res7 = expr(expr7, &su7);
    assert(su7 != false);
    res8 = expr(expr8, &su8);
    assert(su8 != false);

    printf("test_expr: %d\n", res2);
    printf("test_expr: %d\n", res3);
    printf("test_expr: %d\n", res4);
    printf("test_expr: %d\n", res5);
    printf("test_expr: %d\n", res6);
    printf("test_expr: %d\n", res7);
    printf("test_expr: %d\n", res8);
//     std::ifstream infile("/home/wzm/Desktop/ysyx-workbench/nemu/tools/gen-expr/build
// /input");
//     if (!infile) {
//         printf("\nATTENTION: Can not open the file!\n\n\n");
//         return INT_MAX;
//     }

    return;
}

