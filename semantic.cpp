#include <string>
#include <stack>
#include <map>
#include <vector>
#include "semantic.h"
#include <iostream>


// Vector of tree Nodes
typedef std::vector <Node*> nodeVector;

// // individual scope symbol table
// typedef  std::map <std::string, symbol> symbolTable;

// typedef std::stack <symbolTable> scopeStack;


int checkSemantics(std::string testcase, Node* node){
	// Checks if last declared function is main
	if (testcase == "last_fun_is_main"){
		if(node->Data != "fun-declaration"){exit(-1);}
		std::string name = node->Children[1]->Data;
		if(name != "main"){exit(-1);}
		checkSemantics("is_main_valid", node);
	}

	// Check if main function follows pattern "void main(void)"	
	else if (testcase == "is_main_valid"){
		// Type of main function. must be "void"
		std::string type = node->Children[0]->Data;
		// Number of parameters in main function. Must be 0 
		int paramSize = node->Children[2]->Children.size();
		// Number of declarations or statements inside main function. Must not be 0
		int compoundStmtSize = node->Children[3]->Children.size();
		// || compoundStmtSize == 0
		if(type != "void" || paramSize != 0 ){exit(-1);}
	}

	// Check if var is declared with type int
	else if (testcase == "is_var_type_int"){
		std::string type = node->Children[0]->Data;
		if(type != "int"){exit(-1);}
	}

	// Check if return type matches function declaration
	else if (testcase == "is_return_valid"){
		std::string funType = node->Children[0]->Data;
		// Search for return statement inside compound stmt
		for(std::vector<int>::size_type i = 0; i != node->Children[3]->Children.size(); i++){
			if(node->Children[3]->Children[i]->Data == "return-stmt"){
				int returnSize = node->Children[3]->Children[i]->Children.size();
				//return void, functionType must also be void
				if(returnSize == 0 && funType != "void"){
					exit(-1);
				}
				// else return type not void, functionType must also not be void 
				if(returnSize == 1 && funType == "void"){ //return void, functionType must also be void
					exit(-1);
				}
			}
		}
	}
	// else if (testcase == ""){}
	// else if (testcase == ""){}
	// else if (testcase == ""){}
	// else if (testcase == ""){}
	// else if (testcase == ""){}
	// else if (testcase == ""){}
	return 0;
}

///Tests made in main bison file:
	// Function call is made with same number or arguments of a defined function
	// Var declared is not previously declared (error?)
	

// Missing tests:
	// Check if arguments from function call are declared
	// Test arg type
	// Array treatment
	// LHS(!function) and RHS(recursive typecheck) of statement

