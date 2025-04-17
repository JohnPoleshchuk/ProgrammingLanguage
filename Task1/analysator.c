#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int numerator;
    int denominator;
} Fraction;

typedef struct {
    char **tokens;
    int count;
    int pos;
} Tokenizer;

// Функции для работы с дробями
Fraction add_fractions(Fraction a, Fraction b) {
    Fraction result;
    result.numerator = a.numerator * b.denominator + b.numerator * a.denominator;
    result.denominator = a.denominator * b.denominator;
    return result;
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void simplify_fraction(Fraction *f) {
    int common_divisor = gcd(abs(f->numerator), abs(f->denominator));
    f->numerator /= common_divisor;
    f->denominator /= common_divisor;
    
    if (f->denominator < 0) {
        f->numerator *= -1;
        f->denominator *= -1;
    }
}

// Функции для токенизации
Tokenizer tokenize(const char *input) {
    Tokenizer tokenizer;
    tokenizer.count = 0;
    tokenizer.pos = 0;
    
    // Подсчет количества токенов
    const char *p = input;
    while (*p) {
        while (isspace(*p)) p++;
        if (!*p) break;
        
        if (*p == '+' || *p == '/') {
            tokenizer.count++;
            p++;
        } else if (isdigit(*p) || (*p == '-' && isdigit(*(p+1)))) {
            tokenizer.count++;
            if (*p == '-') p++;
            while (isdigit(*p)) p++;
        } else {
            fprintf(stderr, "Неизвестный символ: %c\n", *p);
            exit(1);
        }
    }
    
    // Выделение памяти и заполнение токенов
    tokenizer.tokens = (char**)malloc(tokenizer.count * sizeof(char*));
    p = input;
    int i = 0;
    while (*p) {
        while (isspace(*p)) p++;
        if (!*p) break;
        
        const char *start = p;
        if (*p == '+' || *p == '/') {
            p++;
        } else if (isdigit(*p) || (*p == '-' && isdigit(*(p+1)))) {
            if (*p == '-') p++;
            while (isdigit(*p)) p++;
        }
        
        int len = p - start;
        tokenizer.tokens[i] = (char*)malloc(len + 1);
        strncpy(tokenizer.tokens[i], start, len);
        tokenizer.tokens[i][len] = '\0';
        i++;
    }
    
    return tokenizer;
}

void free_tokenizer(Tokenizer *tokenizer) {
    for (int i = 0; i < tokenizer->count; i++) {
        free(tokenizer->tokens[i]);
    }
    free(tokenizer->tokens);
}

// Функции парсера
char* current_token(Tokenizer *tokenizer) {
    if (tokenizer->pos < tokenizer->count) {
        return tokenizer->tokens[tokenizer->pos];
    }
    return NULL;
}

void consume(Tokenizer *tokenizer, const char *expected) {
    char *token = current_token(tokenizer);
    if (token && strcmp(token, expected) == 0) {
        tokenizer->pos++;
    } else {
        fprintf(stderr, "Ошибка: ожидалось '%s', получено '%s'\n", expected, token ? token : "NULL");
        exit(1);
    }
}

Fraction parse_number(Tokenizer *tokenizer) {
    char *token = current_token(tokenizer);
    if (token && (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1])))) {
        tokenizer->pos++;
        return (Fraction){atoi(token), 1};
    }
    fprintf(stderr, "Ошибка: ожидалось число, получено '%s'\n", token ? token : "NULL");
    exit(1);
}

Fraction parse_T(Tokenizer *tokenizer) {
    Fraction numerator = parse_number(tokenizer);
    consume(tokenizer, "/");
    Fraction denominator = parse_number(tokenizer);
    
    if (denominator.numerator == 0) {
        fprintf(stderr, "Ошибка: деление на ноль\n");
        exit(1);
    }
    
    return (Fraction){numerator.numerator, denominator.numerator};
}

Fraction parse_E(Tokenizer *tokenizer) {
    Fraction result = parse_T(tokenizer);
    
    while (current_token(tokenizer) && strcmp(current_token(tokenizer), "+") == 0) {
        consume(tokenizer, "+");
        Fraction term = parse_T(tokenizer);
        result = add_fractions(result, term);
    }
    
    return result;
}

Fraction parse_S(Tokenizer *tokenizer) {
    return parse_E(tokenizer);
}

int main() {
    char input[100];
    printf("Введите выражение (например: 1/2 + 3/4 + 5/6): ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0'; // Удаляем символ новой строки
    
    Tokenizer tokenizer = tokenize(input);
    Fraction result = parse_S(&tokenizer);
    simplify_fraction(&result);
    
    printf("Результат: %d/%d\n", result.numerator, result.denominator);
    
    free_tokenizer(&tokenizer);
    return 0;
}
