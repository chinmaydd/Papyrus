#include "Visualizer.h"

using namespace papyrus;

using V = Value::ValueType;

std::string GetBBString(BI idx) {
    return "BB_" + std::to_string(idx);
}

std::string Function::ConvertValueToString(VI val_idx) const {
    std::string res = "";
    Value *val = GetValue(val_idx);

    switch(val->GetType()) {
        case V::VAL_CONST: {
            res += "#" + std::to_string(val->GetValue());
            break;
        }
        case V::VAL_FUNC: {
            res += "&" + val->Identifier();
            break;
        }
        case V::VAL_BRANCH: {
            res += "BB_" + std::to_string(val->GetValue());
            break;
        }
        default: {
            res += "(" + std::to_string(val_idx) + ")";
            break;
        }
    }
    
    return res;
}

std::string Function::ConvertInstructionToString(II ins_idx) const {
    Instruction* ins = GetInstruction(ins_idx);
    bool is_relational = IsRelational(ins->Type());
    std::string res = "";

    res += "(" + std::to_string(ins->Result()) + ")" + " ";
    res += ins_to_str_.at(ins->Type()) + " ";

    for (auto operand: ins->Operands()) {
        res += ConvertValueToString(operand) + " ";
    }

    return res;
}

Visualizer::Visualizer(IRC& irc) :
    irc_(irc) {}

std::string Visualizer::GetBaseNodeString(BI bb_idx, const std::string& func_name) const {
    std::string res = "";

    res += "node: {\n";
    res += "title: \"" + GetBBString(bb_idx) + "\"\n";
    res += "label: \"[" + func_name + "] " + GetBBString(bb_idx) + "\n";

    return res;
}

std::string Visualizer::CloseNode() const {
    return "\"\n}\n";
}

std::string Visualizer::GetEdgeString(BI source, BI target) const {
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
        BI bb_idx = bb_pair.first;
        auto bb   = bb_pair.second;

        bb_graph += GetBaseNodeString(bb_idx, func_name);

        for (auto ins_idx: bb->InstructionOrder()) {
            if (!func->IsActive(ins_idx)) {
                continue;
            }

            bb_graph += func->ConvertInstructionToString(ins_idx);
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
    std::string func_name;
    for (auto func_pair: irc_.Functions()) {
        func_name = func_pair.first;
        if (!irc_.IsIntrinsic(func_name)) {
            DrawFunc(func_pair.second);
        }
    }
}

void Visualizer::WriteVCG(const std::string& fname) {
    graph_.open(fname);

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
