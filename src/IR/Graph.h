#ifndef PAPYRUS_GRAPH_H
#define PAPYRUS_GRAPH_H

#include "Papyrus/Logger/Logger.h"
#include "FrontEnd/AST.h"

#include <vector>
#include <unordered_map>
#include <string>

using ValueIndex = int;
using NodeIndex = int;

namespace papyrus {

class Graph;
class GlobalVariable;
class LocalVariable;
class Function;
class Value;

class IRCtxInfo {
public:
    void AddGlobalVariable(GlobalVariable*);

private:
    Value* global_base_;
    std::unordered_map<ValueIndex, Value*> value_map_;
    std::unordered_map<std::string, GlobalVariable*> globals_;
    std::unordered_map<std::string, Function*> functions_;

    // XXX: This should be initialised to 0
    int value_counter_;
    std::unordered_map<std::string, ValueIndex> global_values_;
};

class Function {
private:
    Value* local_base_;
    std::unordered_map<std::string, LocalVariable*> locals_;
    Graph* ir;
};

class Variable {
public:
    Variable(bool, const std::string&, const std::vector<int>&);
    const std::string& GetIdentifier() const { return identifier_; };

protected:
    std::string identifier_;
    bool is_array_;
    std::vector<int> dimensions_;
};

class LocalVariable : public Variable {
};

class GlobalVariable : public Variable {
public:
    GlobalVariable(const std::string&, bool, const std::vector<int>&);
};

class Value {
};

enum NodeType {
    NODE_BB,
    NODE_OP,
    // XXX: To be filled up later.
};

class NodeData {
private:
    std::vector<ValueIndex> operands_;
};

class Node {
private:
    NodeData* node_data_;
    std::unordered_map<NodeIndex, bool> successors_;
    std::unordered_map<NodeIndex, bool> predecessors_;
};

class Graph {
private:
    std::unordered_map<NodeIndex, Node*> node_map_;
};

} // namespace papyrus

#endif /* PAPYRUS_GRAPH_H */
