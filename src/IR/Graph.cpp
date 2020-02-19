#include "Graph.h"

using namespace papyrus;

IRCtxInfo::IRCtxInfo() :
    value_counter_(0)
    {}

Function::Function(const std::string& func_name) :
    func_name_(func_name) {}

void Function::SetIR(Graph *ir) {
    ir_ = ir;
}

void Function::AddLocalVariable(LocalVariable* var) {
    locals_[var->GetIdentifierName()] = var;
}

Node::Node(NodeType node_type) :
    node_type_(node_type) {}

Graph::Graph() :
    node_counter_(0),
    root_(new Node(NodeType::NODE_ENTRY)) {
        node_map_[node_counter_] = root_;
}

NodeIndex Graph::CreateExit() {
    Node* exit_node = new Node(NodeType::NODE_EXIT);

    exit_node->AddPredecessor(node_counter_);
    
    node_counter_++;
    node_map_[node_counter_] = exit_node;

    return node_counter_;
}

NodeIndex Graph::CreateNewBB() {
    // TODO: Add predecessor(s) to BB
    node_counter_++;

    Node* bb_node = new Node(NodeType::NODE_BB);

    node_map_[node_counter_] = bb_node;
    basic_blocks_[node_counter_] = true;

    current_bb_ = node_counter_;

    return node_counter_;
}

void Graph::WriteVariable(const std::string& var_repr, NodeIndex bb_idx, ValueIndex value) {
    current_defs_[var_repr][bb_idx] = value;
}
