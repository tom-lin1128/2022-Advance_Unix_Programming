#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <cstdlib>
#include <cmath>
#include <stdarg.h>

using namespace std;

int flag_fd = 0;
int stderrfd = 2;

void file_init(){
    auto new_open = (int (*)(const char*, int, mode_t))dlsym(RTLD_NEXT, "open");
    auto new_write = (ssize_t (*)(int,const void*,size_t))dlsym(RTLD_NEXT, "write");
    char* env = getenv("OUTPUT_FILE");

    if(strcmp(env ,"STDERR") != 0){
        stderrfd = new_open(env, O_RDWR | O_CREAT | O_TRUNC, 0644);
    }
    else{
        stderrfd = dup(2);
    }
    flag_fd = 1;
}

void print(string s){
    auto new_write = (ssize_t (*)(int,const void*,size_t))dlsym(RTLD_NEXT, "write");
    new_write(stderrfd,s.c_str(),strlen(s.c_str()));
}



int chmod(const char *path, mode_t mode){
    if(!flag_fd) file_init();

    auto new_chmod = (int (*)(const char*, mode_t))dlsym(RTLD_NEXT, "chmod");
    string out_path = "[logger] chmod(";
    char abs_path[100];
    char* exist = realpath(path,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(path) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    
    char mode_value[10];
    sprintf(mode_value,"%03o",mode);
    out_path += string(mode_value) + ") = ";

    int ret = new_chmod(path,mode);

    out_path += to_string(ret) + "\n";
    print(out_path);
    return ret;
}

int chown(const char *pathname, uid_t owner, gid_t group){
    if(!flag_fd) file_init();

    auto new_chown = (int (*)(const char*, uid_t, gid_t))dlsym(RTLD_NEXT, "chown");
    string out_path = "[logger] chown(";
    char abs_path[100];
    char* exist = realpath(pathname,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(pathname) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    
    out_path += to_string(owner) + ", " + to_string(group) + ") = ";

    int ret = new_chown(pathname,owner,group);

    out_path += to_string(ret) + "\n";
    print(out_path);
    return ret;
}

int creat(const char *path, mode_t mode){
    if(!flag_fd) file_init();

    auto new_creat = (int (*)(const char*, mode_t))dlsym(RTLD_NEXT, "creat");
    string out_path = "[logger] creat(";
    char abs_path[100];
    char* exist = realpath(path,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(path) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    
    char mode_value[10];
    sprintf(mode_value,"%03o",mode);
    out_path += string(mode_value) + ") = ";

    int ret = new_creat(path,mode);

    out_path += to_string(ret) + "\n";
    print(out_path);
    return ret;
}

int creat64(const char *path, mode_t mode){
    if(!flag_fd) file_init();

    auto new_creat = (int (*)(const char*, mode_t))dlsym(RTLD_NEXT, "creat64");
    string out_path = "[logger] creat64(";
    char abs_path[100];
    char* exist = realpath(path,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(path) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    
    char mode_value[10];
    sprintf(mode_value,"%03o",mode);
    out_path += string(mode_value) + ") = ";

    int ret = new_creat(path,mode);

    out_path += to_string(ret) + "\n";
    print(out_path);
    return ret;
}

int close(int fd){
    if(!flag_fd) file_init();

    auto new_close = (int (*)(int))dlsym(RTLD_NEXT, "close");
    string out_path = "[logger] close(", path;
    char abs_path[1024];
    int length = readlink(string("/proc/self/fd/"+to_string(fd)).c_str(),abs_path,1023);
    if(length != -1){
        abs_path[length] = '\0';
        path = string(abs_path);
    }

    out_path += "\"" + string(path) + "\") = ";

    int ret = new_close(fd);
    out_path += to_string(ret) + "\n";
    print(out_path);
    return ret;
}

int fclose(FILE *stream){
    if(!flag_fd) file_init();

    auto new_fclose = (int (*)(FILE*))dlsym(RTLD_NEXT, "fclose");
    int fd = fileno(stream);
    string out_path = "[logger] fclose(", path;
    char abs_path[1024];
    int length = readlink(string("/proc/self/fd/"+to_string(fd)).c_str(),abs_path,1023);
    if(length != -1){
        abs_path[length] = '\0';
        path = string(abs_path);
    }

    out_path += "\"" + string(path) + "\") = ";

    int ret = new_fclose(stream);
    out_path += to_string(ret) + "\n";
    print(out_path);
    return ret;
}

FILE *fopen(const char * pathname, const char * mode){
    if(!flag_fd) file_init();

    auto new_fopen = (FILE* (*)(const char*, const char *))dlsym(RTLD_NEXT, "fopen");
    string out_path = "[logger] fopen(";
    char abs_path[100];
    char* exist = realpath(pathname,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(pathname) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    
    out_path += "\"" + string(mode) + "\") = ";

    FILE* ret = new_fopen(pathname,mode);

    char addr[20];
    sprintf(addr,"%p",ret);
    out_path += string(addr) + "\n";
    print(out_path);
    return ret;
}

FILE *fopen64(const char * pathname, const char * mode){
    if(!flag_fd) file_init();

    auto new_fopen = (FILE* (*)(const char*, const char *))dlsym(RTLD_NEXT, "fopen64");
    string out_path = "[logger] fopen64(";
    char abs_path[100];
    char* exist = realpath(pathname,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(pathname) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    
    out_path += "\"" + string(mode) + "\") = ";

    FILE* ret = new_fopen(pathname,mode);

    char addr[20];
    sprintf(addr,"%p",ret);
    out_path += string(addr) + "\n";
    print(out_path);
    return ret;
}

size_t fread(void * ptr, size_t size, size_t count, FILE * stream){
    if(!flag_fd) file_init();

    auto new_fread = (size_t (*)(void *, size_t, size_t, FILE *))dlsym(RTLD_NEXT, "fread");
    int fd = fileno(stream);
    string out_path = "[logger] fread(\"";
    char abs_path[100];
    int length = readlink(string("/proc/self/fd/"+to_string(fd)).c_str(),abs_path,strlen(abs_path));

    size_t ret = new_fread(ptr,size,count,stream);
    string buf;
    for(int i = 0; i < min((unsigned long)32,size*count); i++){
        if(*((char*)ptr+i) == '\0')
            break;

        if(isprint(*((char*)ptr+i)))
            buf += *((char*)ptr+i);
        else
            buf += '.';
    }

    out_path += buf + "\", " + to_string(size) +", " + to_string(count) + ", \"";

    string path;
    if(length != -1){
        abs_path[length] = '\0';
        path = string(abs_path);
    }
    
    out_path += path + "/\" ) = ";

    out_path += to_string(ret) + "\n";

    print(out_path);
    return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream){
    if(!flag_fd) file_init();

    auto new_fwrite = (size_t (*)(const void *, size_t, size_t, FILE *))dlsym(RTLD_NEXT, "fwrite");
    int fd = fileno(stream);
    string out_path = "[logger] fwrite(";
    char abs_path[1024];
    int length = readlink(string("/proc/self/fd/"+to_string(fd)).c_str(),abs_path,1023);

    size_t ret = new_fwrite(ptr,size,nmemb,stream);
    string buf;
    for(int i = 0; i < min((unsigned long)32,size*nmemb); i++){
        if(*((char*)ptr+i) == '\0')
            break;

        if(isprint(*((char*)ptr+i)))
            buf += *((char*)ptr+i);
        else
            buf += '.';
    }

    out_path += "\"" + buf + "\", " + to_string(size) +", " + to_string(nmemb) + ", ";

    string path;
    if(length != -1){
        abs_path[length] = '\0';
        path = string(abs_path);
    }
    
    out_path += path + "/ ) = ";

    out_path += to_string(ret) + "\n";

    print(out_path);
    return ret;

}

int open(const char *pathname, int flags, ...){
    if(!flag_fd) file_init();
    
    mode_t mode = 0;
    auto new_open = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open");
    string out_path = "[logger] open(";
    char abs_path[100];
    char* exist = realpath(pathname,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(pathname) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    char ss_mode[10],ss_flag[10];
    sprintf(ss_flag,"%o",flags);
    out_path += string(ss_flag) + ", ";
    int ret;
    if(__OPEN_NEEDS_MODE (flags))
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
        sprintf(ss_mode, "%03o", mode);
        ret = new_open(pathname, flags, mode);
    }
    else
    {
        ret = new_open(pathname, flags);
        sprintf(ss_mode, "%03o", mode);
    }

    out_path += string(ss_mode)+ ") = " + to_string(ret) + "\n";

    print(out_path);
    return ret;
}

int open64(const char *pathname, int flags, ...){
    if(!flag_fd) file_init();
    
    mode_t mode = 0;
    auto new_open = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open64");
    string out_path = "[logger] open64(";
    char abs_path[100];
    char* exist = realpath(pathname,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(pathname) + "\",";
    else
        out_path += "\"" + string(abs_path) + "\",";
    char ss_mode[10],ss_flag[10];
    sprintf(ss_flag,"%o",flags);
    out_path += string(ss_flag) + ", ";
    int ret;
    if(__OPEN_NEEDS_MODE (flags))
    {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
        sprintf(ss_mode, "%03o", mode);
        ret = new_open(pathname, flags, mode);
    }
    else
    {
        ret = new_open(pathname, flags);
        sprintf(ss_mode, "%03o", mode);
    }

    out_path += string(ss_mode)+ ") = " + to_string(ret) + "\n";

    print(out_path);
    return ret;
}

ssize_t read(int fd, void *buf, size_t len){
    if(!flag_fd) file_init();

    auto new_read = (ssize_t (*)(int , void*, size_t))dlsym(RTLD_NEXT, "read");
    string out_path = "[logger] read(";
    char abs_path[1024];
    int length = readlink(string("/proc/self/fd/"+to_string(fd)).c_str(),abs_path,1023);

    ssize_t ret = new_read(fd,buf,len);
    string buffer;
    for(int i = 0; i < min((unsigned long)32,len); i++){
        if(*((char*)buf+i) == '\0')
            break;

        if(isprint(*((char*)buf+i)))
            buffer += *((char*)buf+i);
        else
            buffer += '.';
    }

    string path;
    if(length != -1){
        abs_path[length] = '\0';
        path = string(abs_path);
    }

    out_path += "\"" + path + "/\", \"" + buffer + "\", " + to_string(len) + ") = ";

    out_path += to_string(ret) + "\n";

    print(out_path);
    return ret;
}    

int remove(const char *pathname){
    if(!flag_fd) file_init();

    auto new_remove = (int (*)(const char*))dlsym(RTLD_NEXT, "remove");
    string out_path = "[logger] remove(";
    char abs_path[100];
    char* exist = realpath(pathname,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(pathname) + "\") = ";
    else
        out_path += "\"" + string(abs_path) + "\") = ";
    
    int res = new_remove(pathname);

    out_path += to_string(res) + "\n";
    print(out_path);
    return res;
}

int rename(const char *oldpath, const char *newpath){
    if(!flag_fd) file_init();

    auto new_rename = (int (*)(const char *,const char *))dlsym(RTLD_NEXT, "rename");
    string out_path = "[logger] rename(";
    char abs_path[100];
    char* exist = realpath(oldpath,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(oldpath) + "\", ";
    else
        out_path += "\"" + string(abs_path) + "\", ";

    int res = new_rename(oldpath,newpath);
    memset(abs_path,0,100);

    exist = realpath(newpath,abs_path);
    if(exist == NULL)
        out_path += "\"" + string(newpath) + "\") = ";
    else
        out_path += "\"" + string(abs_path) + "\") = ";
    
    out_path += to_string(res) + "\n";
    print(out_path);
    return res;
}

ssize_t write(int fd, const void *buf, size_t count){
    if(!flag_fd) file_init();

    auto new_write = (ssize_t (*)(int ,const void*, size_t))dlsym(RTLD_NEXT, "write");
    string out_path = "[logger] write(";
    char abs_path[1024];
    int length = readlink(string("/proc/self/fd/"+to_string(fd)).c_str(),abs_path,1023);
    ssize_t ret = new_write(fd,buf,count);
    string buffer;
    for(int i = 0; i < min((unsigned long)32,count); i++){
        if(*((char*)buf+i) == '\0')
            break;

        if(isprint(*((char*)buf+i)))
            buffer += *((char*)buf+i);
        else
            buffer += '.';
    }

    string path;
    if(length != -1){
        abs_path[length] = '\0';
        path = string(abs_path);
    }

    out_path += "\"" + path + "/\", \"" + buffer + "\", " + to_string(count) + ") = ";

    out_path += to_string(ret) + "\n";

    print(out_path);
    return ret;
}

FILE* tmpfile(void){
    if(!flag_fd) file_init();

    auto new_tmpfile = (FILE* (*)(void))dlsym(RTLD_NEXT, "tmpfile");
    string out_path = "[logger] tmpfile() = ";
    FILE* ret = new_tmpfile();
    char buf[20];
    sprintf(buf,"%p", ret);
    out_path += string(buf) + "\n";
    print(out_path);
    return ret;
}

FILE* tmpfile64(void){
    if(!flag_fd) file_init();

    auto new_tmpfile = (FILE* (*)(void))dlsym(RTLD_NEXT, "tmpfile64");
    string out_path = "[logger] tmpfile64() = ";
    FILE* ret = new_tmpfile();
    char buf[20];
    sprintf(buf,"%p", ret);
    out_path += string(buf) + "\n";
    print(out_path);
    return ret;
}


