#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fstream>

using namespace std;

// 函数原型
int run_command(vector<string> &tokens);
void printf_info();
void execute_script(const string &filename);

// 主函数
int main(int argc, char* argv[]) {
    if (argc > 1) {
        // 如果提供了脚本文件作为参数，则执行脚本文件
        execute_script(argv[1]);
        return 0;
    }

    string input;
    while (true) {
        printf_info();
        getline(cin, input);

        // 解析命令
        vector<string> tokens;
        string token;
        for (char c : input) {
            if (c == ' ') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }

        // 检查是否有管道
        vector<vector<string>> commands;
        size_t start = 0;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i] == "|") {
                vector<string> cmd(tokens.begin() + start, tokens.begin() + i);
                commands.push_back(cmd);
                start = i + 1;
            }
        }
        vector<string> cmd(tokens.begin() + start, tokens.end());
        commands.push_back(cmd);

        // 创建管道
        int numPipes = commands.size() - 1;
        if (numPipes == 0) {
            int i = run_command(tokens);
            if (i == 1) {
                return 0;
            }
            continue;
        }

        int pipefd[numPipes][2];

        for (int i = 0; i < numPipes; ++i) {
            if (pipe(pipefd[i]) == -1) {
                cerr << "Pipe failed." << endl;
                exit(EXIT_FAILURE);
            }
        }

        // 执行命令
        pid_t pid;
        int input_fd = STDIN_FILENO;
        for (int i = 0; i < commands.size(); ++i) {
            pid = fork();

            if (pid == 0) { // 子进程
                if (i != 0) {
                    dup2(pipefd[i - 1][0], STDIN_FILENO);
                }
                if (i != numPipes) {
                    dup2(pipefd[i][1], STDOUT_FILENO);
                }

                // 关闭管道文件描述符
                for (int j = 0; j < numPipes; ++j) {
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }

                run_command(commands[i]);
                exit(EXIT_SUCCESS);
            } else if (pid < 0) {
                cerr << "Fork failed." << endl;
                exit(EXIT_FAILURE);
            }
        }

        // 父进程关闭管道文件描述符并等待子进程结束
        for (int i = 0; i < numPipes; ++i) {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }

        for (int i = 0; i < commands.size(); ++i) {
            wait(NULL);
        }
    }

    return 0;
}

// 执行命令
int run_command(vector<string> &tokens) {
    pid_t pid = fork();
    if (pid < 0) {
        cout << "执行命令时子进程创建失败！" << endl;
        exit(-1);
    } else if (pid == 0) { // 子进程的执行步骤
        // 输入重定向
        int index = 0;
        int type_in = 0;
        for (; index < tokens.size(); index++) {
            if (tokens.at(index) == "<") {
                type_in = 1;
                break;
            }
            if (tokens.at(index) == "<<") {
                type_in = 2;
                break;
            }
        }
        int fd_input = -1;
        int pipefd[2];
        if (index == 0) {
            cout << "输入重定向使用错误！" << endl;
        } else if (index != tokens.size()) {
            if (type_in == 1) {
                fd_input = open(tokens.at(index + 1).c_str(), O_RDONLY);
                close(0);
                dup2(fd_input, 0);
                tokens.erase(tokens.begin() + index + 1);
                tokens.erase(tokens.begin() + index);
            } else { // 处理输入重定向符号 <<
                string temp;
                string input;
                cout << "> ";
                fflush(stdout);
                while (getline(cin, temp)) { // 读取输入并保存进ss中
                    if (temp == tokens.at(index + 1)) {
                        break;
                    }
                    cout << "> ";
                    fflush(stdout);
                    input += temp;
                    input += "\n";
                }
                int length = input.length();
                const char* data = input.c_str();
                pipe(pipefd); // 创建一个管道
                write(pipefd[1], data, length); // 向管道中写入数据，write无缓存，无需刷新
                close(0);
                dup2(pipefd[0], 0);
                for (int i = 0; i < 2; i++) {
                    close(pipefd[i]);
                }
                tokens.erase(tokens.begin() + index + 1);
                tokens.erase(tokens.begin() + index);
            }
        }
        // 输出重定向
        int type_out = 0;
        index = 0;
        for (; index < tokens.size(); index++) {
            if (tokens.at(index) == ">") {
                type_out = 1;
                break;
            }
            if (tokens.at(index) == ">>") {
                type_out = 2;
                break;
            }
        }
        int fd_output = -1;
        if (index == 0) {
            cout << "输出重定向使用错误！" << endl;
        } else if (index != tokens.size()) { // 处理输出重定向符号
            if (type_out == 1) {
                fd_output = open(tokens.at(index + 1).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
            } else if (type_out == 2) {
                fd_output = open(tokens.at(index + 1).c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
            }
            close(1);
            dup2(fd_output, 1);
            tokens.erase(tokens.begin() + index + 1);
            tokens.erase(tokens.begin() + index);
        }
        bool isInterCmd = true;
        // 尝试执行shell内置命令
        if (tokens[0] == "help") {
            struct passwd* pwp = getpwuid(getuid());
            printf("欢迎来到HELP界面,尊敬的用户 %s !\n", pwp->pw_name);
            printf("在shell中实现了以下的内部指令: \n");
            printf("1.   指令: cd  作用: 用于切换工作目录  指令格式: cd path  解释: path为想要切换到的工作目录，如果不输入path，则切换到当前用户的home目录 \n");
            printf("2.   指令: exit  作用: 用于退出shell  指令格式: exit \n");
            printf("3.   指令: help  作用: 用于获取内部指令的使用帮助  指令格式: help \n");
        } else {
            isInterCmd = false;
        }
        if (isInterCmd) { // 内部命令已经执行完毕，开始退出
            exit(0);
        }
        const int tokenCount = tokens.size();
        const char *cmd[tokenCount + 1];
        for (int i = 0; i < tokenCount; ++i) {
            cmd[i] = tokens[i].c_str();
        }
        cmd[tokenCount] = nullptr;
        // 执行外部指令
        execvp(cmd[0], const_cast<char *const *>(cmd));
        exit(-1);
    } else if (pid > 0) {
        wait(NULL);
        if (tokens[0] == "exit") { // 执行内部的exit指令
            return 1;
        } else if (tokens[0] == "cd") { // 执行内部的cd指令
            if (tokens.size() == 2) {
                if (chdir(tokens[1].c_str()) != 0) {
                    perror("当前目录修改失败！");
                }
            } else if (tokens.size() == 1) { // 返回当前用户的home目录
                uid_t uid = geteuid(); // 获取有效用户ID
                struct passwd *pw = getpwuid(uid);
                if (pw) {
                    string homeDir(pw->pw_dir);
                    if (chdir(homeDir.c_str()) != 0) {
                        perror("当前目录修改失败！");
                    }
                }
            }
        }
    }
    return 0;
}

// 打印用户提示信息
void printf_info() {
    char path[1000];
    struct passwd *pwp;
    pwp = getpwuid(getuid()); // 获取当前用户的passwd结构体
    char hostname[1000];
    gethostname(hostname, sizeof(hostname)); // 获取当前主机名
    getcwd(path, sizeof(path)); // 获取当前工作目录
    printf("\033[1;31m%s@%s\033[0m:\033[1;33m%s\033[0m$ ", pwp->pw_name, hostname, path);
    fflush(stdout); // 刷新标准输出缓存区，确保显示及时
}

// 执行脚本文件
void execute_script(const string &filename) {
    ifstream script_file(filename);
    string line;
    while (getline(script_file, line)) {
        vector<string> tokens;
        string token;
        for (char c : line) {
            if (c == ' ') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        if (!tokens.empty()) {
            run_command(tokens);
        }
    }
}
