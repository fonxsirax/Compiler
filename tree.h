#ifndef TREE_H
#define TREE_H
#include <vector>


class Node {
	typedef std::vector <Node*> nodeVector;
	public:
		std::string Data;

		nodeVector Children;

		Node(std::string Data, nodeVector Vector);
		
		Node(std::string Data);
};




#endif