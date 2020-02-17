#include "Graph.h"

using namespace papyrus;

Function::Function(const std::string& func_name) :
    func_name_(func_name) {}

void Function::AddLocalVariable(LocalVariable* var) {
    locals_[var->GetIdentifierName()] = var;
}

Node::Node(NodeType node_type) :
    node_type_(node_type) {}

Graph::Graph() {
    root_ = new Node(NodeType::NODE_ENTRY);
}
