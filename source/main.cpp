#include "lexer.h"

int main(int argc, char *argv[]) {
    Lexer lexer;

    std::string output;
    switch (argc){
        case 2:
            output = argv[1];
            output += ".satin";
            lexer.build(argv[1], output.c_str());
            return 0;
        case 3:
            lexer.build(argv[1], argv[2]);
            return 0;
        default:
            std::cout << "INVALID ARGUMENT COUNT" << std::endl;
            return -1;
    }
}