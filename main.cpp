#include "head.h"
#include "stdllvm.h"
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
    Builder = new IRBuilder<>(getGlobalContext());
    DecafToLLVM = new Module("DecafToLLVM", getGlobalContext());

    yyparse();
    DecafToLLVM->dump();

    return 0;
}
