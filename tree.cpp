#include <string>
#include "tree.h"

Node::Node(std::string value, nodeVector Vector){
		Data = value;
		Children = Vector;

};

Node::Node(std::string value){

		Data = value;
		// Children = NULL;
};
