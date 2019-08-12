%{
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



%}


%union {
    int numval;
    char *idval;
    Node *node;
}

%token <idval> ID
%token <numval> NUM

%token T_NEWLINE

%token T_IF
%token T_ELSE
%token T_INT
%token T_RETURN
%token T_VOID
%token T_WHILE


%token T_CLOSE
%token T_OPEN
%token T_NOTEQ
%token T_ASSIGN
%token T_SEMI
%token T_COMMA
%token T_OPENCOLCHETE
%token T_CLOSECOLCHETE
%token T_OPENCHAVE
%token T_CLOSECHAVE



%left  T_GREATER T_LESSER T_ASSIGN T_NOTEQUAL T_GREEQ T_LESEQ T_EQUAL
%left  T_PLUS T_MINUS
%left  T_MUL T_DIV

%type <node> factor additive-expression relop addop mulop term simple-expression var expression 
%type <node> expression-stmt selection-stmt iteration-stmt return-stmt statement statement-list
%type <node> local-declarations compound-stmt param param-list params fun-declaration type-specifier
%type <node> var-declaration program declaration-list declaration call args arg-list
%%

// Default action = {$$=$1}
program : declaration-list {nodeVector Children = {$1->Children}; $$ = new Node("program",Children); buildScope(scopes, $$); printNode($$);}
;

declaration-list : declaration-list declaration {nodeVector Children = $1->Children; Children.push_back($2); $$ = new Node("declaration-list", Children);}
	| declaration {nodeVector Children = {$1};$$ = new Node("declaration-list", Children);}
;

declaration : var-declaration {$$ = $1;} 
	| fun-declaration {$$ = $1;}
;

var-declaration : type-specifier ID T_SEMI {nodeVector Children ={$1, new Node($2)} ;$$ = new Node("var-declaration", Children);}
	| type-specifier ID T_OPENCOLCHETE NUM T_CLOSECOLCHETE T_SEMI {nodeVector Children = {$1, new Node($2), new Node(std::to_string($4))}; $$ = new Node("var-declaration", Children);}
;
type-specifier : T_INT {$$ = new Node("int");} 
	| T_VOID  {$$ = new Node("void");}
;

fun-declaration : type-specifier ID T_OPEN params T_CLOSE compound-stmt {nodeVector Children = {$1, new Node($2), $4, $6}; $$ = new Node("fun-declaration", Children);}
;
params : param-list {nodeVector Children = {$1->Children}; $$ = new Node("params", Children);}
	| T_VOID  {$$ = new Node("params");}
;

param-list : param-list T_COMMA param{nodeVector Children = $1->Children; Children.push_back($3); $$ = new Node("param-list", Children);}
	| param {nodeVector Children = {$1};$$ = new Node("param-list", Children);}
;

param : type-specifier ID {nodeVector Children = {$1, new Node($2)}; $$ = new Node("param", Children);} 
	| type-specifier ID T_OPENCOLCHETE T_CLOSECOLCHETE {nodeVector Children = {$1, new Node($2), new Node("\\[\\]")};$$ = new Node("param", Children);} 
;

compound-stmt : T_OPENCHAVE local-declarations statement-list T_CLOSECHAVE {nodeVector Children = joinChildren($2->Children, $3->Children); $$ = new Node("compound-stmt",Children);}
;


local-declarations : local-declarations var-declaration {nodeVector Children = $1->Children; Children.push_back($2); $$ = new Node("local-declarations", Children);}
	| {$$ = new Node("");}
;

statement-list : statement-list statement {nodeVector Children = $1->Children; Children.push_back($2); $$ = new Node("statement-list", Children);}
	| {$$ = new Node("");}
;

statement : expression-stmt 
	| compound-stmt 
	| selection-stmt 
	| iteration-stmt 
	| return-stmt 
;

expression-stmt : expression T_SEMI  {}
	| T_SEMI {$$ = new Node(";");}
;

selection-stmt : T_IF T_OPEN expression T_CLOSE statement {nodeVector Children = {$3, $5}; $$ = new Node("selection-stmt", Children);}
  	| T_IF T_OPEN expression T_CLOSE statement T_ELSE statement {nodeVector Children = {$3, $5, $7}; $$ = new Node("selection-stmt", Children);}
;

iteration-stmt : T_WHILE T_OPEN expression T_CLOSE statement {nodeVector Children = {$3, $5}; $$ = new Node("iteration-stmt", Children);}
;
	
return-stmt : T_RETURN T_SEMI {$$ = new Node("return-stmt");} 
	| T_RETURN expression T_SEMI{nodeVector Children = {$2}; $$ = new Node("return-stmt", Children);}
;

expression : var T_ASSIGN expression {nodeVector Children = {$1, $3}; $$ = new Node("=", Children);}
	| simple-expression
;

var: ID {nodeVector Children = {new Node($1)}; $$ = new Node("var", Children);} 
	| ID T_OPENCOLCHETE expression T_CLOSECOLCHETE {nodeVector Children = {new Node($1), $3}; $$ = new Node("var", Children);} // declaração de vetor ex: a[5]
;
simple-expression : additive-expression relop additive-expression {nodeVector Children = {$1, $3}; $$ = new Node($2->Data, Children);}
  	| additive-expression 
;	
relop:  T_GREATER {$$ = new Node(">");}
	| T_LESSER {$$ = new Node("<");}
	| T_NOTEQ {$$ = new Node("!=");}
	| T_GREEQ {$$ = new Node(">=");}
	| T_LESEQ {$$ = new Node("<=");}
	| T_EQUAL {$$ = new Node("==");}
	| T_ASSIGN {$$ = new Node("=");}
;
additive-expression: additive-expression addop term {nodeVector Children = {$1, $3}; $$ = new Node($2->Data, Children);};
	| term
;
addop: T_PLUS {$$ = new Node("+");} 
	| T_MINUS {$$ = new Node("-");} 
;

term : term mulop factor {nodeVector Children = {$1, $3}; $$ = new Node($2->Data, Children);} 
	| factor
;
mulop : T_MUL {$$ = new Node("*");} 
	| T_DIV {$$ = new Node("/");}
;

factor: T_OPEN expression T_CLOSE {$$ = $2;}
	| var {}
	| call {}
	| NUM {if($1 >= 2147483648 || $1<=-2147483648){exit(-1);} ;$$ = new Node(std::to_string($1));}
;

call : ID T_OPEN args T_CLOSE {nodeVector Children ={new Node($1), $3}; $$ = new Node("call", Children);}
;

args : arg-list {nodeVector Children = {$1->Children}; $$ = new Node("args", Children);} 
	| {$$ = new Node("args");}
;
arg-list : arg-list T_COMMA expression {nodeVector Children = $1->Children; Children.push_back($3); $$ = new Node("arg-list", Children);} 
	| expression {nodeVector Children = {$1}; $$ = new Node("arg-list", Children);}
;

%%

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
