#include <iostream>
#include <cstdio>
#include <cassert>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

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

    // std::cout << line_command.cmd_count << std::endl;
    if( line_command.cmd_count == 0 ){
        return false;
    }

    SingleCommand first_cmd = line_command.cmds[0];

    if( is_internal_command(first_cmd.executable) ){
       bool is_exit = shell_internal_command(first_cmd);
       return is_exit;
    }

    const int NO_PIPE = -1;
    int last_pipe_r = NO_PIPE;
    int current_pipe_r = NO_PIPE, current_pipe_w = NO_PIPE;
    int stdin_file_fd = -1, stdout_file_fd = -1;

    for( auto single_cmd : line_command.cmds ){
        // redirection file and pipe initialization
            // file creation
            // pipe sanity check and creation.

        // STDIN
        assert(!(single_cmd.std_input.type == REDIR_PIPE && last_pipe_r == NO_PIPE));
        if( single_cmd.std_input.type == REDIR_FILE ){
            stdin_file_fd = open(single_cmd.std_input.filename.c_str(), O_RDONLY);
            if( stdin_file_fd == -1 ){
                perror("open failed");
                exit(EXIT_FAILURE);
            }
        }
        // STDOUT
        if( single_cmd.std_output.type == REDIR_PIPE ){
            int ret = create_pipe(current_pipe_r, current_pipe_w);
            if(ret == -1) perror("pipe");
        }
        if( single_cmd.std_output.type == REDIR_FILE ){
            stdout_file_fd = open(single_cmd.std_output.filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
            if( stdout_file_fd == -1 ){
                perror("open failed");
                exit(EXIT_FAILURE);
            }
        }

        pid_t child_pid = fork();
        if( child_pid == -1 ){
            perror("fork failed");
        }
        else if( child_pid == 0 ){
            // Child
            // fork() and exec()
            // setpgid() 

            // single_cmd.print();

            // redirection: dup2
            if( single_cmd.std_input.type == REDIR_PIPE ){
                int ret;
                ret = dup2(last_pipe_r, STDIN_FILENO);
                if(ret == -1) perror("dup2");
                ret = close(last_pipe_r);
                if(ret == -1) perror("close");
            }
            else if( single_cmd.std_input.type == REDIR_FILE ){
                int ret;
                ret = dup2(stdin_file_fd, STDIN_FILENO);
                if(ret == -1) perror("dup2");
                ret = close(stdin_file_fd);
                if(ret == -1) perror("close");
            }

            if( single_cmd.std_output.type == REDIR_PIPE ){
                int ret;
                ret = dup2(current_pipe_w, STDOUT_FILENO);
                if(ret == -1) perror("dup2");
                ret = close(current_pipe_r);
                if(ret == -1) perror("close");
                ret = close(current_pipe_w);
                if(ret == -1) perror("close");
            }
            else if( single_cmd.std_output.type == REDIR_FILE ){
                int ret;
                ret = dup2(stdout_file_fd, STDOUT_FILENO);
                if(ret == -1) perror("dup2");
                ret = close(stdout_file_fd);
                if(ret == -1) perror("close");
            }

            // exec
            char **argv = single_cmd.gen_argv();
            execvp(single_cmd.executable.c_str(), argv);

            // exec error
            perror("exec error");
            std::cerr << "Unknown command: "<< single_cmd.executable << std::endl;
            exit(EXIT_FAILURE);
        }
        else {
            // parent
            // wait for child | background running
            waitpid(-1, NULL, 0);

            // file/pipe deletion
            close(stdin_file_fd);
            close(stdout_file_fd);

            close(last_pipe_r);
            last_pipe_r = current_pipe_r;
            close(current_pipe_w);
        }
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
        // std::cout << "line_command: " << line_command << std::endl;
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
