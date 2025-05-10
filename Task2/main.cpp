#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <fstream>
#include <streambuf>

using namespace std;

enum class TokenType {
    AND, BREAK, DO, ELSE, ELSEIF, END, FALSE, FOR, FUNCTION,
    GOTO, IF, IN, LOCAL, NIL, NOT, OR, REPEAT, RETURN,
    THEN, TRUE, UNTIL, WHILE,
    IDENTIFIER, NUMBER, STRING,
    PLUS, MINUS, MUL, DIV, MOD, POW, LEN,
    EQ, NEQ, LTE, GTE, LT, GT, ASSIGN,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMI, COLON, COMMA, CONCAT, DOTS,
    EOF_TOKEN, UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
};

class Lexer {
    string input;
    size_t pos = 0;
    int line = 1;
    int column = 1;
    unordered_map<string, TokenType> keywords = {
        {"and", TokenType::AND}, {"break", TokenType::BREAK},
        {"do", TokenType::DO}, {"else", TokenType::ELSE},
        {"elseif", TokenType::ELSEIF}, {"end", TokenType::END},
        {"false", TokenType::FALSE}, {"for", TokenType::FOR},
        {"function", TokenType::FUNCTION}, {"goto", TokenType::GOTO},
        {"if", TokenType::IF}, {"in", TokenType::IN}, {"local", TokenType::LOCAL},
        {"nil", TokenType::NIL}, {"not", TokenType::NOT}, {"or", TokenType::OR},
        {"repeat", TokenType::REPEAT}, {"return", TokenType::RETURN},
        {"then", TokenType::THEN}, {"true", TokenType::TRUE},
        {"until", TokenType::UNTIL}, {"while", TokenType::WHILE}
    };

    char current() { return (pos < input.size()) ? input[pos] : '\0'; }
    
    char advance() { 
        char c = current();
        if (c == '\n') {
            line++;
            column = 0;
        }
        pos++;
        column++;
        return c;
    }

    void skipWhitespace() {
        while (isspace(current())) {
            advance();
        }
    }

    string readNumber() {
        string num;
        while (isdigit(current()) || current() == '.' || 
               tolower(current()) == 'e' || current() == '+' || current() == '-') {
            num += advance();
        }
        return num;
    }

    string readString(char delim) {
        string str;
        advance();
        while (current() != delim && current() != '\0') {
            if (current() == '\\') advance();
            str += advance();
        }
        advance();
        return str;
    }

    string readIdentifier() {
        string id;
        while (isalnum(current()) || current() == '_') {
            id += advance();
        }
        return id;
    }

public:
    Lexer(const string& input) : input(input) {}

    Token nextToken() {
        skipWhitespace();

        if (current() == '\0') return {TokenType::EOF_TOKEN, "", line, column};

        char c = current();

        if (c == '"' || c == '\'') {
            string value = readString(c);
            return {TokenType::STRING, value, line, column};
        }

        if (isdigit(c)) {
            string value = readNumber();
            return {TokenType::NUMBER, value, line, column};
        }

        if (isalpha(c) || c == '_') {
            string id = readIdentifier();
            return {keywords.count(id) ? keywords[id] : TokenType::IDENTIFIER, id, line, column};
        }

        switch(c) {
            case '+': advance(); return {TokenType::PLUS, "+", line, column};
            case '-': advance(); return {TokenType::MINUS, "-", line, column};
            case '*': advance(); return {TokenType::MUL, "*", line, column};
            case '/': advance(); return {TokenType::DIV, "/", line, column};
            case '%': advance(); return {TokenType::MOD, "%", line, column};
            case '^': advance(); return {TokenType::POW, "^", line, column};
            case '#': advance(); return {TokenType::LEN, "#", line, column};
            case '=': {
                advance();
                if (current() == '=') {
                    advance();
                    return {TokenType::EQ, "==", line, column};
                }
                return {TokenType::ASSIGN, "=", line, column};
            }
            case '<': {
                advance();
                if (current() == '=') {
                    advance();
                    return {TokenType::LTE, "<=", line, column};
                }
                return {TokenType::LT, "<", line, column};
            }
            case '>': {
                advance();
                if (current() == '=') {
                    advance();
                    return {TokenType::GTE, ">=", line, column};
                }
                return {TokenType::GT, ">", line, column};
            }
            case '~': {
                advance();
                if (current() == '=') {
                    advance();
                    return {TokenType::NEQ, "~=", line, column};
                }
                break;
            }
            case '.': {
                advance();
                if (current() == '.') {
                    advance();
                    if (current() == '.') {
                        advance();
                        return {TokenType::DOTS, "...", line, column};
                    }
                    return {TokenType::CONCAT, "..", line, column};
                }
                return {TokenType::UNKNOWN, ".", line, column};
            }
            case '(': advance(); return {TokenType::LPAREN, "(", line, column};
            case ')': advance(); return {TokenType::RPAREN, ")", line, column};
            case '{': advance(); return {TokenType::LBRACE, "{", line, column};
            case '}': advance(); return {TokenType::RBRACE, "}", line, column};
            case '[': advance(); return {TokenType::LBRACKET, "[", line, column};
            case ']': advance(); return {TokenType::RBRACKET, "]", line, column};
            case ';': advance(); return {TokenType::SEMI, ";", line, column};
            case ':': advance(); return {TokenType::COLON, ":", line, column};
            case ',': advance(); return {TokenType::COMMA, ",", line, column};
        }

        advance();
        return {TokenType::UNKNOWN, string(1, c), line, column};
    }
};

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Cannot open file " << filename << endl;
        exit(1);
    }
    return string((istreambuf_iterator<char>(file)), 
            istreambuf_iterator<char>());
}

int main() {
    string code = readFile("init.lua");
    Lexer lexer(code);
    
    while (true) {
        Token token = lexer.nextToken();
        if (token.type == TokenType::EOF_TOKEN) break;
        
        cout << "Line " << token.line << ":" << token.column
             << " \tType: " << static_cast<int>(token.type)
             << " \tValue: " << token.value << endl;
    }
    
    return 0;
}
