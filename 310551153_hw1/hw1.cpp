// COMMMAND /proc/[pid]/comm s
// PID      /proc/[pid] s
// UID      /proc/[pid]/status -> UID (id -nu </proc/[pid]/loginuid) 
// FD s
// TYPE s
// NODE
// NAME s
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <set>
#include <vector>
#include <regex>
using namespace std::filesystem;
using namespace std;

struct process{
    string command;
    string pid;
    string uid;
    string fd;
    string type;
    string node;
    string name;
};

string files_status(file_status s, string p)
{
    if(p.find("sock") != string::npos) return "SOCK";
    if(p.find("pipe") != string::npos) return "FIFO";
    if(p.find("fifo") != string::npos) return "FIFO";
    if(is_regular_file(s)) return "REG";
    if(is_directory(s)) return "DIR";
    if(is_block_file(s)) return "BLK";
    if(is_character_file(s)) return "CHR";
    if(is_fifo(s)) return "FIFO";
    //if(is_socket(s)) return "SOCK";
    return "unknown";
}

string files_status(file_status s)
{
    if(is_regular_file(s)) return "REG";
    if(is_directory(s)) return "DIR";
    if(is_block_file(s)) return "BLK";
    if(is_character_file(s)) return "CHR";
    if(is_fifo(s)) return "FIFO";
    if(is_socket(s)) return "SOCK";
    else    return "unknown";
}

string readFileIntoString(const string& path) {
    ifstream input_file(path);
    string data;
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
             << path << "'" << endl;
        exit(EXIT_FAILURE);
    }
    getline(input_file,data,'\n');
    return data;
}

string getusername(unsigned int uid){
    auto name = getpwuid(uid);
    return name->pw_name;
}

char perm(path dir){
    perms p = symlink_status(dir).permissions();;
    bool owner_read = ( (p & perms::owner_read) !=  perms::none );
	bool owner_write = ( (p & perms::owner_write) != perms::none );

    if (owner_read and owner_write)
		return 'u';
	else if (owner_read)
		return 'r';
	else if (owner_write)
		return 'w';
    return ' ';
}

int main(int argc, char** argv){
    
    cout<<"COMMAND"<<"\t\t"<<"PID"<<'\t'<<"USER"<<'\t'<<"FD"<<'\t'<<"TYPE"<<'\t'<<"NODE"<<'\t'<<"NAME"<<endl;
    path str("/proc");
    directory_iterator list(str);
    vector<struct process> process_list;
    for(auto & it:list){

        if(isdigit(it.path().filename().string()[0])){
            path dir(it.path());
            directory_entry entry(dir);

            set<string> tmp;

            string pid = dir.filename().string();

            struct stat filestat;
            const char *filepath = it.path().string().c_str();
            stat(filepath, &filestat);
            string uid = getusername(filestat.st_uid);


            string comm = it.path().string() += "/comm";
            path comms(comm);
            if(exists(comms)){
                comm = readFileIntoString(comm);
            }


            if(entry.status().type() == file_type::directory){
                directory_iterator proc_list(dir);
                struct process p_tmp;
                

                    string file = it.path().string() += "/cwd";
                    path i(file);
                            struct stat filestat;
                            const char *filepath = i.c_str();
                            stat(filepath, &filestat);
                        try{
                            read_symlink(i);
                            if((read_symlink(i).string()).find("delete") == string::npos){
                                p_tmp.command = comm;
                                p_tmp.pid = pid;
                                p_tmp.uid = uid;
                                p_tmp.fd = "cwd";
                                p_tmp.type = files_status(status(read_symlink(i).string()));
                                p_tmp.node = to_string(filestat.st_ino);
                                tmp.insert(p_tmp.node);
                                p_tmp.name = read_symlink(i).string();
                                process_list.push_back(p_tmp);
                            }
                            else{
                                p_tmp.command = comm;
                                p_tmp.pid = pid;
                                p_tmp.uid = getusername(filestat.st_uid);
                                p_tmp.fd = "cwd";
                                p_tmp.type = files_status(status(read_symlink(i).string()));
                                p_tmp.node = to_string(filestat.st_ino);
                                tmp.insert(p_tmp.node);
                                p_tmp.name = read_symlink(i).string().substr(0,read_symlink(i).string().find("("));
                                process_list.push_back(p_tmp);
                            }
                        }
                        catch(const exception& e){
                            p_tmp.command = comm;
                            p_tmp.pid = pid;
                            p_tmp.uid = uid;
                            p_tmp.fd = "cwd";
                            p_tmp.type = "unknown";
                            p_tmp.node = "";
                            p_tmp.name = i.string()+" (Permission denied)";
                            process_list.push_back(p_tmp);
                            
                        }
                    
                    file = it.path().string() += "/root";
                    i = file;    
                            struct stat filestatss;
                            const char *filepathss = i.string().c_str();
                            stat(filepathss, &filestatss);
                        try{
                            read_symlink(i);
                            if(read_symlink(i).string().find("delete") == string::npos){
                                p_tmp.command = comm;
                                p_tmp.pid = pid;
                                p_tmp.uid = uid;
                                p_tmp.fd = "rtd";
                                p_tmp.type = files_status(status(read_symlink(i).string()));
                                p_tmp.node = to_string(filestatss.st_ino);
                                tmp.insert(p_tmp.node);
                                p_tmp.name = read_symlink(i).string();
                                process_list.push_back(p_tmp);
                            }
                            else{
                                p_tmp.command = comm;
                                p_tmp.pid = pid;
                                p_tmp.uid = uid;
                                p_tmp.fd = "rtd";
                                p_tmp.type = files_status(status(read_symlink(i).string()));
                                p_tmp.node = to_string(filestat.st_ino);
                                tmp.insert(p_tmp.node);
                                p_tmp.name = read_symlink(i).string().substr(0,read_symlink(i).string().find("("));
                                process_list.push_back(p_tmp);
                            }
                        }
                        catch(const exception& e){
                            p_tmp.command = comm;
                            p_tmp.pid = pid;
                            p_tmp.uid = uid;
                            p_tmp.fd = "rtd";
                            p_tmp.type = "unknown";
                            p_tmp.node = "";
                            p_tmp.name = i.string()+" (Permission denied)";
                            process_list.push_back(p_tmp);
                        }   

                    file = it.path().string() += "/exe";
                    i = file;
                        
                            struct stat filestats;
                            const char *filepaths = i.string().c_str();
                            stat(filepaths, &filestats);
                        try{
                            if(is_symlink(i) && exists(i)){
                                p_tmp.command = comm;
                                p_tmp.pid = pid;
                                p_tmp.uid = uid;
                                p_tmp.fd = "txt";
                                p_tmp.type = files_status(status(read_symlink(i).string()));
                                p_tmp.node = to_string(filestats.st_ino);
                                tmp.insert(p_tmp.node);
                                p_tmp.name = read_symlink(i).string();
                                process_list.push_back(p_tmp);      
                            }   
                        }
                        catch(const exception& e){
                            p_tmp.command = comm;
                            p_tmp.pid = pid;
                            p_tmp.uid = uid;
                            p_tmp.fd = "txt";
                            p_tmp.type = "unknown";
                            p_tmp.node = "";
                            p_tmp.name = i.string()+" (Permission denied)";
                            process_list.push_back(p_tmp);
                        }
                    
                     file = it.path().string() += "/maps";
                    i = file;
                        try{
                            ifstream mem(i,ios::in);
                            if(!mem.is_open()){
                                cout<<"";
                            }
                            else{
                                string s;
                                while(getline(mem,s)){
                                    string inode;
                                    string content;
                                    for(int i = 46; i < s.size(); i++){
                                        if(s[i] == ' ') break;
                                        inode += s[i];
                                    }
                                    if(inode == "0" || tmp.count(inode) == 1)
                                        continue;
                                    tmp.insert(inode);
                                    for(int i = 57; i < s.size(); i++){
                                        content += s[i];
                                    }
                                    if(content.find("delete") == string::npos){
                                        p_tmp.command = comm;
                                        p_tmp.pid = pid;
                                        p_tmp.uid = uid;
                                        p_tmp.fd = "mem";
                                        p_tmp.type = "REG";
                                        p_tmp.node = inode;
                                        p_tmp.name = content;
                                        process_list.push_back(p_tmp);
                                    }
                                    else{
                                        p_tmp.command = comm;
                                        p_tmp.pid = pid;
                                        p_tmp.uid = uid;
                                        p_tmp.fd = "DEL";
                                        p_tmp.type = "REG";
                                        p_tmp.node = inode;
                                        p_tmp.name = content.substr(0,content.find("("));
                                        process_list.push_back(p_tmp);
                                    }
                                }
                            }
                            mem.close();
                        }
                        catch(const exception& e){
                            asm volatile("nop");
                        }
                    
                    file = it.path().string() += "/fd";
                    i = file;
                        path dir(i);
                        try{
                            directory_iterator proc_list(dir);
                            for(auto & item: proc_list){
                                struct stat filestat;
                                const char *filepath = item.path().string().c_str();
                                stat(filepath, &filestat);
                                string fd = item.path().filename().string() + perm(item.path());

                                if(is_symlink(item.path())){
                                    if(read_symlink(item.path()).string().find("delete") == string::npos){
                                        p_tmp.command = comm;
                                        p_tmp.pid = pid;
                                        p_tmp.uid = uid;
                                        p_tmp.fd = fd;
                                        p_tmp.type = files_status(status(read_symlink(item.path()).string()),read_symlink(item.path()).string());
                                        p_tmp.node = to_string(filestat.st_ino);
                                        tmp.insert(p_tmp.node);
                                        p_tmp.name = read_symlink(item.path()).string();
                                        process_list.push_back(p_tmp);
                                    }
                                    else{
                                        p_tmp.command = comm;
                                        p_tmp.pid = pid;
                                        p_tmp.uid = uid;
                                        p_tmp.fd = fd;
                                        p_tmp.type = files_status(status(read_symlink(item.path()).string()),read_symlink(item.path()).string());
                                        p_tmp.node = to_string(filestat.st_ino);
                                        tmp.insert(p_tmp.node);
                                        p_tmp.name = read_symlink(item.path()).string().substr(0,read_symlink(item.path()).string().find("("));
                                        process_list.push_back(p_tmp);
                                    }
                                }
                             }
                        }
                        catch(const exception& e){
                            p_tmp.command = comm;
                            p_tmp.pid = pid;
                            p_tmp.uid = uid;
                            p_tmp.fd = "NOFD";
                            p_tmp.type = "";
                            p_tmp.name = i.string()+" (Permission denied)";
                            process_list.push_back(p_tmp);
                        } 
                
            }
            tmp.clear();
        }
        
        
    }
    if(argc > 1){
        vector<string> matches;
        for(int i = 1; i < argc; i++){
            matches.push_back(argv[i]);
        }
        auto it = find(matches.begin(),matches.end(),"-c");
        if(it != matches.end()){
            string target = *(it + 1);
            smatch m;
            regex reg(target);
            int count=0;
            for(auto i = process_list.begin(); i < process_list.end(); i++){
                if(!regex_search(i->command,m,reg)){
                    i->command = "*";
                }
            }
        }

        it = find(matches.begin(),matches.end(),"-t");
        vector<string> TYPEs{"REG", "CHR",  "DIR",  "FIFO",  "SOCK", "unknown"};
	    if (it != matches.end()) {
		    string target = *(it+1);
		    if (find(TYPEs.begin(), TYPEs.end(), target) == TYPEs.end()) {
			    cout << "Invalid TYPE option.";
			    exit(0);
		    }
            else{
                smatch m;
                regex reg(target);
                for(auto i = process_list.begin(); i < process_list.end(); i++){
                    if(!regex_search(i->type,m,reg)){
                        i->command = "*";
                    }
                }
            }
        }

        it = find(matches.begin(),matches.end(),"-f");
        if(it != matches.end()){
            string target = *(it + 1);
            smatch m;
            regex reg(target);
            for(auto i = process_list.begin(); i < process_list.end(); i++){
                if(!regex_search(i->name,m,reg)){
                    i->command = "*";
                }
            }
        }

    }
    for(auto i : process_list){
        if(i.command != "*")
            cout<<i.command<<"         "<<i.pid<<"         "<<i.uid<<"            "<<i.fd<<"        "<<i.type<<"        "<<i.node<<"       "<<i.name<<endl;
    }

    return 0;
}