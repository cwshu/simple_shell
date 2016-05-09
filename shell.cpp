#include <iostream>
#include <stdio.h>

#include <unistd.h>

#include "cmd_parser.h"

void shell_init(){
    // initialize simple shell, like CWD, enviroment variables, chroot ...
}

void shell_prompt(){
    /* print shell prompt */
    std::cout << "(simple)> ";
}

bool is_internal_command(std::string executable){
    static std::string internal_command[10] = {
        "cd",
        "exit",
    };

    for(int i = 0; i < 10; i++){
        if( executable == internal_command[i] ){
            return true;
        }
    }
    return false;
}

bool shell_internal_command(SingleCommand cmd){
    /* 
     * processing shell internal command.
     * return true if internal command is exit.
     *
     * internal commands:
     * - cd 
     * - exit
     * - setenv
     * - block signal
     */

    if( cmd.executable == "exit" ){
        return true;
    }
    return false;
}

int create_pipe(int& read_end, int& write_end){
    int tmp[2];
    int ret = pipe(tmp);

    read_end = tmp[0];
    write_end = tmp[1];

    return ret;
}

bool execute_command(OneLineCommand line_command){
    /*
     * return is_exit or not.
     */

    if( line_command.cmd_count == 0 ){
        return false;
    }

    SingleCommand first_cmd = line_command.cmds[0];

    if( is_internal_command(first_cmd.executable) ){
       bool is_exit = shell_internal_command(first_cmd);
       return is_exit;
    }
    return false;
}

void simple_shell(){
    shell_init();

    while( 1 ){
        // print shell prompt
        shell_prompt();

        // read standard input
        char line_command_buffer[512];
        std::cin.getline(line_command_buffer, 512);
        if( std::cin.eof() ){
            std::cerr << "exit\n";
            break;
        }

        std::string line_command(line_command_buffer);
        OneLineCommand cmd;
        cmd.parse_one_line_cmd(line_command);

        bool is_exit = execute_command(cmd);
        if( is_exit ){
            break;
        }
    }
}

int main(){
    // argument parse

    // run shell
    simple_shell();

    return 0;
}
