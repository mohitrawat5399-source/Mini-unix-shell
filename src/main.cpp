#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <vector>

int main() {
    std::string command;


    while (true) {
        std::cout << "msh> ";

        if (!std::getline(std::cin, command)) {
            break;
        }

        if (command == "exit") {
            break;
        }
         std::istringstream iss(command);
         std::vector<std::string> tokens;
         std::string token;

         while (iss >> token) {
             tokens.push_back(token);
        }
        if (tokens.empty()) {
             continue;
        }

        
        if (tokens[0] == "cd") {
        if (tokens.size() < 2) {
            std::cerr << "msh: cd: missing argument\n";
        } 
         else if (chdir(tokens[1].c_str()) != 0) {
           std::cerr << "msh: cd: failed to change directory\n";
         }

         continue;
        }
        bool redirect_output = false;
        std::string output_file;

        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i] == ">") {
               if (i + 1 >= tokens.size()) {
               std::cerr << "msh: missing output file\n";
               redirect_output = false;
               output_file.clear();
                break;
            }

             redirect_output = true;
             output_file = tokens[i + 1];

             tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
             break;
           }
        }
        pid_t pid = fork();
        std::vector<char*> argv;

        for (auto& token : tokens) {
        argv.push_back(token.data());
        }


        argv.push_back(nullptr);
        
        if (pid < 0) {
            std::cerr << "msh: fork failed\n";
            continue;
        }

        if (pid == 0) {
            // Child process
            if (redirect_output) {
                int fd = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd < 0) {
                perror("msh: open");
                return 1;
              }

             if (dup2(fd, STDOUT_FILENO) < 0) {
                 perror("msh: dup2");
                 close(fd);
                 return 1;
             }

              close(fd);
            }
            execvp(argv[0], argv.data());


            // Only reached if exec fails
            std::cerr << "msh: command not found: " << command << '\n';
            return 1;
        }

        // Parent process
        waitpid(pid, nullptr, 0);
    }

    return 0;
}
