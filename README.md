# Compiler for subset of C language

This project includes the code for a compiler built from scratch for a subset of C language. The grammar of the language is as shown below:

```
<program>         : class <id> '{' <field_decl>* <method_decl>* '}'

<field_decl>      : <type> { <id> | <id> '[' <int_literal> ']' }+, ;

<method_decl>     : { <type> | void } <id> ( [{<type> <id>}+,] ) <block>

<block>           : '{' <var_decl>* <statement>* '}'

<var_decl>        : <type> <id>+, ;

<type>            : int | boolean | char

<statement>       : <location> <assign_op> <expr> ;
                  | <method_call> ;
                  | if ( <expr> ) <block> [else <block>]
                  | for <id> = <expr> , <expr> <block>
                  | return [<expr>] ;
                  | break ;
                  | continue ;
                  | <block>
                
<assign_op>       : = | += | -=

<method_call>     : <id> ( [<expr>+,] )
                  | callout ( <string_literal> [, <callout_arg>+,] )
                
<location>        : <id> | <id> ‘[’ <expr> ‘]’

<expr>            : <location> | <method_call> | <literal>
                  | <expr> <bin_op> <expr>
                  | - <expr> | ! <expr> | ( <expr> )

<callout_arg>     : <expr> | <string_literal>

<bin_op>          : <arith_op> | <rel_op> | <eq_op> | <cond_op>

<arith_op>        : + | - | * | / | %
<rel_op>          : > | < | >= | <=
<eq_op>           : == | !=
<cond_op>         : && | ||

<literal>         : <int_literal> | <char_literal> | <bool_literal>

<id>              : <alpha> <alpha_num>*

<alpha>           : a | b | c | … | z | A | B | C | … | Z
<aplha_num>       : <alpha> | <digit>

<digit>           : 0 | 1 | … | 9

<int_literal>     : <decimal_literal> | <hex_literal>

<decimal_literal> : <digit> <digit>*

<hex_literal>     : 0x <hex_digit> <hex_digit>*

<hex_digit>       : <digit> | a | b | c | d | e | f | A | B | C | D | E | F

<bool_literal>    : true | false

<char_literal>    : ‘ <char> ’

<string_literal>  : “ <char>* ”
```


__Project Description__: The project uses Bison and Flex for scanning, lexing and parsing to generate an Abstract Syntax Tree. The AST generation code is designed using Visitor Design Pattern. Once the AST is generated, each node value is evaluated with LLVM Intermediate Representation (IR) builder and converted to equivalent IR. The IR is converted to LLVM Bitcode and written to an output file.
The Bitcode is compiled into an object file using the `llc` tool provided by LLVM. The binary is generated using gcc/g++.

## How To Run
(This code has been run and tested on Linux only)

First of all, you need to build LLVM v3.6.2 from source. The source can be obtained from [here](http://releases.llvm.org/3.6.2/llvm-3.6.2.src.tar.xz). Extract the archive in $LOCATION directory and follow the steps given in the commands:

```
tar -xvf llvm-3.6.2.src.tar.xz -C $LOCATION
cd $LOCATION/llvm-3.6.2.src
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

If there are any required dependencies that cause errors in CMake, please install them and run the `cmake` command again.
Once the `make` command successfully completes, run `sudo make install` to install the binaries, static libraries and includes to `/usr/local`.
If you need to change the prefix where it installs, you can add the flag `-DCMAKE_INSTALL_PREFIX=PATH:$PREFIX` to your `cmake` command, where $PREFIX is the path where you want to install.

Once LLVM v3.6.2 is installed (assuming you have gcc v4.8.x (<= 4.9) installed on your system), you can follow the steps given below to compile the project:

```
git clone https://github.com/dracarys983/Compiler.git
cd Compiler
make
```

This should compile the sources and generate a binary named `decaf`. To test the compiler, run the following commands:

```
./decaf tests/Test_x                # x = 0/1/2. This will generate a Bitcode file.
llc -filetype=obj tests/Test_x.bc   # This will generate the object code.
gcc tests/Test_x.o                  # This will generate the binary a.out
./a.out                             # Run the program
```

Stay tuned for more test examples and extensions to the compiler so that complex constructs can be used.

__NOTE__: The code requires LLVM v3.6.2 and gcc (<= v4.9, preferably 4.8.x) to run without any modifications. The code will be updated with adaptations with latest versions of these libraries soon.

__*If anyone is interested in contributing, please open an issue and submit a pull request. All contributions welcome!*__
