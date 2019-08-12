/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "gram.y" /* yacc.c:339  */

#include <cstdio>
#include <iostream>
#include <string>
#include <stack>
#include <map>
#include <vector>
#include "tree.h"
#include "semantic.h"

#define YYDEBUG 1 
using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
extern "C" int yyparse();
extern "C" int line;
extern "C" FILE *yyin;
 
void yyerror(const char *s);

struct symbol {
	int value; //if symbol is a function, value is the number of parameters; if var, it's the int value
	string type;   // void or int
	string nature; // vector or variable or function
	std::vector <std::string> argTypes;
};

// Vector of tree Nodes
typedef std::vector <Node*> nodeVector;

// individual scope symbol table
typedef  std::map <std::string, symbol> symbolTable;

typedef std::stack <symbolTable> scopeStack;


// Empty scope stack
stack<symbolTable> scopes;

// Auxiliary functions(move to external include file?):


symbol simpleLookup(scopeStack stack, string name) {
	// iterate 	over scopes
	scopeStack copyStack = stack;
	for(std::vector<int>::size_type i = 0; i <= copyStack.size(); i++){
		// Iterate over symbols
		symbolTable table = copyStack.top();
		for(auto it = table.begin(); it != table.end(); ++it ){
	      	// Success if name and number of arguments match a previously declared function
	      	// missing argument type check
	      	if(name == it->first){
				return it->second;
	      	};
	      
	    }
		if(i!= 0){copyStack.pop(); i--;}
	}
	exit(-1);
}

int lookupLegalFunction (scopeStack stack, string name, int argNum) {
	// Iterate over scopes
	scopeStack copyStack = stack;
	for(std::vector<int>::size_type i = 0; i <= copyStack.size(); i++){
		// Iterate over symbols
		symbolTable table = copyStack.top();
		for(auto it = table.begin(); it != table.end(); ++it ){
	      	// Success if name and number of arguments match a previously declared function
	      	// ----> missing argument type check
	      	if(name == it->first && argNum == it->second.value ){
				return 1;
	      	};
	      
	    }
		if(i!= 0){copyStack.pop(); i--;}
	}
	return 0;
}

int lookupSymbolInScope (symbolTable table, string name, symbol entry) {
	// Iterate over symbols
	for(auto it = table.begin(); it != table.end(); ++it ){
      if(name == it->first) {return 1;};
    }

	return 0;
}


string getLeafId (Node* node) {
	if(node->Children.size() == 1) {return node->Children[0]->Data;}
	if(node->Children.size() != 0) {getLeafId (node->Children[0]);}
}


scopeStack buildScope(scopeStack scopes, Node *node){
	scopeStack modScopes = scopes;
	// cout << "test" << endl;
	if(!node) return modScopes;
	string token = node->Data;
	// cout << token << endl;
	if(token == "program"){
		// initial "global" scope
		symbolTable newTable;
		std::vector <std::string> printArgs;
		printArgs.push_back("int");
		symbol printSymbol = {1,"void","function", printArgs};
		symbol inputSymbol = {0,"int","function"};
		
		newTable.insert(std::pair<std::string, symbol>("println", printSymbol));
		newTable.insert(std::pair<std::string, symbol>("input", inputSymbol));
		
		modScopes.push(newTable);
		// cout << "initial scope created" << endl;
		Node* lastDeclaration;
		for(std::vector<int>::size_type i = 0; i != node->Children.size(); i++){
			modScopes = buildScope(modScopes, node->Children[i]);
			lastDeclaration = node->Children[i];
		}
		checkSemantics("last_fun_is_main", lastDeclaration);
		// printTable(modScopes);
		return modScopes;
	}

	else if(token == "var-declaration"){
		// set variable initially (e.g. 'int g;')
		checkSemantics("is_var_type_int", node);
		symbol newEntry;
		newEntry.type = node->Children[0]->Data; // type
		string name = node->Children[1]->Data;
		// Decide if common variable or vector
		if (node->Children.size() > 2){
			newEntry.nature = "vector";
		}
		else {
			newEntry.nature = "variable";
		}

		// Exit program if symbol already defined, else insert symbol to 
		if(lookupSymbolInScope(modScopes.top(), name, newEntry)){exit(-1);};
		modScopes.top().insert(std::pair<std::string, symbol>(name, newEntry));
		return modScopes;
	}

	else if(token == "fun-declaration"){
		checkSemantics("is_return_valid", node);
		symbol newEntry;
		newEntry.type = node->Children[0]->Data; // type
		string name = node->Children[1]->Data;
		newEntry.nature = "function";
		// Build scope for content of function
		for(std::vector<int>::size_type i = 0; i != node->Children.size(); i++){
			if (node->Children[i]->Data == "params"){
				// function entry in table will be the number of parameters
				newEntry.value = node->Children[i]->Children.size(); 
				for(std::vector<int>::size_type j = 0; j != newEntry.value; j++) {
					string argType = node->Children[i]->Children[j]->Children[0]->Data;
					newEntry.argTypes.push_back(argType);
				}
				if(modScopes.top().count(name) == 0){
					modScopes.top().insert(std::pair<std::string, symbol>(name, newEntry));
				}
				else{
					exit(-1);
				}
				 
			}
			modScopes = buildScope(modScopes, node->Children[i]);
		}
		// insert function scope
		return modScopes;
	}

	else if(token == "params"){
		for(std::vector<int>::size_type i = 0; i != node->Children.size(); i++) {
	    	 modScopes = buildScope(modScopes, node->Children[i]);
		}
	}

	else if(token == "param"){
		symbol newEntry;
		newEntry.type = node->Children[0]->Data; // type
		string name = node->Children[1]->Data;
		modScopes.top().insert(std::pair<std::string, symbol>(name, newEntry));
		return modScopes;
	}

	else if(token == "compound-stmt"){
		symbolTable newTable;
		modScopes.push(newTable);
		
		for(std::vector<int>::size_type i = 0; i != node->Children.size(); i++) {
	    	 modScopes = buildScope(modScopes, node->Children[i]);
		}
		
		modScopes.pop();
		return modScopes;
	}
	else if(token == "selection-stmt"){
		modScopes = buildScope(modScopes, node->Children[1]);
		return modScopes;
	}
	else if(token == "return-stmt"){
		if(node->Children.size() != 0) modScopes = buildScope(modScopes, node->Children[0]);
		return modScopes;
	}
	else if(token == "iteration-stmt"){
		// If condition is function, check return type; exit if void
		if(node->Children[0]->Data == "call"){
			string funcName = node->Children[0]->Children[0]->Data;
			symbol function = simpleLookup(modScopes, funcName);
			if(function.type == "void"){exit(-1);};
		}
		modScopes = buildScope(modScopes, node->Children[0]);
		return modScopes;
	}
	else if(token == "call"){
		string name = node->Children[0]->Data;
		int argNum = node->Children[1]->Children.size();
		//check if call is legal (if there is a function defined with its name and number of args)
		if(!lookupLegalFunction(modScopes, name, argNum)){exit(-1);};

		symbol calledFunc = simpleLookup(modScopes, name);
		// check if argument types match function definition
		for(std::vector<int>::size_type i = 0; i != node->Children[1]->Children.size(); i++) {
			Node* passedArg = node->Children[1]->Children[i];
			// check if argument is literal int
	    	string passedArgType;
	    	if ((isdigit(passedArg->Data[0]))){
				passedArgType = "int";
		  	}
		  	else {
		  		passedArgType = (simpleLookup(modScopes, passedArg->Children[0]->Data)).type;
		  	}
		  	// If types dont 
	    	if(calledFunc.argTypes[i] != passedArgType){exit(-1);}
		}
		return modScopes;

	}

	else if(token == "=") {
		Node* left = node->Children[0];
		Node* right = node->Children[1];

		// Treat left node
		// If literal integer or function call on left side, error
		if(isdigit(left->Data[0]) || left->Data == "call"){exit(-1);}
		// else, lookup for valid variable (declared, not function)
		symbol leftSide = simpleLookup(modScopes, left->Children[0]->Data);
		if(leftSide.nature == "function"){exit(-1);}


		// Treat right node
		for(std::vector<int>::size_type i = 0; i != right->Children.size(); i++) {
			// If terminal
			if(node->Children[i]->Children.size() <= 1){
				Node* term;
				switch(node->Children[i]->Children.size()){
					case 0: term = node->Children[i]; break;
					case 1: term = node->Children[i]->Children[0];break;
				}
				// If not integer literal
				if(!isdigit(node->Children[i]->Data[0])){
					// lookup type and existance 
					symbol terminalSymbol = simpleLookup(modScopes, term->Data);
					if(terminalSymbol.type == "void"){exit(-1);}
					if(terminalSymbol.nature == "vector"){exit(-1);}
					if(terminalSymbol.nature == "function"){exit(-1);}
				}	
				
				// If literal, just return
				else {
					// cout << node->Children[i]->Data;
					return modScopes;
				}			
			}

			// not terminal or func call
			else {
				// func call
				if(node->Children[i]->Data == "call"){
					// cout << node->Children[i]->Children.size() << endl;
					symbol funcSymbol = simpleLookup(modScopes, node->Children[i]->Children[0]->Data);
					if(funcSymbol.type == "void"){exit(-1);}
				}
				// really not terminal
				else {
					modScopes = buildScope(modScopes, node->Children[i]);
				}
			};
		}

		return modScopes;








		// int is_Rhs_int = 1;
		// string rightHandSideName, leftHandSideName;
		// symbol entryLeft, entryRight;
		// // LHS is literal int, error
		// if(isdigit(node->Children[0]->Data[0])){exit(-1);}
		// // LHS is a function, error 
		// // cout << node->Children[1]->Data;
		// if(node->Children[0]->Data == "call"){exit(-1);}

		// leftHandSideName = node->Children[0]->Children[0]->Data;
		// entryLeft = simpleLookup(modScopes, leftHandSideName);
		// if(entryLeft.nature == "vector"){modScopes = buildScope(modScopes, node->Children[0]->Children[1]);}
		// if(entryLeft.nature == "function"){exit(-1);}
		// // LHS and RHS are not literal int
		// if(!isdigit(node->Children[1]->Data[0])){
		// 	for(std::vector<int>::size_type i = 0; i != node->Children[1]->Children.size(); i++) {
	 //    	 	modScopes = buildScope(modScopes, node->Children[1]->Children[i]);
		// 		// cout << node->Children[1]->Children[i]->Children.size() << endl;
		// 		if(node->Children[1]->Children[i]->Children.size() == 0){
		// 			rightHandSideName = node->Children[1]->Children[i]->Data;
		// 			// cout << rightHandSideName;
		// 			entryRight = simpleLookup(modScopes, rightHandSideName);	
		// 			if(entryRight.type != "int"){is_Rhs_int = 0;}
		// 		}
		// 		// rightHandSideName = getLeafId(node->Children[1]->Children[i]);
		// 		// entryRight = simpleLookup(modScopes, rightHandSideName);
		// 	}
		// }
		// // RHS is literal int
		// else {is_Rhs_int = 1;}
		// if(entryLeft.type == "int" && !is_Rhs_int){exit(-1);}
		// return modScopes;
	}

	else if(token == "+" || token == "*" || token == "-" || token == "/")  {
		// Go to left-right children
		for(std::vector<int>::size_type i = 0; i != 2; i++) {
			// If terminal
			if(node->Children[i]->Children.size() <= 1){
				Node* term;
				switch(node->Children[i]->Children.size()){
					case 0: term = node->Children[i]; break;
					case 1: term = node->Children[i]->Children[0];break;
				}
				// If not integer literal
				if(!isdigit(node->Children[i]->Data[0])){
					// lookup type and existance 
					symbol terminalSymbol = simpleLookup(modScopes, term->Data);
					if(terminalSymbol.type == "void"){exit(-1);}
					if(terminalSymbol.nature == "vector"){exit(-1);}
					if(terminalSymbol.nature == "function"){exit(-1);}
				}	
				
				// If literal, just return
				else {
					// cout << node->Children[i]->Data;
					return modScopes;
				}			
			}

			// not terminal or func call
			else {
				// func call
				if(node->Children[i]->Data == "call"){
					// cout << node->Children[i]->Children.size() << endl;
					symbol funcSymbol = simpleLookup(modScopes, node->Children[i]->Children[0]->Data);
					if(funcSymbol.type == "void"){exit(-1);}
				}
				// really not terminal
				else {
					modScopes = buildScope(modScopes, node->Children[i]);
				}
			};
		}

		return modScopes;

	}

	else if(token == "var"){

	 	// Check if called variable/function is defined
	 	string calledID = node->Children[0]->Data;
	 	symbol entry = simpleLookup(modScopes, calledID);
	 	// Check if called variable is a vector, but called without [n]
	 	if(node->Children.size() == 1 && entry.nature == "vector"){exit(-1);}
	 	// Treat vector call
	 	if(node->Children.size() == 2){
	 		if(entry.nature != "vector"){exit(-1);}
	 		Node* innerNum = node->Children[1];
	 		// cout << innerNum->Data;
	 	}
	 	if(entry.nature == "function"){exit(-1);}	

	 }

	return modScopes;
}

void printNode (Node *node) {
	if (!node ) return;

	if(node->Data != ""){
		cout << "[" << node->Data;
	};
	for(std::vector<int>::size_type i = 0; i != node->Children.size(); i++) {
	    printNode(node->Children[i]);
	}
	if(node->Data != ""){cout << "]";};
}

nodeVector joinChildren(nodeVector A, nodeVector B){
	nodeVector AB;
	AB.reserve( A.size() + B.size() ); // preallocate memory
	AB.insert( AB.end(), A.begin(), A.end() );
	AB.insert( AB.end(), B.begin(), B.end() );
	return AB;
}




#line 493 "gram.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "gram.tab.h".  */
#ifndef YY_YY_GRAM_TAB_H_INCLUDED
# define YY_YY_GRAM_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    NUM = 259,
    T_NEWLINE = 260,
    T_IF = 261,
    T_ELSE = 262,
    T_INT = 263,
    T_RETURN = 264,
    T_VOID = 265,
    T_WHILE = 266,
    T_CLOSE = 267,
    T_OPEN = 268,
    T_NOTEQ = 269,
    T_ASSIGN = 270,
    T_SEMI = 271,
    T_COMMA = 272,
    T_OPENCOLCHETE = 273,
    T_CLOSECOLCHETE = 274,
    T_OPENCHAVE = 275,
    T_CLOSECHAVE = 276,
    T_GREATER = 277,
    T_LESSER = 278,
    T_NOTEQUAL = 279,
    T_GREEQ = 280,
    T_LESEQ = 281,
    T_EQUAL = 282,
    T_PLUS = 283,
    T_MINUS = 284,
    T_MUL = 285,
    T_DIV = 286
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 429 "gram.y" /* yacc.c:355  */

    int numval;
    char *idval;
    Node *node;

#line 571 "gram.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAM_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 588 "gram.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   99

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  64
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  103

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   472,   472,   475,   476,   479,   480,   483,   484,   486,
     487,   490,   492,   493,   496,   497,   500,   501,   504,   508,
     509,   512,   513,   516,   517,   518,   519,   520,   523,   524,
     527,   528,   531,   534,   535,   538,   539,   542,   543,   545,
     546,   548,   549,   550,   551,   552,   553,   554,   556,   557,
     559,   560,   563,   564,   566,   567,   570,   571,   572,   573,
     576,   579,   580,   582,   583
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ID", "NUM", "T_NEWLINE", "T_IF",
  "T_ELSE", "T_INT", "T_RETURN", "T_VOID", "T_WHILE", "T_CLOSE", "T_OPEN",
  "T_NOTEQ", "T_ASSIGN", "T_SEMI", "T_COMMA", "T_OPENCOLCHETE",
  "T_CLOSECOLCHETE", "T_OPENCHAVE", "T_CLOSECHAVE", "T_GREATER",
  "T_LESSER", "T_NOTEQUAL", "T_GREEQ", "T_LESEQ", "T_EQUAL", "T_PLUS",
  "T_MINUS", "T_MUL", "T_DIV", "$accept", "program", "declaration-list",
  "declaration", "var-declaration", "type-specifier", "fun-declaration",
  "params", "param-list", "param", "compound-stmt", "local-declarations",
  "statement-list", "statement", "expression-stmt", "selection-stmt",
  "iteration-stmt", "return-stmt", "expression", "var",
  "simple-expression", "relop", "additive-expression", "addop", "term",
  "mulop", "factor", "call", "args", "arg-list", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286
};
# endif

#define YYPACT_NINF -89

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-89)))

#define YYTABLE_NINF -14

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      23,   -89,   -89,     1,    23,   -89,   -89,    14,   -89,   -89,
     -89,    -6,    30,   -89,    17,    -8,    36,    35,    37,   -89,
      54,    41,    55,    23,    58,    57,   -89,   -89,   -89,   -89,
     -89,    23,   -89,    74,    21,    34,    -2,   -89,    65,    32,
      66,     2,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,
      64,    67,   -89,    43,    31,   -89,   -89,     2,     2,     2,
     -89,    68,     2,    69,   -89,     2,   -89,   -89,   -89,   -89,
     -89,   -89,   -89,   -89,   -89,     2,     2,   -89,   -89,     2,
     -89,    71,    70,    72,    73,   -89,    76,   -89,   -89,   -89,
       0,    31,   -89,   -89,     2,   -89,    40,    40,   -89,    79,
     -89,    40,   -89
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     9,    10,     0,     2,     4,     5,     0,     6,     1,
       3,     0,     0,     7,     0,    10,     0,     0,    12,    15,
       0,    16,     0,     0,     0,     0,    20,    11,    14,     8,
      17,    22,    19,     0,     0,     0,    37,    59,     0,     0,
       0,     0,    29,    18,    24,    21,    23,    25,    26,    27,
       0,    57,    36,    40,    49,    53,    58,    62,     0,     0,
      33,     0,     0,     0,    28,     0,    43,    47,    41,    42,
      44,    45,    46,    50,    51,     0,     0,    54,    55,     0,
      64,     0,    61,     0,     0,    34,     0,    56,    35,    57,
      39,    48,    52,    60,     0,    38,     0,     0,    63,    30,
      32,     0,    31
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -89,   -89,   -89,    85,    59,    -9,   -89,   -89,   -89,    75,
      77,   -89,   -89,   -88,   -89,   -89,   -89,   -89,   -39,   -12,
     -89,   -89,    18,   -89,    16,   -89,    15,   -89,   -89,   -89
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     5,     6,     7,     8,    17,    18,    19,
      44,    31,    34,    45,    46,    47,    48,    49,    50,    51,
      52,    75,    53,    76,    54,    79,    55,    56,    81,    82
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      61,     9,    63,    16,   -13,    36,    37,    12,    99,   100,
      13,    57,    14,   102,    16,    41,    58,    11,    80,    83,
      84,    20,    33,    86,    36,    37,    88,    38,    73,    74,
      39,     1,    40,     2,    41,    36,    37,    42,     1,    21,
      15,    26,    43,    36,    37,    41,    38,    22,    60,    39,
      13,    40,    14,    41,    23,    98,    42,    66,    67,    25,
      26,    77,    78,    89,    89,    68,    69,    89,    70,    71,
      72,    73,    74,    24,    29,    26,    30,    35,    59,    62,
      64,    87,    65,    93,    85,    96,   101,    94,    97,    10,
      32,    95,    91,    90,    92,     0,     0,     0,    28,    27
};

static const yytype_int8 yycheck[] =
{
      39,     0,    41,    12,    12,     3,     4,    13,    96,    97,
      16,    13,    18,   101,    23,    13,    18,     3,    57,    58,
      59,     4,    31,    62,     3,     4,    65,     6,    28,    29,
       9,     8,    11,    10,    13,     3,     4,    16,     8,     3,
      10,    20,    21,     3,     4,    13,     6,    12,    16,     9,
      16,    11,    18,    13,    17,    94,    16,    14,    15,    18,
      20,    30,    31,    75,    76,    22,    23,    79,    25,    26,
      27,    28,    29,    19,    16,    20,    19,     3,    13,    13,
      16,    12,    15,    12,    16,    12,     7,    17,    12,     4,
      31,    19,    76,    75,    79,    -1,    -1,    -1,    23,    22
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     8,    10,    33,    34,    35,    36,    37,    38,     0,
      35,     3,    13,    16,    18,    10,    37,    39,    40,    41,
       4,     3,    12,    17,    19,    18,    20,    42,    41,    16,
      19,    43,    36,    37,    44,     3,     3,     4,     6,     9,
      11,    13,    16,    21,    42,    45,    46,    47,    48,    49,
      50,    51,    52,    54,    56,    58,    59,    13,    18,    13,
      16,    50,    13,    50,    16,    15,    14,    15,    22,    23,
      25,    26,    27,    28,    29,    53,    55,    30,    31,    57,
      50,    60,    61,    50,    50,    16,    50,    12,    50,    51,
      54,    56,    58,    12,    17,    19,    12,    12,    50,    45,
      45,     7,    45
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    32,    33,    34,    34,    35,    35,    36,    36,    37,
      37,    38,    39,    39,    40,    40,    41,    41,    42,    43,
      43,    44,    44,    45,    45,    45,    45,    45,    46,    46,
      47,    47,    48,    49,    49,    50,    50,    51,    51,    52,
      52,    53,    53,    53,    53,    53,    53,    53,    54,    54,
      55,    55,    56,    56,    57,    57,    58,    58,    58,    58,
      59,    60,    60,    61,    61
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     3,     6,     1,
       1,     6,     1,     1,     3,     1,     2,     4,     4,     2,
       0,     2,     0,     1,     1,     1,     1,     1,     2,     1,
       5,     7,     5,     2,     3,     3,     1,     1,     4,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     1,
       4,     1,     0,     3,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 472 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[0].node)->Children}; (yyval.node) = new Node("program",Children); buildScope(scopes, (yyval.node)); printNode((yyval.node));}
#line 1739 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 475 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = (yyvsp[-1].node)->Children; Children.push_back((yyvsp[0].node)); (yyval.node) = new Node("declaration-list", Children);}
#line 1745 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 476 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[0].node)};(yyval.node) = new Node("declaration-list", Children);}
#line 1751 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 479 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = (yyvsp[0].node);}
#line 1757 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 480 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = (yyvsp[0].node);}
#line 1763 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 483 "gram.y" /* yacc.c:1646  */
    {nodeVector Children ={(yyvsp[-2].node), new Node((yyvsp[-1].idval))} ;(yyval.node) = new Node("var-declaration", Children);}
#line 1769 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 484 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-5].node), new Node((yyvsp[-4].idval)), new Node(std::to_string((yyvsp[-2].numval)))}; (yyval.node) = new Node("var-declaration", Children);}
#line 1775 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 486 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("int");}
#line 1781 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 487 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("void");}
#line 1787 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 490 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-5].node), new Node((yyvsp[-4].idval)), (yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node("fun-declaration", Children);}
#line 1793 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 492 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[0].node)->Children}; (yyval.node) = new Node("params", Children);}
#line 1799 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 493 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("params");}
#line 1805 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 496 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = (yyvsp[-2].node)->Children; Children.push_back((yyvsp[0].node)); (yyval.node) = new Node("param-list", Children);}
#line 1811 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 497 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[0].node)};(yyval.node) = new Node("param-list", Children);}
#line 1817 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 500 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-1].node), new Node((yyvsp[0].idval))}; (yyval.node) = new Node("param", Children);}
#line 1823 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 501 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-3].node), new Node((yyvsp[-2].idval)), new Node("\\[\\]")};(yyval.node) = new Node("param", Children);}
#line 1829 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 504 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = joinChildren((yyvsp[-2].node)->Children, (yyvsp[-1].node)->Children); (yyval.node) = new Node("compound-stmt",Children);}
#line 1835 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 508 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = (yyvsp[-1].node)->Children; Children.push_back((yyvsp[0].node)); (yyval.node) = new Node("local-declarations", Children);}
#line 1841 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 509 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("");}
#line 1847 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 512 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = (yyvsp[-1].node)->Children; Children.push_back((yyvsp[0].node)); (yyval.node) = new Node("statement-list", Children);}
#line 1853 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 513 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("");}
#line 1859 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 523 "gram.y" /* yacc.c:1646  */
    {}
#line 1865 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 524 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node(";");}
#line 1871 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 527 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node("selection-stmt", Children);}
#line 1877 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 528 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node("selection-stmt", Children);}
#line 1883 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 531 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node("iteration-stmt", Children);}
#line 1889 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 534 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("return-stmt");}
#line 1895 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 535 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-1].node)}; (yyval.node) = new Node("return-stmt", Children);}
#line 1901 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 538 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node("=", Children);}
#line 1907 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 542 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {new Node((yyvsp[0].idval))}; (yyval.node) = new Node("var", Children);}
#line 1913 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 543 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {new Node((yyvsp[-3].idval)), (yyvsp[-1].node)}; (yyval.node) = new Node("var", Children);}
#line 1919 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 545 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node((yyvsp[-1].node)->Data, Children);}
#line 1925 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 548 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node(">");}
#line 1931 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 549 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("<");}
#line 1937 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 550 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("!=");}
#line 1943 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 551 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node(">=");}
#line 1949 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 552 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("<=");}
#line 1955 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 553 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("==");}
#line 1961 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 554 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("=");}
#line 1967 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 556 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node((yyvsp[-1].node)->Data, Children);}
#line 1973 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 559 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("+");}
#line 1979 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 560 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("-");}
#line 1985 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 563 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[-2].node), (yyvsp[0].node)}; (yyval.node) = new Node((yyvsp[-1].node)->Data, Children);}
#line 1991 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 566 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("*");}
#line 1997 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 567 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("/");}
#line 2003 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 570 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = (yyvsp[-1].node);}
#line 2009 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 571 "gram.y" /* yacc.c:1646  */
    {}
#line 2015 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 572 "gram.y" /* yacc.c:1646  */
    {}
#line 2021 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 573 "gram.y" /* yacc.c:1646  */
    {if((yyvsp[0].numval) >= 2147483648 || (yyvsp[0].numval)<=-2147483648){exit(-1);} ;(yyval.node) = new Node(std::to_string((yyvsp[0].numval)));}
#line 2027 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 576 "gram.y" /* yacc.c:1646  */
    {nodeVector Children ={new Node((yyvsp[-3].idval)), (yyvsp[-1].node)}; (yyval.node) = new Node("call", Children);}
#line 2033 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 579 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[0].node)->Children}; (yyval.node) = new Node("args", Children);}
#line 2039 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 580 "gram.y" /* yacc.c:1646  */
    {(yyval.node) = new Node("args");}
#line 2045 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 582 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = (yyvsp[-2].node)->Children; Children.push_back((yyvsp[0].node)); (yyval.node) = new Node("arg-list", Children);}
#line 2051 "gram.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 583 "gram.y" /* yacc.c:1646  */
    {nodeVector Children = {(yyvsp[0].node)}; (yyval.node) = new Node("arg-list", Children);}
#line 2057 "gram.tab.c" /* yacc.c:1646  */
    break;


#line 2061 "gram.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 586 "gram.y" /* yacc.c:1906  */


int main(int argc, char *argv[]) {

	FILE *file_in = fopen(argv[1], "r");
    yyin = file_in;
	if(argv[2]){freopen(argv[2], "w", stdout);} /* redirect printf to output file */

    // parsestrdup through the input until there is no more:
	do {
		yyparse();
	} while (!feof(yyin));
	
	fclose(file_in);
    return 0;
}

void yyerror(const char *s) {
	// printf("Error: %s in line: %d \n", s,line);	
	exit(-1);
}
