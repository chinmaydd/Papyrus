#include "Visualizer.h"

using namespace papyrus;

using V = Value::ValueType;

std::string GetBBString(BI idx) {
    return "BB_" + std::to_string(idx);
}

Color Visualizer::GetColor(VI val_idx) const {
    if (coloring_.find(val_idx) == coloring_.end()) {
        return Color::COL_WHITE;
    } else {
        return coloring_.at(val_idx);
    }
}

// Should this be a part of the visualizer?
std::string Function::ConvertValueToString(VI val_idx) const {
    std::string res = "";
    Value *val = GetValue(val_idx);

    switch(val->Type()) {
        case V::VAL_CONST: {
            res += "#" + std::to_string(val->GetConstant());
            // Constant has a value; but displaying that in the IR does not
            // really make sense.
            // res += "_(" + std::to_string(val_idx) + ")";
            break;
        }
        case V::VAL_FUNC: {
            res += "&" + val->Identifier();
            res += "_(" + std::to_string(val_idx) + ")";
            break;
        }
        case V::VAL_BRANCH: {
            res += "BB_" + std::to_string(val->GetConstant());
            // BB has a value assigned; to keep the IR sane.
            // res += "_(" + std::to_string(val_idx) + ")";
            break;
        }
        case V::VAL_PHI:
        {
            // res += val->Identifier();
            res += "(" + std::to_string(val_idx) + ")";
            break;
        }
        case V::VAL_LOCATION: {
            res += "&" + val->Identifier();
            res += "_(" + std::to_string(val_idx) + ")";
            break;
        }
        case V::VAL_GLOBALBASE: {
            res += "GlobalBase";
            res += "_(" + std::to_string(val_idx) + ")";
            break;
        }
        case V::VAL_LOCALBASE: {
            res += "LocalBase";
            res += "_(" + std::to_string(val_idx) + ")";
            break;
        }
        default: {
            res += "(" + std::to_string(val_idx) + ")";
            break;
        }
    }
    
    return res;
}

Visualizer::Visualizer(IRC& irc) :
    irc_(irc) {}

void Visualizer::UpdateColoring(const std::unordered_map<VI, Color>& coloring) {
    coloring_ = coloring;
}

std::string Visualizer::GetBaseNodeString(BI bb_idx, const std::string& func_name) const {
    std::string res = "";

    res += "node: {\n";
    res += "title: \"" + func_name + ":" + GetBBString(bb_idx) + "\"\n";
    res += "label: \"[" + func_name + "] " + GetBBString(bb_idx) + "\n";

    return res;
}

std::string Visualizer::CloseNode() const {
    return "\"\n}\n";
}

std::string Visualizer::GetEdgeString(const std::string& func_name, BI source, BI target) const {
    std::string res = "";

    res += "edge: {\n";
    res += "sourcename: \"" + func_name + ":" + GetBBString(source) + "\"\n";
    res += "targetname: \"" + func_name + ":" + GetBBString(target) + "\"\n";
    res += "color: black\n";
    res += "}\n";

    return res;
}

std::string Visualizer::RegisterString(VI val_idx) const {
    auto val = irc_.GetValue(val_idx);
    std::string retval = "";

    switch (val->Type()) {
        case V::VAL_GLOBALBASE: {
            retval += "GlobalBase ";
            break;
        }
        case V::VAL_LOCATION: {
            retval += "&" + val->Identifier() + " ";
            break;
        }
        case V::VAL_FUNC: {
            retval += "&" + val->Identifier() + " ";
            break;
        }
        case V::VAL_BRANCH: {
            retval += "BB_" + std::to_string(val->GetConstant()) + " ";
            break;
        }
        case V::VAL_CONST: {
            retval += "#" + std::to_string(val->GetConstant()) + " ";
            break;
        }
        case V::VAL_ANY:
        case V::VAL_VAR: {
            retval += "(R" + std::to_string(GetColor(val_idx)) + ") ";
            break;
        }
        case V::VAL_LOCALBASE: {
            retval += "LocalBase ";
            break;
        }
        default: {
            retval += "(R" + std::to_string(GetColor(val_idx)) + ") ";
        }
    }

    return retval;
}

std::string Visualizer::ConvertInstructionToString(const Function* fn, II ins_idx) const {
    Instruction* ins = fn->GetInstruction(ins_idx);
    auto result_idx  = ins->Result();
    auto result      = fn->GetValue(result_idx);
    std::string res = "";

    if (RADone()) {
        res += RegisterString(result_idx);
    } else if (!ins->IsKill()) {
        res += "(" + std::to_string(result_idx) + ")" + " ";
    }

    res += ins_to_str_.at(ins->Type()) + " ";

    for (auto operand: ins->Operands()) {
        if (RADone()) {
            res += RegisterString(operand);
        } else {
            res += fn->ConvertValueToString(operand) + " ";
        }
    }

    return res;
}

void Visualizer::DrawFunc(const Function* func) {
    std::string func_name = func->FunctionName();

    std::string bb_graph = "";
    for (auto bb_pair: func->BasicBlocks()) {
        BI bb_idx = bb_pair.first;
        auto bb   = bb_pair.second;

        if (bb->IsDead()) {
            continue;
        }

        bb_graph += GetBaseNodeString(bb_idx, func_name);

        for (auto ins_idx: bb->InstructionOrder()) {
            if (!func->IsActive(ins_idx)) {
                continue;
            }

            bb_graph += ConvertInstructionToString(func, ins_idx);
            bb_graph += "\n";
        }

        bb_graph += CloseNode();


        for (auto succ: bb->Successors()) {
            bb_graph += GetEdgeString(func_name, bb_idx, succ);
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

void Visualizer::WriteFinalIR(const std::string& fname) {
    writing_ir_before_ra_ = false;

    WriteVCG(fname);
}

void Visualizer::WriteIR(const std::string& fname) {
    writing_ir_before_ra_ = true;

    WriteVCG(fname);
}
