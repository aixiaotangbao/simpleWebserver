#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <getopt.h>
//#include <mcheck.h>
#include "Util.h"
#include "Server.h"
#include <queue>

// 实现守护进程

std::string basePath = "..";

void daemon_run(){
    int pid;
    signal(SIGCHLD,SIG_IGN);    // 忽略子进程状态改变信号
    pid = fork();
    if(pid < 0){
        std::cout<<"fork error"<<std::endl;
        exit(-1);
    } else if (pid > 0){    // 父进程退出
        exit(0);
    }
    //子进程建立一个新会话，并且该进程成为session leader以及进程组的组长
    setsid();
//    chdir("/");
    int fd;
    fd = open("/dev/null",O_RDWR,0);
    // 标准输入输出重定向到空设备
    if(fd != -1){
        dup2(fd,STDIN_FILENO);
        dup2(fd,STDOUT_FILENO);
        dup2(fd,STDERR_FILENO);
    }
    if(fd > 2){
        close(fd);
    }
}
int main(int argc,char *argv[]) {
//    setenv("MALLOC_TRACE","output",1);
//    mtrace();
    int ThreadNumber = 6;   // 默认线程数
    int port = 8080;          // 默认端口号
    std::string logPath = "./webserver.log";    // 默认日志路径
    bool isDaemon = false;
    int opt = 0;
    const char *str = "t:l:p:d";

    //TODO 添加用户自定义设置，先使用默认的配置
    while ((opt = getopt(argc,argv,str)) != -1){
        switch (opt){
            case 't':{
                ThreadNumber = atoi(optarg);
                break;
            }
            case 'l':{
                logPath = std::string(optarg);
                if(logPath.size() < 2 || optarg[0] != '/'){
                    cout<<"logPaht must start with /"<<endl;
                    abort();
                }
                break;
            }
            case 'p':{
                port = atoi(optarg);
                break;
            }
            case 'd':{
                isDaemon = true;
                break;
            }
            case '?':{
                cout<<"error option:"<<optopt<<endl;
                cout<<"error opterr:"<<opterr<<endl;
                break;
            }
            default:
                break;
        }
    }

    // 创建守护进程
    if(isDaemon)
        daemon_run();
    // 使用库函数daemon创建守护进程
//    if(daemon(0,0) == -1){
//        std::cout<<"daemon error"<<std::endl;
//        exit(1);
//    }
    int fd = open("/home/ubuntu/daemon.log",O_RDWR|O_CREAT|O_APPEND,0644);
    time_t t = time(0);
    char *buf = asctime(localtime(&t));
    write(fd,buf,strlen(buf));
    close(fd);

    if(check_base_path(basePath) < 0)
        std::cout<<"设置的基本工作路径不正确,工作路径为绝对路径,要使用相对路径请使用./"<<endl;
    {
        std::cout<<"************webServer 配置信息************"<<std::endl;
        std::cout<<"端口："<<port<<std::endl;
        std::cout<<"线程："<<ThreadNumber<<std::endl;
        std::cout<<"日志目录："<<logPath<<std::endl;
    }
    char *currentPath;
    currentPath = getcwd(nullptr,0);
    std::cout<<"当前工作路径为："<<currentPath<<std::endl;
    handle_for_sigpipe();
    HttpServer httpServer(port,"0.0.0.0");
    httpServer.run(ThreadNumber,1024);
    return 0;
}

