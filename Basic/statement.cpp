/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// LET statement implementation
void LetStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    state.setValue(var, value);
}

// PRINT statement implementation
void PrintStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

// INPUT statement implementation
void InputStatement::execute(EvalState &state, Program &program) {
    std::string input;
    std::cout << " ? ";
    std::cout.flush();
    if (!std::getline(std::cin, input)) {
        error("EOF encountered");
    }
    // Parse integer from input
    try {
        int value = std::stoi(input);
        state.setValue(var, value);
    } catch (const std::invalid_argument&) {
        error("INVALID NUMBER");
    } catch (const std::out_of_range&) {
        error("INVALID NUMBER");
    }
}

// END statement implementation
void EndStatement::execute(EvalState &state, Program &program) {
    // For program mode, END should stop execution
    // We need a way to signal end of program
    throw ErrorException("END");
}

// GOTO statement implementation
void GotoStatement::execute(EvalState &state, Program &program) {
    program.setCurrentLine(lineNumber);
    throw ErrorException("GOTO");
}

// IF statement implementation
void IfStatement::execute(EvalState &state, Program &program) {
    int val1 = exp1->eval(state);
    int val2 = exp2->eval(state);
    bool condition = false;

    if (op == "=") {
        condition = (val1 == val2);
    } else if (op == "<") {
        condition = (val1 < val2);
    } else if (op == ">") {
        condition = (val1 > val2);
    } else {
        error("Invalid comparison operator");
    }

    if (condition) {
        program.setCurrentLine(lineNumber);
        throw ErrorException("IF-THEN");
    }
}
