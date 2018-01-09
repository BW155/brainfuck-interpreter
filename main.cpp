#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <deque>

#define BF_OP_VINC      '+'
#define BF_OP_VDEC      '-'
#define BF_OP_PINC      '>'
#define BF_OP_PDEC      '<'
#define BF_OP_LSTART    '['
#define BF_OP_LEND      ']'
#define BF_OP_IN        ','
#define BF_OP_OUT       '.'

std::string get_file_contents(const char *filename) {
    std::FILE *fp = std::fopen(filename, "r");
    if (fp) {
        std::string contents;
        std::fseek(fp, 0, SEEK_END);
        contents.resize(std::ftell(fp));
        std::rewind(fp);
        std::fread(&contents[0], 1, contents.size(), fp);
        std::fclose(fp);
        return(contents);
    }
    throw(errno);
}

std::map<unsigned int, unsigned int> index_brackets(std::string code) {
    std::vector<unsigned int> temp_brackets = {};
    std::map<unsigned int, unsigned int> bracket_index = std::map<unsigned int, unsigned int>();

    for (unsigned int i = 0; i < code.size(); i++) {
        char command = code[i];
        switch(command) {
            case '[' :
                temp_brackets.push_back(i);
                break;
            case ']':
                auto start = temp_brackets[temp_brackets.size() - 1];
                temp_brackets.pop_back();
                bracket_index[start] = i;
                bracket_index[i] = start;
        }
    }
    return bracket_index;
}

std::deque<unsigned int> index_clears(std::string code) {
    std::deque<unsigned int> clears = {};

    for (unsigned int i = 0; i < code.size(); i++) {
        if (code[i] == '[') {
            if (code[i + 1] == '-') {
                if (code[i + 2] == ']') {
                    clears.push_back(i);
                }
            }
        }
    }
    return clears;
}

int main(int argc, char* argv[]) {
    std::string code = get_file_contents(argv[1]);
    code.erase(std::remove(code.begin(), code.end(), '\n'), code.end());
    std::map<unsigned int, unsigned int> bracket_index = index_brackets(code);

    std::vector<int> cells = {0};
    unsigned int cell_pointer = 0;
    unsigned int code_pointer = 0;

    auto code_size = (int) code.size();

    while (code_pointer < code_size) {
        int duplicates = 1;
        switch (code[code_pointer]) {
            case BF_OP_PINC :
                while (code[code_pointer + duplicates] == '>') {duplicates++;}
                cell_pointer += duplicates;
                while (cell_pointer >= cells.size()) {
                    cells.push_back(0);
                }
                break;
            case BF_OP_PDEC :
                while (code[code_pointer + duplicates] == '<') {duplicates++;}
                cell_pointer -= duplicates;
                break;
            case BF_OP_VINC :
                while (code[code_pointer + duplicates] == '+') {duplicates++;}
                cells[cell_pointer] += duplicates;
                if (cells[cell_pointer] > 255) {
                    cells[cell_pointer] -= 255;
                }
                break;
            case BF_OP_VDEC :
                while (code[code_pointer + duplicates] == '-') {duplicates++;}
                cells[cell_pointer] -= duplicates;
                if (cells[cell_pointer] < 0) {
                    cells[cell_pointer] += 255;
                }
                break;
            case BF_OP_OUT :
                std::printf("%c", cells[cell_pointer]);
                break;/*
            case BF_OP_IN :
                cells[cell_pointer] = getchar();
                break;*/
            case BF_OP_LSTART :
                if (code[code_pointer + 1] == BF_OP_VDEC && code[code_pointer + 2] == BF_OP_LEND) {
                    cells[cell_pointer] = 0;
                    duplicates = 3;
                    break;
                }
                if (cells[cell_pointer] == 0) {
                    code_pointer = bracket_index[code_pointer];
                }
                break;
            case BF_OP_LEND :
                if (cells[cell_pointer] != 0) {
                    code_pointer = bracket_index[code_pointer];
                }
        }
        code_pointer += duplicates;
    }
    return 0;
}
