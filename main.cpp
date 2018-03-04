#include "include/head.h"
#include "include/stdllvm.h"
#include <fstream>
using namespace llvm;

extern Module *DecafToLLVM;
extern IRBuilder<> *Builder;

// prototype of bison-generated parser function
int yyparse();

int main(int argc, char **argv)
{
    if ((argc > 1) && (freopen(argv[1], "r", stdin) == NULL))
    {
        cerr << argv[0] << ": File " << argv[1] << " cannot be opened.\n";
        exit( 1 );
    }
    string fname(argv[1]);
    fname += ".bc";
    error_code EC;
    raw_fd_ostream bc_file(StringRef(fname.c_str()), EC, llvm::sys::fs::F_None);
    Builder = new IRBuilder<>(getGlobalContext());
    DecafToLLVM = new Module("DecafToLLVM", getGlobalContext());

    yyparse();
    WriteBitcodeToFile(DecafToLLVM, bc_file);

    return 0;
}
