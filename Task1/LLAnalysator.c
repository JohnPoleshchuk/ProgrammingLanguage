#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_STACK 100

typedef enum {
    TOK_1, TOK_PLUS, TOK_SLASH, TOK_LPAREN, TOK_RPAREN, TOK_MUL, TOK_DIGIT, TOK_END, TOK_INVALID
} TokenType;

typedef struct {
    TokenType type;
    int value;
    int pos;
} Token;

typedef struct {
    const char *input;
    int pos;
    Token lookahead;
    bool has_error;
    char error_msg[256];
    long long sum_num;
    long long sum_den;
    int current_a;
    int current_b;
} Parser;

void skip_whitespace(Parser *p) {
    while (p->input[p->pos] != '\0' && isspace(p->input[p->pos])) {
        p->pos++;
    }
}

Token get_next_token(Parser *p) {
    skip_whitespace(p);
    Token tok = {TOK_INVALID, 0, p->pos};
    char c = p->input[p->pos];
    
    if (c == '\0') {
        tok.type = TOK_END;
    } else if (c == '1' && (p->input[p->pos + 1] == '/' || p->pos == 0)) {
        tok.type = TOK_1;
        p->pos++;
    } else if (c == '+') {
        tok.type = TOK_PLUS;
        p->pos++;
    } else if (c == '/') {
        tok.type = TOK_SLASH;
        p->pos++;
    } else if (c == '(') {
        tok.type = TOK_LPAREN;
        p->pos++;
    } else if (c == ')') {
        tok.type = TOK_RPAREN;
        p->pos++;
    } else if (c == '*') {
        tok.type = TOK_MUL;
        p->pos++;
    } else if (isdigit(c)) {
        tok.type = TOK_DIGIT;
        tok.value = 0;
        int start_pos = p->pos;
        while (isdigit(p->input[p->pos])) {
            int digit = p->input[p->pos] - '0';
            if (tok.value > (INT_MAX - digit) / 10) {
                sprintf(p->error_msg, "Ошибка: Число превышает INT_MAX (позиция %d)", start_pos);
                p->has_error = true;
                return tok;
            }
            tok.value = tok.value * 10 + digit;
            p->pos++;
        }
        if (p->input[start_pos] == '0' && tok.value != 0) {
            sprintf(p->error_msg, "Ошибка: Натуральное число не может начинаться с '0' (позиция %d)", start_pos);
            p->has_error = true;
        }
    } else {
        sprintf(p->error_msg, "Ошибка: Неизвестный символ '%c' (позиция %d)", c, p->pos);
        p->has_error = true;
    }
    return tok;
}

void init_parser(Parser *p, const char *input) {
    p->input = input;
    p->pos = 0;
    p->has_error = false;
    p->sum_num = 1;
    p->sum_den = 1;
    p->lookahead = get_next_token(p);
}

void advance(Parser *p) {
    if (!p->has_error && p->lookahead.type != TOK_END) {
        p->lookahead = get_next_token(p);
    }
}

const char* token_type_to_str(TokenType type) {
    switch (type) {
        case TOK_1: return "'1'";
        case TOK_PLUS: return "'+'";
        case TOK_SLASH: return "'/'";
        case TOK_LPAREN: return "'('";
        case TOK_RPAREN: return "')'";
        case TOK_MUL: return "'*'";
        case TOK_DIGIT: return "натуральное число";
        case TOK_END: return "конец строки";
        default: return "неизвестный токен";
    }
}

bool match(Parser *p, TokenType expected) {
    if (p->lookahead.type == expected) {
        advance(p);
        return true;
    }
    sprintf(p->error_msg, "Ошибка: Ожидалось %s, но найдено %s (позиция %d)", 
            token_type_to_str(expected), 
            token_type_to_str(p->lookahead.type), 
            p->lookahead.pos);
    p->has_error = true;
    return false;
}

long long compute_gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

bool parse_N(Parser *p, int *value) {
    if (p->lookahead.type != TOK_DIGIT || p->lookahead.value < 1) {
        sprintf(p->error_msg, "Ошибка: Ожидалось натуральное число ≥1 (позиция %d)", p->lookahead.pos);
        p->has_error = true;
        return false;
    }
    *value = p->lookahead.value;
    match(p, TOK_DIGIT);
    return true;
}

void parse_F(Parser *p) {
    match(p, TOK_1);
    match(p, TOK_SLASH);
    match(p, TOK_LPAREN);
    int a, b;
    if (!parse_N(p, &a)) return;
    match(p, TOK_MUL);
    if (!parse_N(p, &b)) return;
    match(p, TOK_RPAREN);
    
    long long product = (long long)a * b;
    if (product > INT_MAX) {
        sprintf(p->error_msg, "Ошибка: Произведение %d*%d превышает INT_MAX (позиция %d)", a, b, p->lookahead.pos);
        p->has_error = true;
        return;
    }
    p->current_a = a;
    p->current_b = b;
}

void parse_S_prime(Parser *p) {
    if (p->lookahead.type == TOK_PLUS) {
        match(p, TOK_PLUS);
        parse_F(p);
        if (p->has_error) return;
        
        long long a = p->current_a;
        long long b = p->current_b;
        long long den = a * b;
        long long num = 1;
        
        long long new_num = p->sum_num * den + p->sum_den * num;
        long long new_den = p->sum_den * den;
        long long gcd = compute_gcd(new_num, new_den);
        p->sum_num = new_num / gcd;
        p->sum_den = new_den / gcd;
        
        parse_S_prime(p);
    }
}

void parse_S(Parser *p) {
    if (p->lookahead.type == TOK_1) {
        p->sum_num = 1;
        p->sum_den = 1;
        match(p, TOK_1);
        if (p->lookahead.type == TOK_SLASH) {
            parse_F(p);
            if (p->has_error) return;
            long long a = p->current_a;
            long long b = p->current_b;
            p->sum_num = 1;
            p->sum_den = a * b;
        }
        parse_S_prime(p);
    } else {
        sprintf(p->error_msg, "Ошибка: Ожидалось '1' (позиция %d)", p->lookahead.pos);
        p->has_error = true;
    }
}

void parse(const char *input) {
    Parser p;
    init_parser(&p, input);
    parse_S(&p);
    
    if (!p.has_error && p.lookahead.type == TOK_END) {
        printf("✅ Корректный ряд! Сумма: %lld/%lld\n", p.sum_num, p.sum_den);
    } else {
        printf("❌ %s\n", p.error_msg);
    }
}

int main() {
    const char *valid_series = "1 + 1/(2*3) + 1/(10*5)";
    const char *invalid_series = "1 + 1/(10*5) + 1/(2*579465612786526758178457124675824781)";
    const char *second_valid_series = "1 + 1/(2*3) + 1/(12321*6526) + 1/(54*123) + 1/(1*1)";

    printf("Проверка корректного ряда:\n");
    parse(valid_series);

    printf("\nПроверка некорректного ряда:\n");
    parse(invalid_series);

    printf("\nПроверка второго корректного ряда:\n");
    parse(second_valid_series);

    return 0;
}
