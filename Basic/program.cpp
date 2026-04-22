/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"



Program::Program() {
    // Initialize empty program
    currentLine = -1;
}

Program::~Program() {
    clear();
}

void Program::clear() {
    // Delete all parsed statements
    for (auto& pair : parsedStatements) {
        delete pair.second;
    }
    sourceLines.clear();
    parsedStatements.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    // If line already exists, delete its parsed statement first
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
        parsedStatements.erase(lineNumber);
    }
    sourceLines[lineNumber] = line;
}

void Program::removeSourceLine(int lineNumber) {
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
        parsedStatements.erase(lineNumber);
    }
    sourceLines.erase(lineNumber);
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        return it->second;
    }
    return "";
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (sourceLines.find(lineNumber) == sourceLines.end()) {
        error("Line number " + std::to_string(lineNumber) + " not found");
    }
    // Delete existing statement if any
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
    }
    parsedStatements[lineNumber] = stmt;
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
   auto it = parsedStatements.find(lineNumber);
   if (it != parsedStatements.end()) {
       return it->second;
   }
   return nullptr;
}

int Program::getFirstLineNumber() {
    if (sourceLines.empty()) {
        return -1;
    }
    return sourceLines.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines.upper_bound(lineNumber);
    if (it != sourceLines.end()) {
        return it->first;
    }
    return -1;
}

// Additional methods for BASIC interpreter
void Program::listProgram(std::ostream& os) const {
    for (const auto& pair : sourceLines) {
        os << pair.second << std::endl;
    }
}

void Program::runProgram(EvalState& state) {
    currentLine = getFirstLineNumber();
    while (currentLine != -1) {
        Statement* stmt = getParsedStatement(currentLine);
        if (stmt) {
            try {
                stmt->execute(state, *this);
                // If no jump occurred, go to next line
                currentLine = getNextLineNumber(currentLine);
            } catch (ErrorException& ex) {
                std::string msg = ex.getMessage();
                if (msg == "END") {
                    break; // End program execution
                } else if (msg == "GOTO" || msg == "IF-THEN") {
                    // Jump occurred, currentLine already set
                    continue;
                } else {
                    throw; // Re-throw other errors
                }
            }
        } else {
            // No parsed statement for this line
            currentLine = getNextLineNumber(currentLine);
        }
    }
    currentLine = -1; // Reset after program finishes
}

bool Program::hasLine(int lineNumber) const {
    return sourceLines.find(lineNumber) != sourceLines.end();
}

void Program::setCurrentLine(int lineNumber) {
    if (!hasLine(lineNumber)) {
        error("LINE NUMBER ERROR");
    }
    currentLine = lineNumber;
}

int Program::getCurrentLine() const {
    return currentLine;
}


