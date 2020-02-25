#include "Visualizer.h"

using namespace papyrus;

std::string GetBBString(BBIndex idx) {
    return "BB_" + std::to_string(idx);
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
    return "\"\n}";
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

    std::string bb_graph;
    std::string edge_graph;
    for (auto bb_pair: func->BasicBlocks()) {
        bb_graph = "";

        BBIndex bb_idx = bb_pair.first;
        auto bb        = bb_pair.second;

        bb_graph += GetBaseNodeString(bb_idx, func_name);

        bb_graph += CloseNode();

        graph_ << bb_graph;

        for (auto succ: bb->Successors()) {
            graph_ << GetEdgeString(bb_idx, succ);
        }
    }
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
