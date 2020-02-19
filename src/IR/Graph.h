#ifndef PAPYRUS_GRAPH_H
#define PAPYRUS_GRAPH_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/ASTConstructor.h"
#include "Variable.h"

#include <vector>
#include <unordered_map>
#include <string>

using ValueIndex = int;
using NodeIndex = int;

namespace papyrus {

class Graph;
class Function;
class Value;

class IRCtxInfo {
public:
    IRCtxInfo();
    void AddFunction(Function*);

private:
    Value* global_base_;
    std::unordered_map<ValueIndex, Value*> global_value_map_;
    std::unordered_map<std::string, Function*> functions_;

    // XXX: This should be initialised to 0
    int value_counter_;
    std::unordered_map<std::string, ValueIndex> global_values_;
};

class Function {
public:
    Function(const std::string&);
    const std::string& GetFunctionName() const { return func_name_; }

    Graph* GetIR() { return ir_; }
    void SetIR(Graph*);

private:
    std::string func_name_;
    Value* local_base_;
    Graph* ir_;
};

class Value {
};

class ConstantValue : public Value {
private:
    int val_;
};

class NamedValue : public Value {
private:
    std::string name_;
};

enum NodeType {
    NODE_ENTRY,
    NODE_EXIT,
    NODE_BB,
    NODE_OP,
    NODE_PHI,
    // XXX: To be filled up later.
};

class NodeData {
public:
    NodeData();
private:
    std::vector<ValueIndex> operands_;
};

class Node {
public:
    Node(NodeType);

    void AddPredecessor(NodeIndex ni) { predecessors_[ni] = true; }
    void AddSuccessor(NodeIndex ni) { successors_[ni] = true; }
    void AddData(NodeData* node_data) { node_data_ = node_data; }

    bool IsEntryNode() const { return node_type_ == NODE_ENTRY; }
    bool IsBBNode() const { return node_type_ == NODE_BB; }

private:
    NodeType node_type_;
    NodeData* node_data_;
    std::unordered_map<NodeIndex, bool> successors_;
    std::unordered_map<NodeIndex, bool> predecessors_;
};

class Graph {
public:
    Graph();

    NodeIndex CurrentCounter() const { return node_counter_; }
    Node* CurrentNode() { return node_map_[node_counter_]; }
    NodeIndex CurrentBB() const { return current_bb_; }

    NodeIndex CreateNewBB();
    NodeIndex CreateExit();

    void WriteVariable(const std::string&, NodeIndex, ValueIndex);

private:
    Node* root_;
    NodeIndex node_counter_;
    NodeIndex current_bb_;
    std::unordered_map<NodeIndex, Node*> node_map_;
    std::unordered_map<NodeIndex, bool> basic_blocks_;
    std::unordered_map<std::string, std::unordered_map<NodeIndex, ValueIndex> > current_defs_;
    std::unordered_map<ValueIndex, Value*> value_map_;
};

} // namespace papyrus

#endif /* PAPYRUS_GRAPH_H */
