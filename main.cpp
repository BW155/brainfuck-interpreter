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
    unsigned int pointer;
};

void compile(std::string code, Command (&result)[12000]) {
    std::vector<unsigned int> temp_brackets = {};
    std::vector<char> temp_chars = {};
    int cur_command_row = -1;
    unsigned int start;

    unsigned int p = 0;
    while (p < code.size()) {
        if (cur_command_row == -1) {
            cur_command_row = p;
        }
        switch (code[p]) {
            case BF_OP_PINC:
                temp_chars.push_back(BF_OP_PINC);
                if (code[p + 1] != BF_OP_PINC) {
                    result[cur_command_row] = Command {
                        POINTER_INC,
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
                        POINTER_DEC,
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
                        VALUE_INC,
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
                        VALUE_DEC,
                        static_cast<unsigned int>(temp_chars.size())
                    };
                    cur_command_row = -1;
                    temp_chars.clear();
                }
                break;
            case BF_OP_LSTART:
                if (code[p + 1] == '-' && code[p + 2] == ']') {
                    result[p] = Command {
                        CLEAR,
                        3
                    };
                    cur_command_row = -1;
                    p += 2;
                    break;
                }
                result[p] = Command {
                    BRACK_LEFT,
                    1
                };
                cur_command_row = -1;
                temp_brackets.push_back(p);
                break;
            case BF_OP_LEND:
                start = temp_brackets[temp_brackets.size() - 1];
                temp_brackets.pop_back();
                result[start].pointer = p;
                result[p] = Command {
                    BRACK_RIGHT,
                    1,
                    start
                };
                cur_command_row = -1;
                break;
            case BF_OP_OUT:
                result[p] = Command {
                    OUT,
                    1
                };
                cur_command_row = -1;
                break;
            case BF_OP_IN:
                result[p] = Command {
                    IN,
                    1
                };
                cur_command_row = -1;
                break;
        }
        p++;
    }
}

int main(int argc, char* argv[]) {
    std::string code = get_file_contents(argv[1]);
    code.erase(std::remove(code.begin(), code.end(), '\n'), code.end());

    std::vector<int> cells = {0};
    unsigned int cell_pointer = 0;
    unsigned int code_pointer = 0;

    auto code_size = (int) code.size();

    Command compiled[12000] = {};
    compile(code, compiled);

    while (code_pointer < code_size) {
        Command command = compiled[code_pointer];
        int duplicates = command.duplicates;

        switch (command.command_type) {
            case POINTER_INC:
                cell_pointer += duplicates;
                if (cell_pointer >= cells.size()) {
                    cells.resize(cell_pointer + 1);
                }
                break;
            case POINTER_DEC:
                cell_pointer -= duplicates;
                break;
            case VALUE_INC:
                cells[cell_pointer] += duplicates;
                if (cells[cell_pointer] > 255) {
                    cells[cell_pointer] -= 255;
                }
                break;
            case VALUE_DEC:
                cells[cell_pointer] -= duplicates;
                if (cells[cell_pointer] < 0) {
                    cells[cell_pointer] += 255;
                }
                break;
            case OUT:
                std::printf("%c", cells[cell_pointer]);
                break;
            case CommandType::BRACK_LEFT:
                if (cells[cell_pointer] == 0) {
                    code_pointer = command.pointer;
                }
                break;
            case BRACK_RIGHT:
                if (cells[cell_pointer] != 0) {
                    code_pointer = command.pointer;
                }
                break;
            case CLEAR:
                cells[cell_pointer] = 0;
                break;
            case IN:
                cells[cell_pointer] = getchar();
                break;
        }
        code_pointer += duplicates;
    }
    return 0;
}
