#include <vector>
#include <string>
#include <string.h>
#include "elftool.hpp"
using namespace std;

typedef enum
{
    LOADED,
    RUNNING,
    UNDEFINE
} state_t;

typedef enum
{
    ARGS,
    SCRIPT
} cmd_source_t;

typedef struct breakpoint
{
    int id;
    long long addr;
    string s_addr;
    char original;
} breakpoint_t;

vector<string> getcmd(string);
void   getargs(int argc, char**);
void   error_ocr(string);
void   init_regs_map();
char   replace_code(long long, char);
int    check_bp();
string disassemble(char*, long long, int);
void   getcode();


//sgd instruction
void list();                        
void load(const char*);    
void run();                         
void vmmap();                       
void setreg(string, string); 
void si();                          
void start();        
void breakpoint(string);             
void cont();                        
void del(string);              
void disasm(string, int);
void dump(string);           
void quit_exit();                   
void getreg(string);            
int  getregs(bool);       
void help();                        
               