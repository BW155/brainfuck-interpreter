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

enum CommandType {
    POINTER_INC,
    POINTER_DEC,
    VALUE_DEC,
    VALUE_INC,
    BRACK_LEFT,
    BRACK_RIGHT,
    CLEAR,
    OUT,
    IN,
};

struct Command {
    CommandType command_type;
    unsigned int duplicates;
};

void compile(std::string code, Command (&result)[12000]) {
    std::vector<char> temp_chars = {};
    int cur_command_row = -1;
    for(int p = 0; p < code.size(); p++) {
        if (cur_command_row == -1) {
            cur_command_row = p;
        }
        switch (code[p]) {
            case BF_OP_PINC:
                temp_chars.push_back(BF_OP_PINC);
                if (code[p + 1] != BF_OP_PINC) {
                    result[cur_command_row] = Command {
                        CommandType::POINTER_INC,
                        static_cast<unsigned int>(temp_chars.size())
                    };
                    cur_command_row = -1;
                    temp_chars.clear();
                }
                break;
            case BF_OP_PDEC:
                temp_chars.push_back(BF_OP_PDEC);
                if (code[p + 1] != BF_OP_PDEC) {
                    result[cur_command_row] = Command {
                        CommandType::POINTER_DEC,
                        static_cast<unsigned int>(temp_chars.size())
                    };
                    cur_command_row = -1;
                    temp_chars.clear();
                }
                break;
            case BF_OP_VINC:
                temp_chars.push_back(BF_OP_VINC);
                if (code[p + 1] != BF_OP_VINC) {
                    result[cur_command_row] = Command {
                        CommandType::VALUE_INC,
                        static_cast<unsigned int>(temp_chars.size())
                    };
                    cur_command_row = -1;
                    temp_chars.clear();
                }
                break;
            case BF_OP_VDEC:
                temp_chars.push_back(BF_OP_VDEC);
                if (code[p + 1] != BF_OP_VDEC) {
                    result[cur_command_row] = Command {
                        CommandType::VALUE_DEC,
                        static_cast<unsigned int>(temp_chars.size())
                    };
                    cur_command_row = -1;
                    temp_chars.clear();
                }
                break;
            case BF_OP_LSTART:
                /*if (code[p + 1] == '-' && code[p + 2] == ']') {
                    result[p] = Command {
                        CommandType::CLEAR,
                        3
                    };
                    cur_command_row = -1;
                    temp_chars.clear();
                    p += 2;
                    break;
                }*/
                result[p] = Command {
                    CommandType::BRACK_LEFT,
                    1
                };
                cur_command_row = -1;
                break;
            case BF_OP_LEND:
                result[p] = Command {
                    CommandType::BRACK_RIGHT,
                    1
                };
                cur_command_row = -1;
                break;
            case BF_OP_OUT:
                result[p] = Command {
                    CommandType::OUT,
                    1
                };
                cur_command_row = -1;
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string code = get_file_contents(argv[1]);
    code.erase(std::remove(code.begin(), code.end(), '\n'), code.end());
    std::map<unsigned int, unsigned int> bracket_index = index_brackets(code);

    std::vector<int> cells = {0};
    unsigned int cell_pointer = 0;
    unsigned int code_pointer = 0;

    auto code_size = (int) code.size();

    Command compiled[12000] = {};
    compile(code, compiled);

    while (code_pointer < code_size) {
        int duplicates = compiled[code_pointer].duplicates;

        switch (compiled[code_pointer].command_type) {
            case CommandType::POINTER_INC:
                cell_pointer += duplicates;
                while (cell_pointer >= cells.size()) {
                    cells.push_back(0);
                }
                break;
            case CommandType::POINTER_DEC:
                cell_pointer -= duplicates;
                break;
            case CommandType::VALUE_INC:
                cells[cell_pointer] += duplicates;
                if (cells[cell_pointer] > 255) {
                    cells[cell_pointer] -= 255;
                }
                break;
            case CommandType::VALUE_DEC:
                cells[cell_pointer] -= duplicates;
                if (cells[cell_pointer] < 0) {
                    cells[cell_pointer] += 255;
                }
                break;
            case CommandType::OUT:
                std::printf("%c", cells[cell_pointer]);
                break;
            case CommandType::BRACK_LEFT:
                if (cells[cell_pointer] == 0) {
                    code_pointer = bracket_index[code_pointer];
                }
                break;
            case CommandType::BRACK_RIGHT:
                if (cells[cell_pointer] != 0) {
                    code_pointer = bracket_index[code_pointer];
                }
                break;
            case CommandType::CLEAR:
                cells[code_pointer] = 0;
        }
        code_pointer += duplicates;
    }
    return 0;
}
