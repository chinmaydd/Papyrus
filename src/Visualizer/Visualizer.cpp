#include "Visualizer.h"

using namespace papyrus;

Visualizer::Visualizer(IRC& irc) :
    irc_(irc) {}

void Visualizer::UpdateVCG() {
}

void Visualizer::WriteVCG() {
    graph_.open("/home/chinmay_dd/Projects/Papyrus/tests/test.vcg");

    // XXX: Subject to change.
    graph_ << "graph: {";
    graph_ << "orientation: top_to_bottom\n";
    graph_ << "manhattan_edges: yes\n";
    graph_ << "layoutalgorithm:compilergraph\n";
    graph_ << "\"program\"\n";

    UpdateVCG();

    graph_ << "}";
    graph_.close();
}
