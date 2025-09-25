
typedef struct token {
    enum token_type {
        TOKEN_END,
        TOKEN_NUMBER,
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_STAR,
        TOKEN_SLASH,
        TOKEN_OPAREN,
        TOKEN_CPAREN,
    } type;
    union {
        double num;
        char rune;
    };
} token_t;

bool error = false;

token_t next_token(const sb_t *expression, int *counter) {
    if ((size_t) *counter >= expression->size) return (token_t) { .type = TOKEN_END };
    printf("%d\n", *counter);
    if (isdigit(expression->data[*counter])) {
        const char *start = &expression->data[*counter];
        bool seen_dot = false;
        while (expression->size > (size_t) *counter && (isdigit(expression->data[*counter]) || expression->data[*counter] == '.')) {
            if (expression->data[*counter] == '.') {
                if (seen_dot) {
                    error = true;
                    return (token_t) { 0 };
                }
                seen_dot = true;
            }
            (*counter)++;
        }
        double num = strtod(start, NULL);
        return (token_t) { .type = TOKEN_NUMBER, .num = num };
    } else {
        switch (expression->data[(*counter)++]) {
        case '+': return (token_t) { .type = TOKEN_PLUS, .rune = '+' };
        case '-': puts("that would be really strange"); return (token_t) { .type = TOKEN_MINUS, .rune = '-' };
        case '*': return (token_t) { .type = TOKEN_STAR, .rune = '*' };
        case '/': return (token_t) { .type = TOKEN_SLASH, .rune = '/' };
        case '(': return (token_t) { .type = TOKEN_OPAREN, .rune = '(' };
        case ')': return (token_t) { .type = TOKEN_CPAREN, .rune = ')' };
        default:
            puts("like really");
            error = true;
            return (token_t) { 0 };
        }
    }
    error = true;
    return (token_t) { 0 };
}

typedef struct {
    int counter;
    const sb_t *expression;
    token_t next;
} token_stream_t;

#define ts_peek(ts) ((ts)->next)

static inline token_t ts_pop(token_stream_t *ts) {
    token_t cur = ts->next;
    ts->next = next_token(ts->expression, &ts->counter);
    return cur;
}

double eval_addsub(token_stream_t *ts);

double eval_primary(token_stream_t *ts) {
    if (ts_peek(ts).type == TOKEN_NUMBER) {
        return ts_pop(ts).num;
    }
    if (ts_peek(ts).type == TOKEN_OPAREN) {
        ts_pop(ts);
        double num = eval_addsub(ts);
        if (ts_peek(ts).type != TOKEN_CPAREN) { error = true; return -420.69; }
        ts_pop(ts);
        return num;
    }
    if (ts_peek(ts).type == TOKEN_END) return 0.0;
    error = true;
    return -420.69;
}

double eval_negation(token_stream_t *ts) {
    if (ts_peek(ts).type == TOKEN_MINUS) {
        puts("NEGATION!");
        ts_pop(ts);
        return -eval_negation(ts);
    }
    return eval_primary(ts);
}

double eval_muldiv(token_stream_t *ts) {
    double num = eval_negation(ts);
    while (ts_peek(ts).type == TOKEN_STAR || ts_peek(ts).type == TOKEN_SLASH) {
        token_t token = ts_pop(ts);
        double rhs = eval_negation(ts);
        if (token.type == TOKEN_STAR) num *= rhs;
        if (token.type == TOKEN_SLASH) num /= rhs;
    }
    return num;
}

double eval_addsub(token_stream_t *ts) {
    double num = eval_muldiv(ts);
    while (ts_peek(ts).type == TOKEN_PLUS || ts_peek(ts).type == TOKEN_MINUS) {
        token_t token = ts_pop(ts);
        double rhs = eval_muldiv(ts);
        if (token.type == TOKEN_PLUS) num += rhs;
        if (token.type == TOKEN_MINUS) num -= rhs;
    }
    return num;
}

double evaluate(sb_t *expression) {
    da_push(expression, 0); expression->size--;
    int counter = 0;
    token_t token = next_token(expression, &counter);
    token_stream_t ts = (token_stream_t) { .expression = expression, .next = token, .counter = counter };
    double res = eval_addsub(&ts);
    if (ts_peek(&ts).type != TOKEN_END) { error = true; return 0.0; }
    if (error) return 0.0;
    
    static char memstr[64];
    double integ, frac = modf(res, &integ);
    int size;
    if (frac < 0.000000000001) size = snprintf(memstr, 64, "%.0F", integ);
    else {
        size = snprintf(memstr, 64, "%.12F", res);
        while (size > 0 && memstr[size-1] == '0') size--;
    }
    expression->size = 0;
    for (int i = 0; i < size; i++) da_push(expression, memstr[i]);
    return res;
}
