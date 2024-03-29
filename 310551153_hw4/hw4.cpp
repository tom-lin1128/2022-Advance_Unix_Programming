#include <sys/ptrace.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <capstone/capstone.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hw4.hpp"
using namespace std;


state_t state = UNDEFINE;
cmd_source_t source = ARGS;
struct user_regs_struct regs = {0};
map<string, unsigned long long*> regslist;
map<string, unsigned long long*>::iterator iter;
Elf64_Shdr text;
vector<breakpoint_t> points;
int bpid = 0;
char* code = NULL;
char restore = 0;
long long dumpaddr = -1, bpaddr_now = 0;
unsigned long long rip_now = 0;
elfhandle_t* e = (elfhandle_t*)calloc(1, sizeof(elfhandle_t));
vector<string> cmd;
string program;
ifstream fin;
bool file_flag = 0;
pid_t child = 0;


void help(){
    cout << "- break {instruction-address}: add a break point" << endl;
    cout << "- cont: continue execution" << endl;
    cout << "- delete {break-point-id}: remove a break point" << endl;
    cout << "- disasm addr: disassemble instructions in a file or a memory region" << endl;
    cout << "- dump addr: dump memory content" << endl;
    cout << "- exit: terminate the debugger" << endl;
    cout << "- get reg: get a single value from a register" << endl;
    cout << "- getregs: show registers" << endl;
    cout << "- help: show this message" << endl;
    cout << "- list: list break points" << endl;
    cout << "- load {path/to/a/program}: load a program" << endl;
    cout << "- run: run the program" << endl;
    cout << "- vmmap: show memory layout" << endl;
    cout << "- set reg val: get a single value to a register" << endl;
    cout << "- si: step into instruction" << endl;
    cout << "- start: start the program and stop at the first instruction" << endl;
}

void init_regs_map(){
    regslist["rax"] = 0;
    regslist["rbx"] = 0;
    regslist["rcx"] = 0;
    regslist["rdx"] = 0;
    regslist["r8"] = 0;
    regslist["r9"] = 0;
    regslist["r10"] = 0;
    regslist["r11"] = 0;
    regslist["r12"] = 0;
    regslist["r13"] = 0;
    regslist["r14"] = 0;
    regslist["r15"] = 0;
    regslist["rdi"] = 0;
    regslist["rsi"] = 0;
    regslist["rbp"] = 0;
    regslist["rsp"] = 0;
    regslist["rip"] = 0;
    regslist["flags"] = 0;
}

vector<string> getcmd(string line){
    vector<string> v;
    string tmp;
    stringstream tt;
    tt << line;
    while(tt >> tmp)
        v.push_back(tmp);
    return v;
}

void error_ocr(string msg){
    cout << msg << endl;
    elf_close(e);
    exit(-1);
}

void getargs(int argc, char** argv){
    if(argc >= 2){
        if(string(argv[1]) == "-s"){
            source = SCRIPT;
            fin.open(argv[2]);
            file_flag = 1;
            if(argc > 3){
                load(argv[3]);
            }
        }
        else{
            load(argv[1]);
        }
    }
    return;
}

void getcode(){
    ifstream f;
    f.open(program.c_str(), ios::in | ios::binary | ios::ate);
    streampos size;
    size = f.tellg();
    int codesize = size + 1L;
    f.seekg(0, ios::beg);
    code = new char [codesize];
    f.read(code, size);
    code[size] = 0;
    f.close();
}

string disassemble(char* pos, long long* addr, int len){
    csh handle = 0;
    cs_insn *insn;
    size_t count;
    string instruction = "";
    if(cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
        error_ocr("** disassemble error.");
    if((count = cs_disasm(handle, (uint8_t*)pos, len, *addr, 0, &insn)) > 0){
        stringstream ss;
        string printable, tmp;
        for(int i = 0; i < insn[0].size; i++){
            stringstream sss;
            int val = int(insn[0].bytes[i]);
            sss << hex << setw(2) << setfill('0') << val;
            sss >> tmp;
            printable += tmp + " ";
        }
        if(insn[0].size == 2 || insn[0].size == 1)
            ss << "\t" << hex << insn[0].address << dec << ": " << hex << printable << dec << "\t\t\t" << insn[0].mnemonic << "\t" << insn[0].op_str << endl;
        else 
            ss << "\t" << hex << insn[0].address << dec << ": " << hex << printable << dec << "\t\t" << insn[0].mnemonic << "\t" << insn[0].op_str << endl;
        *addr += insn[0].size;
        cs_free(insn, count);
        instruction = ss.str();
    }
    else
        error_ocr("** disassemble error.");
    cs_close(&handle);
    return instruction;
}

char code_change(long long addr, char byte){
    long long word = ptrace(PTRACE_PEEKTEXT, child, addr, 0);
    ptrace(PTRACE_POKETEXT, child, addr, (word & 0xffffffffffffff00) | (byte & 0xff));
    return word & 0xff;
}

int check_bp(){
    int status;
    if(waitpid(child, &status, 0) < 0)
        error_ocr("** wait error.");
    if(WIFSTOPPED(status)){
        getregs(false);
        long long rip = *regslist["rip"];
        for(int i = 0; i < int(points.size()); i++){
            if(points[i].addr == (rip - 1)){
                if(rip_now == (long long unsigned int)points[i].addr){
                    *regslist["rip"] = (rip - 1);
                    ptrace(PTRACE_SETREGS, child, 0, &regs);
                    restore = points[i].original;
                    return 1;
                }
                stringstream ss;
                string tmp;
                bpaddr_now = points[i].addr;
                cout << "** breakpoint @\t";
                ss << hex << bpaddr_now;
                tmp = ss.str();
                disasm(tmp, 1);
                *regslist["rip"] = (rip - 1);
                rip_now = (rip - 1);
                ptrace(PTRACE_SETREGS, child, 0, &regs);
                return 0;
            }
        }
        return -1;
    }
    else if(WIFEXITED(status)){
        cout << "** child process " << child << " " << "terminated normally (code " << status << ")" << endl;
        state = LOADED;
        return -1;
    }
    return -1;
}


void load(const char* elfFile){
    if(state == LOADED){
        cout << "** program has already been loaded." << endl;
        return;
    }
    if(elfFile == "NA"){
        cout << "** no program path is given" << endl;
        return;
    }
    int ret = 0;
    ret = elf_open(e, elfFile);
    if(!ret){
        elf_load(e);
        state = LOADED;
        cout << "** program '" << elfFile << "' loaded. entry point 0x" << hex << e->entry << dec << endl;
        program = elfFile;
    }
    else{
        error_ocr("** fail to load '" + string(elfFile) + "'.");
    }
    strtab_t* tab;
    for(tab = e->strtab; tab != NULL; tab = tab->next)
        if(tab->id == e->sh_strndx)
            break;
    for(int i = 0; i < e->sh_cnt; i++)
        if(!strcmp(&tab->data[e->shdr[i].sh_name], ".text")){
            text = e->shdr[i];
            break;
        }
}

int getregs(bool flag){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return -1;
    }
    if(ptrace(PTRACE_GETREGS, child, 0, &regs) < 0)
        error_ocr("** ptrace error.");
    
    if(flag){
        cout << hex;
        cout << "RAX " << regs.rax << "\t\tRBX " << regs.rbx << "\t\tRCX " << regs.rcx << "\t\tRDX " << regs.rdx << endl
             << "R8 "  << regs.r8  << "\t\tR9 "  << regs.r9  << "\t\tR10 " << regs.r10 << "\t\tR11 " << regs.r11 << endl
             << "R12 " << regs.r12 << "\t\tR13 " << regs.r13 << "\t\tR14 " << regs.r14 << "\t\tR15 " << regs.r15 << endl
             << "RDI " << regs.rdi << "\t\tRSI " << regs.rsi << "\t\tRBP " << regs.rbp << "\t\tRSP " << regs.rsp << endl;
        ios::fmtflags f(cout.flags());
        cout << "RIP " << regs.rip << "\t\tFLAGS " << setw(16) << setfill('0') << regs.eflags << endl;
        cout.flags(f);
        cout << dec;
    }
    
    regslist["rax"] = &regs.rax;
    regslist["rbx"] = &regs.rbx;
    regslist["rcx"] = &regs.rcx;
    regslist["rdx"] = &regs.rdx;
    regslist["r8"] = &regs.r8;
    regslist["r9"] = &regs.r9;
    regslist["r10"] = &regs.r10;
    regslist["r11"] = &regs.r11;
    regslist["r12"] = &regs.r12;
    regslist["r13"] = &regs.r13;
    regslist["r14"] = &regs.r14;
    regslist["r15"] = &regs.r15;
    regslist["rdi"] = &regs.rdi;
    regslist["rsi"] = &regs.rsi;
    regslist["rbp"] = &regs.rbp;
    regslist["rsp"] = &regs.rsp;
    regslist["rip"] = &regs.rip;
    regslist["flags"] = &regs.eflags;
    return 0;
}

void getreg(string reg){
    int ret = getregs(false);
    if(ret)
        return;
    if(reg == "NA")
        return;
    iter = regslist.find(reg);
    if(iter != regslist.end())
        cout << reg << " = " << *iter->second << " (0x" << hex << *iter->second << dec << ")\n" << unitbuf;
    else
        cout << "** unknow register." << endl;
}

void setreg(string reg, string s_val){
    if(state != RUNNING)
    {
        cout << "** state must be RUNNING." << endl;
        return;
    }
    if(s_val == "NA")
        return;
    int ret = getregs(false);
    if(ret)
        return;
    long long val = strtoll(s_val.c_str(), NULL, 16);
    *regslist[reg] = val;
    if(reg == "rip")
        rip_now = val;
    ptrace(PTRACE_SETREGS, child, 0, &regs);
}

void vmmap(){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    ifstream f;
    f.open("/proc/" + to_string(child) + "/maps");
    string tmp;
    while(getline(f, tmp)){
        stringstream s(tmp);
        string col;
        s >> col;              
        size_t pos = col.find("-");
        while(pos != col.npos){
            col.replace(pos, 1, " ");
            pos = col.find("-");
        }
        stringstream addrs;
        string addr1, addr2;
        addrs << col;
        addrs >> addr1;
        addrs >> addr2;
        ios::fmtflags f(cout.flags());
        cout << setw(16) << setfill('0') << addr1 << "-" << setw(16) << setfill('0') << addr2 << "\t";
        cout.flags(f);
        s >> col; 
        col.erase(remove(col.begin(), col.end(), 'p')); 
        cout << col << "\t";  
        s >> col; s >> col;             
        s >> col; cout << col << dec <<"\t";  
        s >> col; cout << col << endl;  
    }
}

void dump(string s_addr){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    if(s_addr != "NO")
        dumpaddr = strtoll((s_addr.c_str()), NULL, 16);
    else{
        if(s_addr == "NA")
        {
            cout << "** no addr is given." << endl;
            return;
        }
    }
    string printable = "", bytes = "", content = ""; 
    int line = 5, print_cnt = 0;

    for(int i = 0; i < line; i++){ 
        cout << "\t" << hex << dumpaddr << dec << ": ";
        //cout << endl << line << " " << length << endl;
        bytes = ""; content = "|", printable = "";
        for(int j = 0; j < 2; j++){
            long long out = ptrace(PTRACE_PEEKTEXT, child, dumpaddr, 0);
            bytes += string((char*)&out, 8);
            dumpaddr += 8;
        }
        unsigned char* c = (unsigned char*)bytes.c_str();
        for(int i = 0; i < 16; i++){
            string tmp;
            stringstream ss;
            int val = int(*(c + i));
            ss << hex << setw(2) << setfill('0') << val;
            ss >> tmp;
            printable += tmp + " ";
            if(isprint(val))
                content += *(c + i);
            else
                content += ".";
            print_cnt++;
            
        }
        content += "|\n";
        printable += "\t" + content;
        cout << printable;
    }
    
}

void disasm(string s_addr, int len){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    if(s_addr == "NA"){
        cout << "** no addr is given." << endl;
        return;
    }
    if(code == NULL)
        getcode();
    long long addr = strtoll((s_addr.c_str()), NULL, 16);
    long long end = text.sh_addr + text.sh_size;
    for(int i = 0; i < len; i++){
        if(addr >= end){
            cout << "** the address is out of the range of the text segment" << endl;
            break;
        }
        long long offset = addr + text.sh_offset - text.sh_addr;
        char* pos = code + offset;
        cout << disassemble(pos, &addr,256);
    }
}

void start(){
    if(state == UNDEFINE){
        cout << "** not in loaded state." << endl;
        return;
    }
    else if(state == RUNNING){
        kill(child, SIGTERM);
        child = 0;
    }
    points.clear();
    bpid = 0;
    child = fork();
    if(child < 0)
        error_ocr("** fork error.");
    else if(child == 0){
        if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
            error_ocr("** ptrace error.");
        char* const argv[] = {NULL};
        if(program.c_str()[0] != '.' && program.c_str()[1] != '/'){
            string tmp = "./" + program;
            program = tmp;
        }
        execvp(program.c_str(), argv);
        error_ocr("** execvp error.");
    }
    else{
        int status;
        if(waitpid(child, &status, 0) < 0)
            error_ocr("** wait error.");
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL);          

        cout << "** pid " << child << endl;
        state = RUNNING;
    }
}

void run(){
    if(state == RUNNING){
        cout << "** program " << program << " is already running." << endl;
        cont();
        return;
    }
    else if(state == LOADED){
        start();
        cont();
    }
    else{
        cout << "** state must be LOADED or RUNNING" << endl;
        return;
    }
}

void cont(){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    ptrace(PTRACE_CONT, child, 0, 0);
    int ret = check_bp();
    if(ret == 1){
        long long addr = bpaddr_now;
        code_change(addr, restore);
        ptrace(PTRACE_CONT, child, 0, 0);
        code_change(addr, 0xcc);
        bpaddr_now = 0;
        check_bp();
        code_change(addr, 0xcc);
    }
    
}

void si(){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    ptrace(PTRACE_SINGLESTEP, child, 0, 0);
    int ret = check_bp();
    if(ret == 1){
        long long addr = bpaddr_now;
        code_change(addr, restore);
        ptrace(PTRACE_SINGLESTEP, child, 0, 0);
        bpaddr_now = 0;
        check_bp();
        code_change(addr, 0xcc);
    }
}

void list(){
    for(int i = 0; i < int(points.size()); i++)
        cout << "  " << points[i].id << ":  " << hex << points[i].addr << dec << endl;
}

void breakpoint(string s_addr){
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    long long addr = strtoll((s_addr.c_str()), NULL, 16);
    char original = code_change(addr, 0xcc);
    for(int i = 0; i < int(points.size()); i++)
        if(points[i].addr == addr)
            return;
    breakpoint_t tmp;
    tmp.id = bpid++;
    tmp.addr = addr;
    tmp.s_addr = s_addr;
    tmp.original = original;
    points.push_back(tmp);
}

void del(string s_id) {
    if(state != RUNNING){
        cout << "** state must be RUNNING." << endl;
        return;
    }
    if(s_id == "NA")
        return;
    int id = stoi(s_id);
    int id_tmp = 0, f = 0;
    for(int i = 0; i < int(points.size()); i++){
        if(points[i].id == id){
            code_change(points[i].addr, points[i].original);
            id_tmp = id;
            f = 1;
            bpid--;
            continue;
            //points.erase(points.begin() + i);
            //cout << "** breakpoint " << s_id << " deleted." << endl;
        }
        if(f){
            points[i].id--;
        }
    }
    if(f){
        points.erase(points.begin() + id_tmp);
        cout << "** breakpoint " << s_id << " deleted." << endl;
    }
    else 
        cout << "** no such breakpoint." << endl;
}

void quit_exit(){
    if(child != 0)
        kill(child, SIGTERM);
    elf_close(e);
}

int main(int argc, char** argv){
    setbuf(stdout,nullptr);
    init_regs_map();
    bool eof_flag = false;
    getargs(argc, argv);

    while(1){
        if(!file_flag)
            cout << "sdb> ";
        string line;
        if(file_flag){
            if(!getline(fin, line))
                eof_flag = true;
        }
        else{
            if(!getline(cin, line))
                eof_flag = true;
        }
        if(eof_flag)
            break;
        cmd = getcmd(line);


        if(cmd.empty())
            continue;
        else if(cmd[0] == "load"){
            if(cmd.size() == 2)
                load(cmd[1].c_str());
            else
                load("NA");
        }
        else if(cmd[0] == "delete"){
            if(cmd.size() == 2)
                del(cmd[1]);
            else
                del("NA");
        }   
        else if((cmd[0] == "help" || cmd[0] == "h"))
            help();
        else if((cmd[0] == "exit" || cmd[0] == "q"))
            break;
        else if((cmd[0] == "vmmap" || cmd[0] == "m"))
            vmmap();
        else if((cmd[0] == "get" || cmd[0] == "g")){
            if(cmd.size() == 2)
                getreg(cmd[1]);
            else
                getreg("NA");
        }
        else if((cmd[0] == "disasm" || cmd[0] == "d")){
            if(cmd.size() == 2)
                disasm(cmd[1], 10);
            else if(cmd.size() == 1)
                disasm("NA", 10);   
        }    
        else if((cmd[0] == "set" || cmd[0] == "s")){
            if(cmd.size() == 3)
                setreg(cmd[1], cmd[2]);
            else
                setreg("NA", "NA");
        }
        else if((cmd[0] == "break" || cmd[0] == "b")){
            if(cmd.size() == 2)
                breakpoint(cmd[1]);
            else
                breakpoint("NA");
        }        
        else if((cmd[0] == "list" || cmd[0] == "l"))
            list();
        else if((cmd[0] == "cont" || cmd[0] == "c"))
            cont();
        else if((cmd[0] == "dump" || cmd[0] == "x")){
            if(cmd.size() == 2)
                dump(cmd[1]);
            else if(cmd.size() == 1){
                if(dumpaddr != -1)
                    dump("NO");
                else
                    dump("NA");     
            }
        }
        else if(cmd[0] == "getregs" && cmd.size() == 1)
            getregs(true);
        else if(cmd[0] == "start" && cmd.size() == 1)
            start();
        else if(cmd[0] == "run" && cmd.size() == 1)
            run();
        else if(cmd[0] == "si" && cmd.size() == 1)
            si();
        else 
            cout<<"no such command "<<cmd[0]<<" "<<endl;
        
        cmd.clear();
    }

    quit_exit();
    return 0;
}