#include "visitor.h"
#include "ast.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include "Typechecker.h"

namespace {
    std::string escapeAsmString(const std::string& value) {
        std::ostringstream escaped;
        for (char ch : value) {
            switch (ch) {
                case '\\': escaped << "\\\\"; break;
                case '"':  escaped << "\\\""; break;
                case '\n': escaped << "\\n"; break;
                case '\r': escaped << "\\r"; break;
                case '\t': escaped << "\\t"; break;
                default:   escaped << ch; break;
            }
        }
        return escaped.str();
    }
}

int GenCodeVisitor::getLocalSlot(const string& nombre) {
    auto it = posicion.find(nombre);
    if (it != posicion.end()) return it->second;
    posicion[nombre] = contador;
    return contador++;
}

void GenCodeVisitor::emitirDefers() {
    for (auto it = deferStack.rbegin(); it != deferStack.rend(); ++it) {
        (*it)->accept(this);
    }
    deferStack.clear();
}

int GenCodeVisitor::alignStackBytes(int bytes) const {
    if (bytes == 0) bytes = 16;
    int rem = bytes % 16;
    return rem == 0 ? bytes : bytes + (16 - rem);
}

size_t GenCodeVisitor::maxRegisterArgs() const {
#ifdef _WIN32
    return 4;
#else
    return 6;
#endif
}

const char* GenCodeVisitor::argRegister(size_t index) const {
#ifdef _WIN32
    static const char* argRegs[] = {"%rcx", "%rdx", "%r8", "%r9"};
#else
    static const char* argRegs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
#endif
    return index < maxRegisterArgs() ? argRegs[index] : nullptr;
}

void GenCodeVisitor::emitCall(const std::string& nombre) {
#ifdef _WIN32
    if (callScratchOffset > 0) {
        out << "    movq %rsp, " << (-callScratchOffset) << "(%rbp)\n";
    } else {
        out << "    movq %rsp, %r11\n";
    }
    out << "    andq $-16, %rsp\n";
    out << "    subq $32, %rsp\n";
    out << "    call " << nombre << "\n";
    if (callScratchOffset > 0) {
        out << "    movq " << (-callScratchOffset) << "(%rbp), %rsp\n";
    } else {
        out << "    movq %r11, %rsp\n";
    }
#else
    out << "    call " << nombre << "\n";
#endif
}

void GenCodeVisitor::emitStringData(const std::string& label, const std::string& value) {
    out << ".data\n";
    out << label << ": .string \"" << escapeAsmString(value) << "\"\n";
    out << ".text\n";
}

GenCodeVisitor::GenCodeVisitor(std::ostream &out) : out(out) {
    tiposPtr = new TypeCheckerVisitor();
}

GenCodeVisitor::~GenCodeVisitor() {
    delete tiposPtr;
}

void GenCodeVisitor::genAddress(Exp* lval) {
    if (auto idx = dynamic_cast<AlgoconcorchetesExp*>(lval)) {
        if (auto idBase = dynamic_cast<IdExp*>(idx->nombre)) {
            if (arrayLocals.count(idBase->value)) {
                out << "  leaq " << (-8*posicion[idBase->value]) << "(%rbp), %rax\n";
            } else {
                idBase->accept(this);   // puntero (ej. de "new i32"): cargar el valor
            }
        } else {
            idx->nombre->accept(this);
        }
        out << "  pushq %rax\n";
        idx->dentroexp->accept(this);
        out << "  movq %rax, %rcx\n";
        out << "  popq %rax\n";
        out << "  leaq (%rax,%rcx,8), %rax\n";
        return;
    } else if (auto idx = dynamic_cast<AlgoconcorchetesExp*>(lval)) {
        idx->nombre->accept(this);
        out << "  pushq %rax\n";
        idx->dentroexp->accept(this);
        out << "  movq %rax, %rcx\n";
        out << "  popq %rax\n";
        out << "  leaq (%rax,%rcx,8), %rax\n";
    } else if (auto p = dynamic_cast<PuntoExp*>(lval)) {
        p->exp->accept(this);
        int off = 0;
        if (!lastTypeName.empty() && structFieldOffsets.count(lastTypeName)) {
            auto& campos = structFieldOffsets[lastTypeName];
            auto it = campos.find(p->id);
            if (it != campos.end()) off = it->second;
        }
        out << "  addq $" << off << ", %rax\n";
    } else {
        lval->accept(this); // caso *p = x: lval ya es la dirección
    }
}

namespace {
    bool getConstInt(Exp* e, int& out) {
        if (auto n = dynamic_cast<NumberExpDecimal*>(e)) { out = n->value; return true; }
        return false;
    }
}
namespace {
    void recolectarVars(Stmt* s, std::unordered_set<std::string>& vars);

    void recolectarVarsBody(Body* b, std::unordered_set<std::string>& vars) {
        if (!b) return;
        for (auto s : b->slist) recolectarVars(s, vars);
    }

    void recolectarVars(Stmt* s, std::unordered_set<std::string>& vars) {
        if (!s) return;
        if (auto a = dynamic_cast<AsignStmt*>(s)) {
            if (a->variable != "__call__") vars.insert(a->variable);
        } else if (auto i = dynamic_cast<IfStmt*>(s)) {
            recolectarVarsBody(i->cuerpodelif, vars);
            if (i->hayelse) recolectarVarsBody(i->cuerpodelelse, vars);
        } else if (auto w = dynamic_cast<WhileStmt*>(s)) {
            for (auto st : w->cuerpodelwhile) recolectarVars(st, vars);
        } else if (auto f = dynamic_cast<ForStmt*>(s)) {
            if (f->asignacion) recolectarVars(f->asignacion, vars);
            if (f->incremento) recolectarVars(f->incremento, vars);
            recolectarVarsBody(f->cuerpo, vars);
        } else if (auto bs = dynamic_cast<BodyStmt*>(s)) {
            recolectarVarsBody(bs->cuerpo, vars);
        } else if (auto sw = dynamic_cast<SwitchStmt*>(s)) {
            for (auto& pr : sw->casos) recolectarVarsBody(pr.second, vars);
            if (sw->default_caso) recolectarVarsBody(sw->default_caso, vars);
        } else if (auto t = dynamic_cast<TryStmt*>(s)) {
            recolectarVarsBody(t->try_body, vars);
            recolectarVarsBody(t->catch_body, vars);
        } else if (auto d = dynamic_cast<DeferStmt*>(s)) {
            recolectarVars(d->stmt, vars);
        }
    }
}

namespace {
    void recolectarArrays(Stmt* s, std::unordered_map<std::string,int>& arrs) {
        if (!s) return;
        if (auto a = dynamic_cast<AsignStmt*>(s)) {
            if (auto arr = dynamic_cast<ArrayType*>(a->tipoDeclarado)) {
                int tam = 1;
                getConstInt(arr->exp1, tam);
                arrs[a->variable] = tam;
            }
        } else if (auto i = dynamic_cast<IfStmt*>(s)) {
            if (i->cuerpodelif) for (auto st : i->cuerpodelif->slist) recolectarArrays(st, arrs);
            if (i->hayelse && i->cuerpodelelse) for (auto st : i->cuerpodelelse->slist) recolectarArrays(st, arrs);
        } else if (auto w = dynamic_cast<WhileStmt*>(s)) {
            for (auto st : w->cuerpodelwhile) recolectarArrays(st, arrs);
        }
        // agregar for/switch/etc si hace falta, mismo patrón que recolectarVars
    }
    void recolectarArraysBody(Body* b, std::unordered_map<std::string,int>& arrs) {
        if (!b) return;
        for (auto s : b->slist) recolectarArrays(s, arrs);
    }
}

//genecode

void GenCodeVisitor::gencode(Programa* program){
    tiposPtr->typecheck(program);

    for (auto dec : program->declist) {
        if (Fundec* f = dynamic_cast<Fundec*>(dec)) {
            std::unordered_set<std::string> vars;
            for (auto& p : f->id_parametros) vars.insert(p);
            recolectarVarsBody(f->cuerpo, vars);

            // detectar arrays de tamaño fijo (necesita recorrer los AsignStmt del body)
            std::unordered_map<std::string,int> arrsLocal;
            recolectarArraysBody(f->cuerpo, arrsLocal);   // función nueva, ver abajo
            for (auto& kv : arrsLocal) arrayLocals[kv.first] = kv.second;

            // cada elemento del array ocupa un slot: sumar (tamaño-1) slots extra por cada array
            int extra = 0;
            for (auto& kv : arrsLocal) extra += (kv.second - 1);
            funcontador[f->nombre] = (int)vars.size() + extra;
        }
        else if (Template* t = dynamic_cast<Template*>(dec)) {  
            std::string nombreCompleto = t->id1;
            std::unordered_set<std::string> vars;
            for (auto& p : t->id_parametros) vars.insert(p);
            recolectarVarsBody(t->block, vars);
            funcontador[nombreCompleto] = (int)vars.size();

            funParamNames[nombreCompleto] = t->id_parametros;
            std::vector<std::string> tiposParam;
            for (auto* tp : t->tipo_parametros) {
                tp->accept(tiposPtr);
                tiposParam.push_back(tiposPtr->currentType);
            }
            funParamTypes[nombreCompleto] = tiposParam;
        }
    }

    program->accept(this);
}

void GenCodeVisitor::visit(Programa *program) {
    
    out << ".data\n";
    out << "print_int_fmt: .string \"%ld\\n\"\n";
    out << "print_str_fmt: .string \"%s\\n\"\n";
    out << "\n.text\n";

    for (auto dec : program->declist)
        dec->accept(this);

#ifndef _WIN32
    out << "\n.section .note.GNU-stack,\"\",@progbits\n";
#endif
}

void GenCodeVisitor::visit(Body* b) {
    for(auto stm : b->slist)
        stm->accept(this);
}

// -----------------------------------------------------------------------------
// dec-gencode
// -----------------------------------------------------------------------------

void GenCodeVisitor::visit(Template *t) {
    string nombreCompleto = t->id1;
    funcionActual = nombreCompleto;
    posicion.clear();
    contador = 1;
    deferStack.clear();

    out << "\n.global " << nombreCompleto << "\n";
    out << nombreCompleto << ":\n";
    out << "    pushq %rbp\n";
    out << "    movq %rsp, %rbp\n";

    int bytes = alignStackBytes(funcontador[nombreCompleto] * 8 + 8);
    callScratchOffset = bytes;

    out << "    subq $" << bytes << ", %rsp\n";

    for (size_t i = 0; i < t->id_parametros.size() && i < maxRegisterArgs(); i++) {
        int slot = getLocalSlot(t->id_parametros[i]);
        out << "    movq " << argRegister(i) << ", " << (-8 * slot) << "(%rbp)\n";
    }

    if (t->block) t->block->accept(this);

    emitirDefers();
    out << "end_" << nombreCompleto << ":\n";
    out << "    movq $0, %rax\n";
    out << "    leave\n";
    out << "    ret\n";
}

void GenCodeVisitor::visit(ConstDec *c) {
    out << ".data\n";
    out << ".global " << c->nombre << "\n";
    out << c->nombre << ":\n";

    if (c->exp != nullptr) {
        //se usa constant folding para evaluar la expresión y obtener su valor inmediato
        // out << "    .quad " << c->exp->get_valor_inmediato() << "\n";
    } else {
        out << "    .quad 0\n";
    }
}

void GenCodeVisitor::visit(VarDec *v) {
    if (!this->dentroDeFuncion) {
        out << ".data\n";
        out << ".global g_" << v->nombre << "\n";
        out << "g_" << v->nombre << ":\n";

        if (v->exp != nullptr) {
            //se usa constant folding para evaluar la expresión y obtener su valor inmediato
            // out << "    .quad " << c->exp->get_valor_inmediato() << "\n";
        } else {
            out << "    .quad 0\n"; 
        }
    } 
    else {
        
        if (posicion.count(v->nombre) == 0) {
            int nuevoOffset = posicion.size() + 1;
            posicion[v->nombre] = nuevoOffset;
        }

        int offset = -8 * posicion[v->nombre];

        if (v->exp != nullptr) {
            v->exp->accept(this);
            out << "    movq %rax, " << offset << "(%rbp)\n"; 
        } else {
            out << "    movq $0, " << offset << "(%rbp)\n"; 
        }
    }
}

void GenCodeVisitor::visit(Structdec* dec) {
    int numeroCampos = dec->id_parametros.size();
    structFields[dec->nombre] = numeroCampos;

    std::unordered_map<std::string,int> offsets;
    for (int i = 0; i < numeroCampos; i++)
        offsets[dec->id_parametros[i]] = i * 8;
    structFieldOffsets[dec->nombre] = offsets;
}

void GenCodeVisitor::visit(Fundec* dec) {
    funcionActual = dec->nombre;
    posicion.clear();
    contador = 1;
    deferStack.clear();

    out << ".globl " << dec->nombre << "\n";
    out << dec->nombre << ":\n";
    out << "    pushq %rbp\n";
    out << "    movq %rsp, %rbp\n";

    int bytes = alignStackBytes(funcontador[dec->nombre] * 8 + 8);
    callScratchOffset = bytes;

    out << "    subq $" << bytes << ", %rsp\n";

    for (size_t i = 0; i < dec->id_parametros.size() && i < maxRegisterArgs(); i++) {
        int slot = getLocalSlot(dec->id_parametros[i]);
        out << "    movq " << argRegister(i) << ", " << (-8 * slot) << "(%rbp)\n";
    }

    if (dec->cuerpo) dec->cuerpo->accept(this);

    emitirDefers();
    out << "end_" << dec->nombre << ":" << endl;
    out << "    movq $0, %rax\n"; 
    out << "    leave\n";       
    out << "    ret\n";
}

// -----------------------------------------------------------------------------
// exp-gencode
// -----------------------------------------------------------------------------

Value GenCodeVisitor::visit(BinaryExp *exp) {
    exp->left->accept(this);
    out << "  pushq %rax\n";
    exp->right->accept(this);
    out << "  movq %rax, %rcx\n";
    out << "  popq %rax\n";

    switch (exp->op) {
    case PLUS_OP:  out << "  addq %rcx, %rax\n";  break;
    case MINUS_OP: out << "  subq %rcx, %rax\n";  break;
    case MUL_OP:   out << "  imulq %rcx, %rax\n"; break;
    case DIV_OP:
            out << "    cqto\n";             
            out << "    idivq %rcx\n";       
            break;
            
    case MODULO_OP:
            out << "    cqto\n";
            out << "    idivq %rcx\n";        
            out << "    movq %rdx, %rax\n";   
            break;

    case IGUALIGUAL:
        out << "  cmpq %rcx, %rax\n";
        out << "  movq $0, %rax\n";
        out << "  sete %al\n";
        out << "  movzbq %al, %rax\n";
        break;

    case DIFERENTE_OP:
        out << "  cmpq %rcx, %rax\n";
        out << "  movq $0, %rax\n";
        out << "  setne %al\n";
        out << "  movzbq %al, %rax\n";
        break;

    case MENOR:
        out << "  cmpq %rcx, %rax\n";
        out << "  movq $0, %rax\n";
        out << "  setl %al\n";
        out << "  movzbq %al, %rax\n";
        break;

    case MENORI:
        out << "  cmpq %rcx, %rax\n";
        out << "  movq $0, %rax\n";
        out << "  setle %al\n";
        out << "  movzbq %al, %rax\n";
        break;

    case MAYOR:
        out << "  cmpq %rcx, %rax\n";
        out << "  movq $0, %rax\n";
        out << "  setg %al\n";
        out << "  movzbq %al, %rax\n";
        break;

    case MAYORI:
        out << "  cmpq %rcx, %rax\n";
        out << "  movq $0, %rax\n";
        out << "  setge %al\n";
        out << "  movzbq %al, %rax\n";
        break;

    case AND: out << "  andq %rcx, %rax\n"; break;
    case OR:  out << "  orq %rcx, %rax\n";  break;

    default:
        break;
    }
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NumberExpDecimal *exp) {
    out << "  movq $" << exp->value << ", %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NumberExpFlotante *exp) {
    out << "movq $" << (long)exp->value << ", %rax" << endl;
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(StringExp *exp) {
    std::string strLabel = "str_" + std::to_string(labelcont++);
    emitStringData(strLabel, exp->valor);
    out << "    leaq " << strLabel << "(%rip), %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(CharExp *exp) {
    out << "    movq $0, %rax\n";
    out << "    movb $" << (int)exp->valor << ", %al\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(IdExp *exp) {
    if (posicion.count(exp->value)) {
        int varOffset = posicion[exp->value];
        out << "    movq " << (-8 * varOffset) << "(%rbp), %rax\n";
    } 
    else if (globales.count(exp->value)) {
        out << "    movq " << exp->value << "(%rip), %rax\n";
    } 
    else {
        std::cerr << "Error en compilación: Variable '" << exp->value << "' no declarada.\n";
    }
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(BoolExp* exp) {
    out << "movq $" << (exp->booleano == "true" ? 1 : 0) << ", %rax" << endl;
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(UnaryExp *exp) {
    if (exp->op != UnaryExp::ADDRESS) {
        exp->exp->accept(this); 
    }

    switch (exp->op) {
        case UnaryExp::NOT_OP: 
            out << "    cmpq $0, %rax\n";
            out << "    movq $0, %rax\n";
            out << "    sete %al\n";
            out << "    movzbq %al, %rax\n";
            break;

        case UnaryExp::NEGATE: 
            out << "    negq %rax\n";
            break;

        case UnaryExp::DEREF: 
            out << "    movq (%rax), %rax\n"; 
            break;

        case UnaryExp::ADDRESS: 
            IdExp* id = dynamic_cast<IdExp*>(exp->exp);
            if (id != nullptr) {
                if (posicion.count(id->value)) {
                    out << "    leaq " << (-8 * posicion[id->value]) << "(%rbp), %rax\n";
                } else if (globales.count(id->value)) {
                    out << "    leaq " << id->value << "(%rip), %rax\n";
                }
            }
            break;
    }
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NewExp *e) {
    this->lastTypeName = "";

    if (e->tipo != nullptr) {
        e->tipo->accept(this);
    }

    int tamanoBytes = 8; 

    if (!this->lastTypeName.empty() && structFields.count(this->lastTypeName)) {
        int numeroCampos = structFields[this->lastTypeName];
        tamanoBytes = numeroCampos * 8;
    }
    out << "    movq $" << tamanoBytes << ", " << argRegister(0) << "\n";
    emitCall("malloc");
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NullExp *e) {
    out << "    movq $0, %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(UndefinedExp *e) {
    out << "    movq $0, %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(ReferenceExp* exp) {
    if (auto id = dynamic_cast<IdExp*>(exp->exp)) {
        if (posicion.count(id->value))
            out << "leaq " << (-8 * posicion[id->value]) << "(%rbp), %rax" << endl;
        else if (globales.count(id->value))
            out << "leaq g_" << id->value << "(%rip), %rax" << endl;
        else {
            out << "# advertencia: referencia a variable no declarada" << endl;
            out << "movq $0, %rax" << endl;
        }
    } else {
        exp->exp->accept(this);
    }
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(PunteroExp* exp) {
    exp->exp->accept(this);          
    out << "movq (%rax), %rax" << endl; 
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(AlgoconcorchetesylistaExp* exp) {
    int n = (int)exp->argumentos.size();

    out << "movq $" << (n * 8) << ", " << argRegister(0) << endl;
    emitCall("malloc");
    out << "pushq %rax" << endl; 

    for (int i = 0; i < n; i++) {
        exp->argumentos[i]->accept(this); 
        out << "movq %rax, %rcx" << endl;
        out << "movq 0(%rsp), %rax" << endl;
        out << "movq %rcx, " << (i * 8) << "(%rax)" << endl;
    }

    out << "popq %rax" << endl; 
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(PuntoExp *e) {
    e->exp->accept(this);
    out << "    movq %rax, %r10\n";

    int offsetCampo = 0;
    if (!lastTypeName.empty() && structFieldOffsets.count(lastTypeName)) {
        auto& campos = structFieldOffsets[lastTypeName];
        auto it = campos.find(e->id);
        if (it != campos.end()) offsetCampo = it->second;
    }

    out << "    movq " << offsetCampo << "(%r10), %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(AlgoconcorchetesExp* exp) {
    exp->nombre->accept(this);    
    out << "pushq %rax" << endl;
    exp->dentroexp->accept(this);  
    out << "movq %rax, %rcx" << endl;
    out << "popq %rax" << endl;
    out << "leaq (%rax,%rcx,8), %rax" << endl;
    out << "movq (%rax), %rax" << endl;   
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(FcallExp *e) {
    size_t n = std::min(e->argumentos.size(), maxRegisterArgs());

    for (size_t i = n; i > 0; i--) {
        if (e->argumentos[i - 1] != nullptr) {
            e->argumentos[i - 1]->accept(this);
            out << "    pushq %rax\n";
        } else {
            out << "    pushq $0\n";
        }
    }

    for (size_t i = 0; i < n; i++) {
        out << "    popq " << argRegister(i) << "\n";
    }
    emitCall(e->nombre);
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(LambdaExp *e) {
    int lbl = labelcont++;
    
    std::string nombreLambda = "lambda_func_" + std::to_string(lbl);
    std::string labelLambdaSkip = "lambda_skip_" + std::to_string(lbl);

    out << "    jmp " << labelLambdaSkip << "\n";

    out << ".globl " << nombreLambda << "\n";
    out << nombreLambda << ":\n";
    out << "    pushq %rbp\n";
    out << "    movq %rsp, %rbp\n";

    std::string funcionPadre = funcionActual;
    int scratchPadre = callScratchOffset;
    funcionActual = nombreLambda; 
    
    posicion.clear();
    contador = 1;
    deferStack.clear(); 

    int totalVariables = e->id_parametros.size(); 
    funcontador[nombreLambda] = totalVariables + 4; 

    int bytes = alignStackBytes(funcontador[nombreLambda] * 8 + 8);
    callScratchOffset = bytes;

    out << "    subq $" << bytes << ", %rsp\n";

    if (e->hayparametros) {
        for (size_t i = 0; i < e->id_parametros.size() && i < maxRegisterArgs(); i++) {
            int slot = getLocalSlot(e->id_parametros[i]);
            out << "    movq " << argRegister(i) << ", " << (-8 * slot) << "(%rbp)\n";
        }
    }

    if (e->cuerpo) e->cuerpo->accept(this);

    out << "end_" << nombreLambda << ":\n";
    out << "    movq $0, %rax\n"; 
    out << "    leave\n";       
    out << "    ret\n";

    out << labelLambdaSkip << ":\n";

    funcionActual = funcionPadre;
    callScratchOffset = scratchPadre;

    out << "    leaq " << nombreLambda << "(%rip), %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NotExp *exp) {
    if (exp->exp) exp->exp->accept(this);
    out << "    cmpq $0, %rax\n";
    out << "    movq $0, %rax\n";
    out << "    sete %al\n";
    out << "    movzbq %al, %rax\n";
    return Value::makeInt(0);
}

// -----------------------------------------------------------------------------
// stm-gencode
// -----------------------------------------------------------------------------

void GenCodeVisitor::visit(IfStmt *stm) {
    int lbl = labelcont++;
    stm->condicion->accept(this);
    out << "  cmpq $0, %rax\n";
    out << "  je else_" << lbl << "\n";
    stm->cuerpodelif->accept(this);
    out << "  jmp endif_" << lbl << "\n";
    out << "else_" << lbl << ":\n";
    if (stm->cuerpodelelse)
        stm->cuerpodelelse->accept(this);
    out << "endif_" << lbl << ":\n";
}

void GenCodeVisitor::visit(WhileStmt *stm) {
    int lbl = labelcont++;
    std::string oldBreak = currentBreakLabel;
    currentBreakLabel = "endwhile_" + std::to_string(lbl);

    std::string oldContinue = currentContinueLabel;
    currentContinueLabel = "while_" + std::to_string(lbl);

    out << "while_" << lbl << ":\n";
    stm->condicion->accept(this);
    out << "  cmpq $0, %rax\n";
    out << "  je endwhile_" << lbl << "\n";
    for (auto s : stm->cuerpodelwhile) {
        s->accept(this);
    }
    out << "  jmp while_" << lbl << "\n";
    out << "endwhile_" << lbl << ":\n";

    currentBreakLabel = oldBreak;
    currentContinueLabel = oldContinue;
}

void GenCodeVisitor::visit(BodyStmt* stm) {
    stm->cuerpo->accept(this);
}


void GenCodeVisitor::visit(AsignStmt* stm) {
    if (stm->variable == "__call__") { stm->exp->accept(this); return; }

    if (arrayLocals.count(stm->variable) && !posicion.count(stm->variable)) {
        int tam = arrayLocals[stm->variable];
        int base = contador;
        posicion[stm->variable] = base;
        contador += tam;          
        return;                   
    }

    stm->exp->accept(this);
    if (globales.count(stm->variable) && !posicion.count(stm->variable)) {
        out << "movq %rax, g_" << stm->variable << "(%rip)" << endl;
    } else {
        int slot = getLocalSlot(stm->variable);
        out << "movq %rax, " << (-8 * slot) << "(%rbp)" << endl;
    }
}

void GenCodeVisitor::visit(PrintStmt* stm) {
    stm->exp->accept(this);
    if (dynamic_cast<StringExp*>(stm->exp)) {
        out << "movq %rax, " << argRegister(1) << endl;
        out << "leaq print_str_fmt(%rip), " << argRegister(0) << endl;
    } else {
        out << "movq %rax, " << argRegister(1) << endl;
        out << "leaq print_int_fmt(%rip), " << argRegister(0) << endl;
    }
    out << "movl $0, %eax" << endl;
    emitCall("printf");
}

void GenCodeVisitor::visit(ReturnStm* stm) {
    if (stm->exp) stm->exp->accept(this);
    else out << "movq $0, %rax" << endl;

    emitirDefers();
    out << "leave" << endl;
    out << "ret" << endl;
}

void GenCodeVisitor::visit(DeleteStm* stm) {
    stm->exp->accept(this);
    out << "movq %rax, " << argRegister(0) << endl;
    emitCall("free");
}

void GenCodeVisitor::visit(ContinueStm* stm) {
    if (!currentContinueLabel.empty()) {
        out << "  jmp " << currentContinueLabel << "\n";
    } else {
        out << "# error semántico: continue fuera de bucle\n";
    }
}

void GenCodeVisitor::visit(BreakStmt *stm) {
    if (!currentBreakLabel.empty()) {
        out << "  jmp " << currentBreakLabel << "\n";
    } else {
        out << "# error semántico: break fuera de bucle\n";
    }
}

void GenCodeVisitor::visit(SwitchStmt *stm) {
    int lbl = labelcont++;
    
    std::string oldBreak = currentBreakLabel;
    currentBreakLabel = "endswitch_" + std::to_string(lbl);

    std::string labelDefault = "switch_" + std::to_string(lbl) + "_default";

    stm->condicion->accept(this);
    
    out << "  movq %rax, %r10\n"; 
    for (size_t i = 0; i < stm->casos.size(); ++i) {
        std::string labelCasoCuerpo = "switch_" + std::to_string(lbl) + "_case_" + std::to_string(i);
        
        stm->casos[i].first->accept(this); 
        
        out << "  cmpq %rax, %r10\n";
        
        out << "  je " << labelCasoCuerpo << "\n";
    }

    if (stm->default_caso != nullptr) {
        out << "  jmp " << labelDefault << "\n";
    } else {
        out << "  jmp endswitch_" << lbl << "\n";
    }

    for (size_t i = 0; i < stm->casos.size(); ++i) {
        out << "switch_" << lbl << "_case_" << std::to_string(i) << ":\n";
        
        stm->casos[i].second->accept(this); 
        out << "  jmp endswitch_" << lbl << "\n";
    }

    if (stm->default_caso != nullptr) {
        out << labelDefault << ":\n";
        stm->default_caso->accept(this);
    }

    out << "endswitch_" << lbl << ":\n";

    currentBreakLabel = oldBreak;
}

void GenCodeVisitor::visit(TryStmt* stm) {
    if (stm->expr) stm->expr->accept(this);
    if (stm->try_body) stm->try_body->accept(this);
}

//REVISAR, no entendi muy bn lo q hace defer
void GenCodeVisitor::visit(DeferStmt* stm) {
    deferStack.push_back(stm->stmt);
}

void GenCodeVisitor::visit(ForStmt* stm) {
    int lbl = labelcont++;
    
    std::string labelCondicion = "for_cond_" + std::to_string(lbl);
    std::string labelIncremento = "for_inc_" + std::to_string(lbl);
    std::string labelEnd        = "endfor_"    + std::to_string(lbl);

    std::string oldBreak = currentBreakLabel;
    std::string oldContinue = currentContinueLabel;
    
    currentBreakLabel = labelEnd;          
    currentContinueLabel = labelIncremento;

    if (stm->asignacion != nullptr) {
        stm->asignacion->accept(this);
    }

    out << labelCondicion << ":\n";

    if (stm->condicion != nullptr) {
        stm->condicion->accept(this);
        out << "  cmpq $0, %rax\n";
        out << "  je " << labelEnd << "\n"; 
    }

    if (stm->cuerpo != nullptr) {
        stm->cuerpo->accept(this);
    }

    out << labelIncremento << ":\n";
    if (stm->incremento != nullptr) {
        stm->incremento->accept(this);
    }

    out << "  jmp " << labelCondicion << "\n";

    out << labelEnd << ":\n";

    currentBreakLabel = oldBreak;
    currentContinueLabel = oldContinue;
}

//Este si no tengo ni idea d dnd esta en la gramatica
void GenCodeVisitor::visit(DerefAssignStmt* stm) {
    stm->rval->accept(this);
    out << "  pushq %rax\n";
    genAddress(stm->lval);
    out << "  movq %rax, %rcx\n";
    out << "  popq %rax\n";
    out << "  movq %rax, (%rcx)\n";
}

// -----------------------------------------------------------------------------
// type-gencode
// -----------------------------------------------------------------------------

//no generan codigo
void GenCodeVisitor::visit(IdType* tipo) {}
void GenCodeVisitor::visit(PointerType* tipo) {}
void GenCodeVisitor::visit(ArrayType* tipo) {}
void GenCodeVisitor::visit(OptionalType* tipo) {}
void GenCodeVisitor::visit(ErrorType* tipo) {}
void GenCodeVisitor::visit(UnionType* tipo) {}
void GenCodeVisitor::visit(EnumType* tipo) {}
