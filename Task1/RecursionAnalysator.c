#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    const char *input;
    int pos;
    bool error;
    char error_msg[256];
    long long sum_num;
    long long sum_den;
} Parser;

void skip_whitespace(Parser *p) {
    while (p->input[p->pos] != '\0' && isspace(p->input[p->pos])) {
        p->pos++;
    }
}

bool expect(Parser *p, char expected) {
    skip_whitespace(p);
    if (p->input[p->pos] != expected) {
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Ошибка: Ожидалось '%c', но найдено '%c' (позиция %d)",
                 expected, p->input[p->pos], p->pos);
        p->error = true;
        return false;
    }
    p->pos++;
    return true;
}

bool parse_natural(Parser *p, int *result) {
    skip_whitespace(p);
    if (!isdigit(p->input[p->pos])) {
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Ошибка: Ожидалось натуральное число (позиция %d)", p->pos);
        p->error = true;
        return false;
    }
    int start_pos = p->pos;
    while (isdigit(p->input[p->pos])) {
        p->pos++;
    }

    if (p->pos - start_pos > 1 && p->input[start_pos] == '0') {
        snprintf(p->error_msg, sizeof(p->error_msg),
                "Ошибка: Натуральное число не может начинаться с '0' (позиция %d)", start_pos);
        p->error = true;
        return false;
    }

    char num_str[20];
    int len = p->pos - start_pos;
    if (len >= (int)sizeof(num_str)) {
        snprintf(p->error_msg, sizeof(p->error_msg),
                "Ошибка: Число слишком длинное (позиция %d)", start_pos);
        p->error = true;
        return false;
    }
    strncpy(num_str, p->input + start_pos, len);
    num_str[len] = '\0';

    long long num = atoll(num_str);
    if (num > INT_MAX) {
        snprintf(p->error_msg, sizeof(p->error_msg),
                "Ошибка: Число превышает максимальное значение int (позиция %d)", start_pos);
        p->error = true;
        return false;
    }

    *result = (int)num;
    return true;
}

long long compute_gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

bool parse_fraction(Parser *p, int *a, int *b) {
    if (!expect(p, '1')) return false;
    if (!expect(p, '/')) return false;
    if (!expect(p, '(')) return false;
    if (!parse_natural(p, a)) return false;
    if (!expect(p, '*')) return false;
    if (!parse_natural(p, b)) return false;
    if (!expect(p, ')')) return false;

    long long product = (long long)(*a) * (*b);
    if (product > INT_MAX) {
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Ошибка: Произведение %d*%d превышает максимальное значение int (позиция %d)",
                 *a, *b, p->pos);
        p->error = true;
        return false;
    }

    return true;
}

bool parse_series(Parser *p) {
    p->sum_num = 1;
    p->sum_den = 1;

    if (!expect(p, '1')) return false;

    skip_whitespace(p);

    while (p->input[p->pos] == '+') {
        p->pos++;
        int a, b;
        if (!parse_fraction(p, &a, &b)) return false;
        long long d = (long long)a * b;

        // Вычисление нового числителя и знаменателя
        long long new_num = p->sum_num * d + p->sum_den;
        long long new_den = p->sum_den * d;

        // Проверка переполнения
        if ((d > 0 && p->sum_den > LLONG_MAX / d) ||
            (p->sum_num > 0 && d > LLONG_MAX / p->sum_num)) {
            snprintf(p->error_msg, sizeof(p->error_msg),
                     "Ошибка: Переполнение при вычислении суммы (позиция %d)", p->pos);
            p->error = true;
            return false;
        }

        // Сокращение дроби
        long long gcd = compute_gcd(new_num, new_den);
        p->sum_num = new_num / gcd;
        p->sum_den = new_den / gcd;

        skip_whitespace(p);
    }

    if (p->input[p->pos] != '\0') {
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Ошибка: Неожиданный символ '%c' (позиция %d)",
                 p->input[p->pos], p->pos);
        p->error = true;
        return false;
    }
    return true;
}

void parse(const char *input) {
    Parser p = {input, 0, false, "", 0, 0};
    if (parse_series(&p)) {
        printf("✅ Корректный ряд! Сумма: %lld/%lld\n", p.sum_num, p.sum_den);
    } else {
        printf("❌ %s\n", p.error_msg);
    }
}

int main() {
    const char *valid_series = "1 + 1/(2*3) + 1/(10*5)";
    const char *invalid_series = "1 + 1/(10*5) + 1/(2*579465612786526758178457124675824781)";
    const char *second_valid_series = "1 + 1/(2*3) + 1/(12321*6526) + 1/(54*123) + 1/(1*1)";

    printf("\nПроверка корректного ряда:\n");
    parse(valid_series);

    printf("\nПроверка некорректного ряда:\n");
    parse(invalid_series);

    printf("\nПроверка второго корректного ряда:\n");
    parse(second_valid_series);

    return 0;
}
