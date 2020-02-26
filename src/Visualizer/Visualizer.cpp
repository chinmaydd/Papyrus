#include "Visualizer.h"

using namespace papyrus;

std::string GetBBString(BBIndex idx) {
    return "BB_" + std::to_string(idx);
}

std::string Instruction::ConvertToString() const {
    std::string res = "";

    res += "(" + std::to_string(Result()) + ") ";
    res += ins_to_str_.at(ins_type_) + " ";
    for (auto operand: operands_) {
        res += "(" + std::to_string(operand) + ") ";
    }

    return res;
}

Visualizer::Visualizer(IRC& irc) :
    irc_(irc) {}

std::string Visualizer::GetBaseNodeString(BBIndex bb_idx, const std::string& func_name) const {
    std::string res = "";

    res += "node: {\n";
    res += "title: \"" + GetBBString(bb_idx) + "\"\n";
    res += "label: \"[" + func_name + "] " + GetBBString(bb_idx) + "\n";

    return res;
}

std::string Visualizer::CloseNode() const {
    return "\"\n}\n";
}

std::string Visualizer::GetEdgeString(BBIndex source, BBIndex target) const {
    std::string res = "";

    res += "edge: {\n";
    res += "sourcename: \"" + GetBBString(source) + "\"\n";
    res += "targetname: \"" + GetBBString(target) + "\"\n";
    res += "color: black\n";
    res += "}\n";

    return res;
}

void Visualizer::DrawFunc(const Function* func) {
    std::string func_name = func->FunctionName();

    std::string bb_graph = "";
    for (auto bb_pair: func->BasicBlocks()) {
        BBIndex bb_idx = bb_pair.first;
        auto bb        = bb_pair.second;

        bb_graph += GetBaseNodeString(bb_idx, func_name);

        for (auto ins_idx: bb->Instructions()) {
            bb_graph += func->GetInstruction(ins_idx)->ConvertToString();
            bb_graph += "\n";
        }

        bb_graph += CloseNode();


        for (auto succ: bb->Successors()) {
            bb_graph += GetEdgeString(bb_idx, succ);
        }
    }

    graph_ << bb_graph;
}

void Visualizer::UpdateVCG() {
    for (auto func_pair: irc_.Functions()) {
        DrawFunc(func_pair.second);
    }
}

void Visualizer::WriteVCG() {
    graph_.open("/home/chinmay_dd/Projects/Papyrus/tests/test.vcg");

    // XXX: Subject to change.
    graph_ << "graph: {";
    graph_ << "orientation: top_to_bottom\n";
    graph_ << "manhattan_edges: yes\n";
    graph_ << "layoutalgorithm:compilergraph\n";
    graph_ << "title: \"program\"\n";

    UpdateVCG();

    graph_ << "}";
    graph_.close();
}
