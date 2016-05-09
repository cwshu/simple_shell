#include <string>
#include <vector>

const char WHITESPACE[] = " \t\r\n\v\f";
const char REDIRECTION_CHARS[] = "|><";
const char FILE_REDIR_CHARS[] = "><";
const char PIPE_REDIR_CHAR[] = "|";

const char FILEIN_REDIR = '<';
const char FILEOUT_REDIR = '>';

enum RedirectionType{
    REDIR_NONE,
    REDIR_FILE,
    REDIR_PIPE,
};

struct Redirection{
    RedirectionType type;

    std::string filename;

    Redirection(RedirectionType type = REDIR_NONE);
};

struct SingleCommand{
    std::string executable;
    std::vector<std::string> arguments;
    int args_count;

    Redirection std_input;
    Redirection std_output;
    Redirection std_error;

    SingleCommand();
    void add_executable(std::string executable);
    void add_argv(std::string argument);
    void add_redir(int fileno, Redirection redir);

// DEBUG
    void print() const;
};

struct OneLineCommand{
    std::vector<SingleCommand> cmds;
    int cmd_count;

    OneLineCommand();
    void add_single_command(SingleCommand cmd);
 
    // parse
    void parse_one_line_cmd(std::string& line_command);
    void parse_redir_file(SingleCommand& cmd, std::string& file_redir_command);
    int parse_single_command(SingleCommand& cmd, std::string& single_command);

    std::string fetch_word(std::string& input_string) const;
};

