#include <cstdio>

#include <unistd.h>

#include "cmd_parser.h"

// struct Redirection
Redirection::Redirection(RedirectionType type){
    this->type = type;
}

// struct SingleCommand
SingleCommand::SingleCommand(){
    args_count = 0;
}

void SingleCommand::add_executable(std::string executable){
    this->executable = executable;
    this->args_count = 1;
}

void SingleCommand::add_argv(std::string argument){
    this->arguments.push_back(argument);
    this->args_count += 1;
}

void SingleCommand::add_redir(int fileno, Redirection redir){
    if( fileno == STDIN_FILENO ){
        this->std_input = redir;
    }
    else if( fileno == STDOUT_FILENO ){
        this->std_output = redir;
    }
    else if( fileno == STDERR_FILENO ){
        this->std_error = redir;
    }
}

std::string redir_to_string(Redirection redir){
    if( redir.type == REDIR_NONE ){
        return std::string("None");
    }
    else if( redir.type == REDIR_FILE ){
        return std::string("File: ") + redir.filename;
    }
    else if( redir.type == REDIR_PIPE ){
        return std::string("Pipe");
    }
    return std::string();
}

void SingleCommand::print() const {
    printf("SingleCommand: (%s, ", executable.c_str());
    for( auto argument : arguments ){
        printf("%s, ", argument.c_str());
    }
    printf("), redir: (");

    printf("%s, ", redir_to_string(std_input).c_str());
    printf("%s, ", redir_to_string(std_output).c_str());
    printf("%s)\n", redir_to_string(std_error).c_str());
}

// struct OneLineCommand
OneLineCommand::OneLineCommand(){
    cmd_count = 0;
}

void OneLineCommand::add_single_command(SingleCommand cmd){
    cmds.push_back(cmd);
    cmd_count++;
}

std::string OneLineCommand::fetch_word(std::string& input_string) const {
    /* 
     * fetch the first "word" in input_string, and input_string is splited to (word, rest).
     * return word and assign input_string = rest.
     *
     * the "word" means non-whitespace char sequence.
     */
    std::size_t start = input_string.find_first_not_of(WHITESPACE);
    std::size_t end = input_string.find_first_of(WHITESPACE, start);
    if( start == std::string::npos ){
        /* no word */
        input_string.clear();
        return std::string();
    }

    std::string word;
    if( end == std::string::npos ){
        word = input_string.substr(start);
        input_string.clear();
    }
    else{
        word = input_string.substr(start, end - start);
        input_string.erase(0, end + 1);
    }

    return word;
}

int OneLineCommand::parse_single_command(SingleCommand& cmd, std::string& single_command){
    /* 
     * store command in SingleCommand cmd.
     * return argc.
     */
    int argc = 0;
    std::string executable = fetch_word(single_command);
    if( executable.empty() ){
        return argc;
    }

    argc++;
    cmd.add_executable(executable);

    while(1){
        std::string argument = fetch_word(single_command);
        if( argument.empty() ){
            break;
        }

        cmd.add_argv(argument);
        argc++;
    }

    return argc;
}

void OneLineCommand::parse_redir_file(SingleCommand& cmd, std::string& file_redir_command){
    /*
     * parse the file redirection in single command.
     * 
     * e.g. grep "hello" < in > out
     *   => (<, in), (>, out)   
     */

    while(1){
        // one loop parse one file redirection
        // printf("file_redir_command: %s\n", file_redir_command.c_str());

        std::size_t redir_file_index = file_redir_command.find_first_of(FILE_REDIR_CHARS);
        if( redir_file_index == std::string::npos ){ // no more redir file
            break;
        }
        char redir_file_char = file_redir_command[redir_file_index];
        file_redir_command = file_redir_command.substr(redir_file_index+1);
        
        std::string filename = fetch_word(file_redir_command);

        // construct Redirection by (redir_file_char, filename)
        Redirection redir_file(REDIR_FILE);
        redir_file.filename = filename;
        if( redir_file_char == FILEIN_REDIR ){
            cmd.add_redir(STDIN_FILENO, redir_file);
        }
        else if( redir_file_char == FILEOUT_REDIR ){
            cmd.add_redir(STDOUT_FILENO, redir_file);
        }
    }

    return;
}

void OneLineCommand::parse_one_line_cmd(std::string& line_command){
    /* 
     * parse one-line command, and store into OneLineCommand class
     * store array of commands
     * single command: (executable, arguments, stdin/stdout/stderr redirection)
     *
     * Grammer
     *   Line := CmdAndRedirFile (RedirPipe CmdAndRedirFile)*
     *   CmdAndRedirFile := Cmd RedirFile*
     *   Cmd := Exe Argument*
     */

    // Line := CmdAndRedirFile (RedirPipe CmdAndRedirFile)*
    bool prev_is_pipe = false;
    while(1){
        // Find RedirPipe
        std::size_t pipe_index = line_command.find_first_of(PIPE_REDIR_CHAR);

        std::string single_command = line_command.substr(0, pipe_index);
        if( pipe_index != std::string::npos ){
            line_command = line_command.substr(pipe_index+1); // after '|' char
        }

        // Find RedirFile
        std::size_t file_index = single_command.find_first_of(FILE_REDIR_CHARS);

        std::string file_redir_command;
        if( file_index != std::string::npos ){
            file_redir_command = single_command.substr(file_index);
        }
        single_command = single_command.substr(0, file_index);

        // parse Cmd and RedirFile
        SingleCommand cmd;
        int argc = parse_single_command(cmd, single_command);
        if( argc == 0 ){
            break;
        }
        parse_redir_file(cmd, file_redir_command);

        // set pipe
        if( prev_is_pipe ){
            cmd.add_redir(STDIN_FILENO, Redirection(REDIR_PIPE));
        }
        if( pipe_index != std::string::npos ){
            // set command stdout redirect to pipe
            cmd.add_redir(STDOUT_FILENO, Redirection(REDIR_PIPE));
        }

        this->add_single_command(cmd);

        if( pipe_index == std::string::npos ){
            // no next pipe => no next Cmd
            break;
        }
        prev_is_pipe = true;
    }
}

#ifdef __LIB_TEST__

int main(){
    OneLineCommand cmd[10];    
    std::string cmd_string[10] = {
        "ls",
        "ls > out",
        " ./sum < in > out ",
        "ls | grep cpp",
        "ps | grep apache | less",
        "scp -P 12345 name@abc.xyz:~/xd ~/hello/xd",
        "cat < in | grep xd | sort > out",
        // error
        "cat < in > out | grep xd | sort > out"
    };
    
    for( int i = 0; i < 10; i++ ){
        printf("command %d: %s\n", i, cmd_string[i].c_str());

        cmd[i].parse_one_line_cmd(cmd_string[i]);

        for( int j = 0; j < cmd[i].cmd_count; j++ ){
            cmd[i].cmds[j].print();
        }
        printf("\n");
    }

    return 0;
}

#endif
