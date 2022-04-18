#include <sys/types.h> 
#include <fcntl.h>
#include <sys/stat.h> 
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;
void helper(){
    cerr<<"usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]"<<endl;
    cerr<<"\t-p: set the path to logger.so, default = ./logger.so"<<endl;
    cerr<<"\t-o: print output to file, print to \"stderr\" if no file specified"<<endl;
    cerr<<"\t--: separate the arguments for logger and for the command"<<endl;
}
int main(int argc,char** argv){

    if(argc == 1){
        cout<<"no command given."<<endl;
        exit(-1);
    }

    int dash_idx = -1, log_idx = 0, cmd_idx = 0;
    char* log_instr[5];
    char* cmd_instr[10];
    int log_exist = (argv[1][0] == '-') ? 1 : 0;
    for(int i = 0; i < argc; i++){
        if(log_exist == 1){
            if(!strcmp(argv[i],"--")){
                dash_idx = i;
                break;
            }
            else{
                log_instr[log_idx++] = argv[i];
            }
        }
        else break;
    }


    char ch;
    string path,output;
    int is_output = 0, is_path = 0;
    while((ch = getopt(log_idx,log_instr,"o:p:")) != EOF){
        switch(ch){
            case 'o':
                is_output = 1;
                output = optarg;
                break;
            case 'p':
                is_path = 1;
                path = optarg;
                break;
            default:
                helper();
                exit(-1);
        }
    }
    if(!is_path)  path = "./logger.so";
    if(!is_output) output = "STDERR";
    //get cmd index
    if(dash_idx == -1){
        cmd_idx = 1;
    }
    else
        cmd_idx = dash_idx + 1;

    //get cmd form argv
    int cmd_argc = 0;
    for(int i = cmd_idx; i < argc; i++)
        cmd_instr[cmd_argc++] = argv[i];
    cmd_instr[cmd_argc] = NULL;

    /*
    for(int i = 0; i < cmd_argc ; i++)
        cout<<cmd_instr[i]<<" ";
    cout<<endl;
    */
    string preload = "LD_PRELOAD="+path;
    string output_file = "OUTPUT_FILE="+output;
    char* env[3] = {strdup(preload.c_str()), strdup(output_file.c_str()), NULL};
    int ii = execvpe(cmd_instr[0],cmd_instr,env);
}