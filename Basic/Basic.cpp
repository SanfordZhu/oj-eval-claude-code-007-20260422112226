/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <set>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);
bool isKeyword(const std::string& token);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) {
        return; // Empty line
    }

    std::string firstToken = scanner.nextToken();

    // Check if it's a line number (program line)
    if (scanner.getTokenType(firstToken) == NUMBER) {
        int lineNumber = stringToInteger(firstToken);

        if (!scanner.hasMoreTokens()) {
            // Empty line after number - delete the line
            program.removeSourceLine(lineNumber);
            return;
        }

        // Store the source line
        program.addSourceLine(lineNumber, line);

        // Parse and store the statement
        std::string command = scanner.nextToken();
        Statement* stmt = nullptr;

        if (command == "REM") {
            stmt = new RemStatement();
        } else if (command == "LET") {
            std::string var = scanner.nextToken();
            if (isKeyword(var)) {
                error("SYNTAX ERROR");
            }
            if (scanner.nextToken() != "=") {
                error("SYNTAX ERROR");
            }
            Expression* exp = readE(scanner);
            if (scanner.hasMoreTokens()) {
                delete exp;
                error("SYNTAX ERROR");
            }
            stmt = new LetStatement(var, exp);
        } else if (command == "PRINT") {
            Expression* exp = readE(scanner);
            if (scanner.hasMoreTokens()) {
                delete exp;
                error("SYNTAX ERROR");
            }
            stmt = new PrintStatement(exp);
        } else if (command == "INPUT") {
            std::string var = scanner.nextToken();
            if (isKeyword(var)) {
                error("SYNTAX ERROR");
            }
            stmt = new InputStatement(var);
        } else if (command == "END") {
            stmt = new EndStatement();
        } else if (command == "GOTO") {
            std::string lineStr = scanner.nextToken();
            if (scanner.getTokenType(lineStr) != NUMBER) {
                error("SYNTAX ERROR");
            }
            int targetLine = stringToInteger(lineStr);
            stmt = new GotoStatement(targetLine);
        } else if (command == "IF") {
            // Parse IF-THEN statement
            // Need to parse expressions before comparison operator
            // Read tokens until we find a comparison operator
            std::vector<std::string> leftTokens;
            std::string token;
            while (true) {
                token = scanner.nextToken();
                if (token == "=" || token == "<" || token == ">") {
                    break;
                }
                leftTokens.push_back(token);
            }
            std::string op = token;

            // Now parse right expression until THEN
            std::vector<std::string> rightTokens;
            while (true) {
                token = scanner.nextToken();
                if (token == "THEN") {
                    break;
                }
                rightTokens.push_back(token);
            }

            // Parse line number
            std::string lineStr = scanner.nextToken();
            if (scanner.getTokenType(lineStr) != NUMBER) {
                error("SYNTAX ERROR");
            }
            int targetLine = stringToInteger(lineStr);

            // Parse left expression
            std::string leftExprStr;
            for (const auto& t : leftTokens) {
                if (!leftExprStr.empty()) leftExprStr += " ";
                leftExprStr += t;
            }
            TokenScanner leftScanner;
            leftScanner.ignoreWhitespace();
            leftScanner.scanNumbers();
            leftScanner.setInput(leftExprStr);
            Expression* exp1 = readE(leftScanner);

            // Parse right expression
            std::string rightExprStr;
            for (const auto& t : rightTokens) {
                if (!rightExprStr.empty()) rightExprStr += " ";
                rightExprStr += t;
            }
            TokenScanner rightScanner;
            rightScanner.ignoreWhitespace();
            rightScanner.scanNumbers();
            rightScanner.setInput(rightExprStr);
            Expression* exp2 = readE(rightScanner);

            stmt = new IfStatement(exp1, exp2, op, targetLine);
        } else {
            error("SYNTAX ERROR");
        }

        program.setParsedStatement(lineNumber, stmt);

    } else {
        // Immediate command
        std::string command = firstToken;

        if (command == "RUN") {
            program.runProgram(state);
        } else if (command == "LIST") {
            program.listProgram(std::cout);
        } else if (command == "CLEAR") {
            program.clear();
            state.Clear();
        } else if (command == "QUIT") {
            exit(0);
        } else if (command == "HELP") {
            std::cout << "BASIC Interpreter Commands:" << std::endl;
            std::cout << "  RUN - Execute the program" << std::endl;
            std::cout << "  LIST - List program lines" << std::endl;
            std::cout << "  CLEAR - Clear program and variables" << std::endl;
            std::cout << "  QUIT - Exit interpreter" << std::endl;
            std::cout << "  HELP - Show this help" << std::endl;
        } else if (command == "LET") {
            // Immediate LET
            std::string var = scanner.nextToken();
            if (isKeyword(var)) {
                error("SYNTAX ERROR");
            }
            if (scanner.nextToken() != "=") {
                error("SYNTAX ERROR");
            }
            Expression* exp = readE(scanner);
            if (scanner.hasMoreTokens()) {
                delete exp;
                error("SYNTAX ERROR");
            }
            int value = exp->eval(state);
            state.setValue(var, value);
            delete exp;
        } else if (command == "PRINT") {
            // Immediate PRINT
            Expression* exp = readE(scanner);
            if (scanner.hasMoreTokens()) {
                delete exp;
                error("SYNTAX ERROR");
            }
            int value = exp->eval(state);
            std::cout << value << std::endl;
            delete exp;
        } else if (command == "INPUT") {
            // Immediate INPUT
            std::string var = scanner.nextToken();
            if (isKeyword(var)) {
                error("SYNTAX ERROR");
            }
            InputStatement stmt(var);
            stmt.execute(state, program);
        } else {
            error("SYNTAX ERROR");
        }
    }
}

bool isKeyword(const std::string& token) {
    static const std::set<std::string> keywords = {
        "REM", "LET", "PRINT", "INPUT", "END", "GOTO",
        "IF", "THEN", "RUN", "LIST", "CLEAR", "QUIT", "HELP"
    };
    return keywords.find(token) != keywords.end();
}

