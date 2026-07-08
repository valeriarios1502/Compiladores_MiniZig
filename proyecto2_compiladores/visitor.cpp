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

    std::string stripPointerType(std::string typeName) {
        while (!typeName.empty() && typeName[0] == '*') {
            typeName = typeName.substr(1);
        }
        return typeName;
    }

    std::string globalSymbolName(const std::string& name) {
        return "g_" + name;
    }

    bool isStringLikeType(const std::string& typeName) {
        std::string normalized = stripPointerType(typeName);
        return normalized == "str" || normalized == "string" || normalized == "String";
    }

    bool isCharLikeType(const std::string& typeName) {
        std::string normalized = stripPointerType(typeName);
        return normalized == "char";
    }

    struct InlineContext {
        std::unordered_map<std::string, Fundec*> functions;
        std::unordered_set<std::string> inliningStack;
    };

    bool isIntegerLiteral(const Exp* expr, int& value) {
        if (const auto* number = dynamic_cast<const NumberExpDecimal*>(expr)) {
            value = number->value;
            return true;
        }
        return false;
    }

    bool isZero(const Exp* expr) {
        int value = 0;
        return isIntegerLiteral(expr, value) && value == 0;
    }

    bool isOne(const Exp* expr) {
        int value = 0;
        return isIntegerLiteral(expr, value) && value == 1;
    }

    Exp* makeIntLiteral(int value) {
        return new NumberExpDecimal(value);
    }

    Type* cloneType(Type* type) {
        if (!type) return nullptr;
        if (auto* id = dynamic_cast<IdType*>(type)) {
            return new IdType(id->id);
        }
        if (auto* ptr = dynamic_cast<PointerType*>(type)) {
            return new PointerType(cloneType(ptr->tipo));
        }
        if (auto* arr = dynamic_cast<ArrayType*>(type)) {
            return new ArrayType(arr->exp1, arr->exp2, cloneType(arr->tipo), arr->existe_exp2);
        }
        if (auto* opt = dynamic_cast<OptionalType*>(type)) {
            return new OptionalType(cloneType(opt->tipo));
        }
        if (auto* err = dynamic_cast<ErrorType*>(type)) {
            return new ErrorType(cloneType(err->tipo));
        }
        if (auto* unionType = dynamic_cast<UnionType*>(type)) {
            auto* copy = new UnionType(unionType->nombre);
            copy->campo_nombres = unionType->campo_nombres;
            copy->campo_tipos = std::vector<Type*>();
            for (Type* t : unionType->campo_tipos) copy->campo_tipos.push_back(cloneType(t));
            return copy;
        }
        if (auto* enumType = dynamic_cast<EnumType*>(type)) {
            auto* copy = new EnumType(enumType->nombre);
            copy->valores = enumType->valores;
            return copy;
        }
        return nullptr;
    }

    Exp* simplifyBinaryExp(Exp* left, Exp* right, BinaryOp op) {
        if (op == PLUS_OP) {
            if (isZero(left)) return right;
            if (isZero(right)) return left;
        } else if (op == MINUS_OP) {
            if (isZero(right)) return left;
        } else if (op == MUL_OP) {
            if (isZero(left) || isZero(right)) return makeIntLiteral(0);
            if (isOne(left)) return right;
            if (isOne(right)) return left;
        } else if (op == DIV_OP) {
            if (isOne(right)) return left;
            if (isZero(left)) return makeIntLiteral(0);
        } else if (op == MODULO_OP) {
            if (isOne(right)) return makeIntLiteral(0);
        }
        return nullptr;
    }

    Exp* cloneExpTree(Exp* expr) {
        if (!expr) return nullptr;

        if (auto* number = dynamic_cast<NumberExpDecimal*>(expr)) {
            return new NumberExpDecimal(number->value);
        }
        if (auto* floatNum = dynamic_cast<NumberExpFlotante*>(expr)) {
            return new NumberExpFlotante(floatNum->value);
        }
        if (auto* text = dynamic_cast<StringExp*>(expr)) {
            return new StringExp(text->valor);
        }
        if (auto* ch = dynamic_cast<CharExp*>(expr)) {
            return new CharExp(ch->valor);
        }
        if (auto* id = dynamic_cast<IdExp*>(expr)) {
            return new IdExp(id->value);
        }
        if (auto* boolean = dynamic_cast<BoolExp*>(expr)) {
            return new BoolExp();
        }
        if (auto* notExp = dynamic_cast<NotExp*>(expr)) {
            return new NotExp();
        }
        if (auto* call = dynamic_cast<FcallExp*>(expr)) {
            auto* copy = new FcallExp();
            copy->nombre = call->nombre;
            for (Exp* arg : call->argumentos) {
                copy->argumentos.push_back(cloneExpTree(arg));
            }
            return copy;
        }
        if (auto* unary = dynamic_cast<UnaryExp*>(expr)) {
            return new UnaryExp(unary->op, cloneExpTree(unary->exp));
        }
        if (auto* newExp = dynamic_cast<NewExp*>(expr)) {
            return new NewExp(cloneType(newExp->tipo));
        }
        if (auto* nul = dynamic_cast<NullExp*>(expr)) {
            return new NullExp();
        }
        if (auto* undef = dynamic_cast<UndefinedExp*>(expr)) {
            return new UndefinedExp();
        }
        if (auto* ref = dynamic_cast<ReferenceExp*>(expr)) {
            return new ReferenceExp();
        }
        if (auto* ptr = dynamic_cast<PunteroExp*>(expr)) {
            return new PunteroExp();
        }
        if (auto* listExp = dynamic_cast<AlgoconcorchetesylistaExp*>(expr)) {
            auto* copy = new AlgoconcorchetesylistaExp(nullptr, {});
            copy->nombre = cloneExpTree(listExp->nombre);
            for (Exp* arg : listExp->argumentos) {
                copy->argumentos.push_back(cloneExpTree(arg));
            }
            return copy;
        }
        if (auto* indexExp = dynamic_cast<AlgoconcorchetesExp*>(expr)) {
            return new AlgoconcorchetesExp(cloneExpTree(indexExp->nombre), cloneExpTree(indexExp->dentroexp));
        }
        if (auto* pointExp = dynamic_cast<PuntoExp*>(expr)) {
            return new PuntoExp(cloneExpTree(pointExp->exp), pointExp->id);
        }
        if (auto* lambda = dynamic_cast<LambdaExp*>(expr)) {
            std::vector<Type*> clonedTypes;
            for (Type* type : lambda->tipo_parametros) {
                clonedTypes.push_back(cloneType(type));
            }
            return new LambdaExp(cloneType(lambda->tipo), nullptr, lambda->id_parametros, clonedTypes);
        }
        if (auto* bin = dynamic_cast<BinaryExp*>(expr)) {
            return new BinaryExp(cloneExpTree(bin->left), cloneExpTree(bin->right), bin->op);
        }

        return nullptr;
    }

    Exp* substituteExp(Exp* expr, const std::unordered_map<std::string, Exp*>& substitutions) {
        if (!expr) return nullptr;

        if (auto* id = dynamic_cast<IdExp*>(expr)) {
            auto it = substitutions.find(id->value);
            if (it != substitutions.end()) {
                return cloneExpTree(it->second);
            }
            return new IdExp(id->value);
        }

        if (auto* bin = dynamic_cast<BinaryExp*>(expr)) {
            Exp* left = substituteExp(bin->left, substitutions);
            Exp* right = substituteExp(bin->right, substitutions);
            return new BinaryExp(left, right, bin->op);
        }
        if (auto* unary = dynamic_cast<UnaryExp*>(expr)) {
            return new UnaryExp(unary->op, substituteExp(unary->exp, substitutions));
        }
        if (auto* notExp = dynamic_cast<NotExp*>(expr)) {
            return new NotExp();
        }
        if (auto* call = dynamic_cast<FcallExp*>(expr)) {
            auto* copy = new FcallExp();
            copy->nombre = call->nombre;
            for (Exp* arg : call->argumentos) {
                copy->argumentos.push_back(substituteExp(arg, substitutions));
            }
            return copy;
        }
        if (auto* number = dynamic_cast<NumberExpDecimal*>(expr)) {
            return new NumberExpDecimal(number->value);
        }
        if (auto* floatNum = dynamic_cast<NumberExpFlotante*>(expr)) {
            return new NumberExpFlotante(floatNum->value);
        }
        if (auto* text = dynamic_cast<StringExp*>(expr)) {
            return new StringExp(text->valor);
        }
        if (auto* ch = dynamic_cast<CharExp*>(expr)) {
            return new CharExp(ch->valor);
        }
        if (auto* boolean = dynamic_cast<BoolExp*>(expr)) {
            return new BoolExp();
        }
        if (auto* nul = dynamic_cast<NullExp*>(expr)) {
            return new NullExp();
        }
        if (auto* undef = dynamic_cast<UndefinedExp*>(expr)) {
            return new UndefinedExp();
        }
        if (auto* ref = dynamic_cast<ReferenceExp*>(expr)) {
            return new ReferenceExp();
        }
        if (auto* ptr = dynamic_cast<PunteroExp*>(expr)) {
            return new PunteroExp();
        }
        if (auto* listExp = dynamic_cast<AlgoconcorchetesylistaExp*>(expr)) {
            auto* copy = new AlgoconcorchetesylistaExp(nullptr, {});
            copy->nombre = substituteExp(listExp->nombre, substitutions);
            for (Exp* arg : listExp->argumentos) {
                copy->argumentos.push_back(substituteExp(arg, substitutions));
            }
            return copy;
        }
        if (auto* indexExp = dynamic_cast<AlgoconcorchetesExp*>(expr)) {
            return new AlgoconcorchetesExp(substituteExp(indexExp->nombre, substitutions), substituteExp(indexExp->dentroexp, substitutions));
        }
        if (auto* pointExp = dynamic_cast<PuntoExp*>(expr)) {
            return new PuntoExp(substituteExp(pointExp->exp, substitutions), pointExp->id);
        }
        if (auto* lambda = dynamic_cast<LambdaExp*>(expr)) {
            std::vector<Type*> clonedTypes;
            for (Type* type : lambda->tipo_parametros) {
                clonedTypes.push_back(cloneType(type));
            }
            return new LambdaExp(cloneType(lambda->tipo), nullptr, lambda->id_parametros, clonedTypes);
        }
        return nullptr;
    }

    bool isSimpleFunction(Fundec* fundec) {
        if (!fundec || !fundec->cuerpo || fundec->cuerpo->slist.size() != 1) return false;
        auto it = fundec->cuerpo->slist.begin();
        auto* ret = dynamic_cast<ReturnStm*>(*it);
        return ret != nullptr && ret->exp != nullptr;
    }

    Exp* rewriteExp(Exp* expr, InlineContext& ctx) {
        if (!expr) return nullptr;

        if (auto* call = dynamic_cast<FcallExp*>(expr)) {
            std::vector<Exp*> rewrittenArgs;
            rewrittenArgs.reserve(call->argumentos.size());
            for (Exp* arg : call->argumentos) {
                rewrittenArgs.push_back(rewriteExp(arg, ctx));
            }

            auto it = ctx.functions.find(call->nombre);
            if (it != ctx.functions.end() && !ctx.inliningStack.count(call->nombre) &&
                isSimpleFunction(it->second) && rewrittenArgs.size() == it->second->id_parametros.size()) {
                ctx.inliningStack.insert(call->nombre);
                std::unordered_map<std::string, Exp*> substitutions;
                for (size_t i = 0; i < rewrittenArgs.size(); ++i) {
                    substitutions[it->second->id_parametros[i]] = rewrittenArgs[i];
                }

                auto itStmt = it->second->cuerpo->slist.begin();
                auto* ret = dynamic_cast<ReturnStm*>(*itStmt);
                Exp* inlined = nullptr;
                if (ret && ret->exp) {
                    inlined = substituteExp(ret->exp, substitutions);
                }

                ctx.inliningStack.erase(call->nombre);
                return inlined ? inlined : makeIntLiteral(0);
            }

            auto* copy = new FcallExp();
            copy->nombre = call->nombre;
            copy->argumentos = std::move(rewrittenArgs);
            return copy;
        }

        if (auto* bin = dynamic_cast<BinaryExp*>(expr)) {
            Exp* left = rewriteExp(bin->left, ctx);
            Exp* right = rewriteExp(bin->right, ctx);

            if (Exp* simplified = simplifyBinaryExp(left, right, bin->op)) {
                return simplified;
            }
            return new BinaryExp(left, right, bin->op);
        }

        if (auto* unary = dynamic_cast<UnaryExp*>(expr)) {
            Exp* inner = rewriteExp(unary->exp, ctx);
            return new UnaryExp(unary->op, inner);
        }

        if (auto* notExp = dynamic_cast<NotExp*>(expr)) {
            rewriteExp(notExp->exp, ctx);
            return new NotExp();
        }

        if (auto* listExp = dynamic_cast<AlgoconcorchetesylistaExp*>(expr)) {
            Exp* base = rewriteExp(listExp->nombre, ctx);
            std::vector<Exp*> rewrittenArgs;
            rewrittenArgs.reserve(listExp->argumentos.size());
            for (Exp* arg : listExp->argumentos) {
                rewrittenArgs.push_back(rewriteExp(arg, ctx));
            }
            auto* copy = new AlgoconcorchetesylistaExp(base, {});
            copy->argumentos = std::move(rewrittenArgs);
            return copy;
        }

        if (auto* indexExp = dynamic_cast<AlgoconcorchetesExp*>(expr)) {
            Exp* base = rewriteExp(indexExp->nombre, ctx);
            Exp* inner = rewriteExp(indexExp->dentroexp, ctx);
            return new AlgoconcorchetesExp(base, inner);
        }

        if (auto* pointExp = dynamic_cast<PuntoExp*>(expr)) {
            Exp* base = rewriteExp(pointExp->exp, ctx);
            return new PuntoExp(base, pointExp->id);
        }

        return cloneExpTree(expr);
    }

    void rewriteStmt(Stmt* stmt, InlineContext& ctx) {
        if (!stmt) return;

        if (auto* asgn = dynamic_cast<AsignStmt*>(stmt)) {
            asgn->exp = rewriteExp(asgn->exp, ctx);
        } else if (auto* print = dynamic_cast<PrintStmt*>(stmt)) {
            print->exp = rewriteExp(print->exp, ctx);
        } else if (auto* ret = dynamic_cast<ReturnStm*>(stmt)) {
            ret->exp = rewriteExp(ret->exp, ctx);
        } else if (auto* del = dynamic_cast<DeleteStm*>(stmt)) {
            del->exp = rewriteExp(del->exp, ctx);
        } else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
            ifStmt->condicion = rewriteExp(ifStmt->condicion, ctx);
            rewriteStmt(new BodyStmt(ifStmt->cuerpodelif), ctx);
            if (ifStmt->hayelse) rewriteStmt(new BodyStmt(ifStmt->cuerpodelelse), ctx);
        } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
            whileStmt->condicion = rewriteExp(whileStmt->condicion, ctx);
            for (Stmt* inner : whileStmt->cuerpodelwhile) {
                rewriteStmt(inner, ctx);
            }
        } else if (auto* bodyStmt = dynamic_cast<BodyStmt*>(stmt)) {
            if (bodyStmt->cuerpo) {
                for (Stmt* inner : bodyStmt->cuerpo->slist) {
                    rewriteStmt(inner, ctx);
                }
            }
        } else if (auto* switchStmt = dynamic_cast<SwitchStmt*>(stmt)) {
            switchStmt->condicion = rewriteExp(switchStmt->condicion, ctx);
            for (auto& caso : switchStmt->casos) {
                caso.first = rewriteExp(caso.first, ctx);
                if (caso.second) {
                    for (Stmt* inner : caso.second->slist) {
                        rewriteStmt(inner, ctx);
                    }
                }
            }
            if (switchStmt->default_caso) {
                for (Stmt* inner : switchStmt->default_caso->slist) {
                    rewriteStmt(inner, ctx);
                }
            }
        } else if (auto* tryStmt = dynamic_cast<TryStmt*>(stmt)) {
            tryStmt->expr = rewriteExp(tryStmt->expr, ctx);
            if (tryStmt->try_body) {
                for (Stmt* inner : tryStmt->try_body->slist) {
                    rewriteStmt(inner, ctx);
                }
            }
            if (tryStmt->catch_body) {
                for (Stmt* inner : tryStmt->catch_body->slist) {
                    rewriteStmt(inner, ctx);
                }
            }
        } else if (auto* deferStmt = dynamic_cast<DeferStmt*>(stmt)) {
            rewriteStmt(deferStmt->stmt, ctx);
        } else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt)) {
            if (forStmt->asignacion) rewriteStmt(forStmt->asignacion, ctx);
            if (forStmt->condicion) forStmt->condicion = rewriteExp(forStmt->condicion, ctx);
            if (forStmt->incremento) rewriteStmt(forStmt->incremento, ctx);
            if (forStmt->cuerpo) {
                for (Stmt* inner : forStmt->cuerpo->slist) {
                    rewriteStmt(inner, ctx);
                }
            }
        }
    }

    void rewriteTopDec(Top_dec* dec, InlineContext& ctx) {
        if (auto* fundec = dynamic_cast<Fundec*>(dec)) {
            if (!fundec->cuerpo) return;
            ctx.inliningStack.insert(fundec->nombre);
            for (Stmt* inner : fundec->cuerpo->slist) {
                rewriteStmt(inner, ctx);
            }
            ctx.inliningStack.erase(fundec->nombre);
        }
    }
}

void opt(Programa* program) {
    if (!program) return;

    InlineContext ctx;
    for (Top_dec* dec : program->declist) {
        if (auto* fundec = dynamic_cast<Fundec*>(dec)) {
            ctx.functions[fundec->nombre] = fundec;
        }
    }

    for (Top_dec* dec : program->declist) {
        rewriteTopDec(dec, ctx);
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

int GenCodeVisitor::elementSizeBytes(Type* tipo) const {
    return 8;
}

void GenCodeVisitor::emitArrayElementCount(ArrayType* tipo) {
    if (tipo->exp1) {
        tipo->exp1->accept(this);
    } else {
        out << "movq $1, %rax\n";
    }

    if (tipo->existe_exp2 && tipo->exp2) {
        out << "pushq %rax\n";
        tipo->exp2->accept(this);
        out << "movq %rax, %rcx\n";
        out << "popq %rax\n";
        out << "imulq %rcx, %rax\n";
    }

    if (auto innerArray = dynamic_cast<ArrayType*>(tipo->tipo)) {
        out << "pushq %rax\n";
        emitArrayElementCount(innerArray);
        out << "movq %rax, %rcx\n";
        out << "popq %rax\n";
        out << "imulq %rcx, %rax\n";
    }
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
    out << "subq $32, %rsp\n";
#endif
    out << "call " << nombre << "\n";
#ifdef _WIN32
    out << "addq $32, %rsp\n";
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
        out << "pushq %rax\n";
        idx->dentroexp->accept(this);
        out << "movq %rax, %rcx\n";
        out << "popq %rax\n";
        out << "leaq (%rax,%rcx,8), %rax\n";
        return;
    } else if (auto idx = dynamic_cast<AlgoconcorchetesExp*>(lval)) {
        idx->nombre->accept(this);
        out << "pushq %rax\n";
        idx->dentroexp->accept(this);
        out << "movq %rax, %rcx\n";
        out << "popq %rax\n";
        out << "leaq (%rax,%rcx,8), %rax\n";
    } else if (auto p = dynamic_cast<PuntoExp*>(lval)) {
        p->exp->accept(this);
        int off = 0;
        std::string baseType = lastTypeName;
        if (auto id = dynamic_cast<IdExp*>(p->exp)) {
            auto typeIt = variableTypes.find(id->value);
            if (typeIt != variableTypes.end()) baseType = stripPointerType(typeIt->second);
        }
        if (!baseType.empty() && unionFieldOffsets.count(baseType)) {
            auto& campos = unionFieldOffsets[baseType];
            auto it = campos.find(p->id);
            if (it != campos.end()) off = it->second;
        } else if (!baseType.empty() && structFieldOffsets.count(baseType)) {
            auto& campos = structFieldOffsets[baseType];
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

    bool getConstFloat(Exp* e, double& out) {
        if (auto n = dynamic_cast<NumberExpFlotante*>(e)) { out = n->value; return true; }
        if (auto n = dynamic_cast<NumberExpDecimal*>(e))  { out = (double)n->value; return true; }
        return false;
    }

    bool getConstChar(Exp* e, char& out) {
        if (auto n = dynamic_cast<CharExp*>(e)) { out = n->valor; return true; }
        return false;
    }

    bool getConstString(Exp* e, std::string& out) {
        if (auto n = dynamic_cast<StringExp*>(e)) { out = n->valor; return true; }
        return false;
    }
    bool getConstBool(Exp* e, bool& out) {
        if (auto n = dynamic_cast<BoolExp*>(e)) { out = (n->booleano == "true"); return true; }
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

namespace {
    bool getIndexedIdAndConst(Exp* exp, std::string& nombre, int& index) {
        auto idx = dynamic_cast<AlgoconcorchetesExp*>(exp);
        if (!idx) return false;

        auto id = dynamic_cast<IdExp*>(idx->nombre);
        if (!id) return false;

        int value = 0;
        if (!getConstInt(idx->dentroexp, value)) return false;

        nombre = id->value;
        index = value;
        return true;
    }

    void noteDynamicArrayIndex(std::unordered_map<std::string,int>& sizes,
                               const std::string& nombre,
                               int index) {
        if (index < 0) return;
        int needed = index + 1;
        auto it = sizes.find(nombre);
        if (it == sizes.end() || needed > it->second) sizes[nombre] = needed;
    }

    void recolectarDynamicArrays(Stmt* s, std::unordered_map<std::string,int>& sizes);

    void recolectarDynamicArraysBody(Body* b, std::unordered_map<std::string,int>& sizes) {
        if (!b) return;
        for (auto s : b->slist) recolectarDynamicArrays(s, sizes);
    }

    void recolectarDynamicArrays(Stmt* s, std::unordered_map<std::string,int>& sizes) {
        if (!s) return;

        if (auto d = dynamic_cast<DerefAssignStmt*>(s)) {
            std::string nombre;
            int index = 0;
            if (getIndexedIdAndConst(d->lval, nombre, index)) noteDynamicArrayIndex(sizes, nombre, index);
        } else if (auto i = dynamic_cast<IfStmt*>(s)) {
            recolectarDynamicArraysBody(i->cuerpodelif, sizes);
            if (i->hayelse) recolectarDynamicArraysBody(i->cuerpodelelse, sizes);
        } else if (auto w = dynamic_cast<WhileStmt*>(s)) {
            for (auto st : w->cuerpodelwhile) recolectarDynamicArrays(st, sizes);
        } else if (auto f = dynamic_cast<ForStmt*>(s)) {
            if (f->asignacion) recolectarDynamicArrays(f->asignacion, sizes);
            if (f->incremento) recolectarDynamicArrays(f->incremento, sizes);
            recolectarDynamicArraysBody(f->cuerpo, sizes);
        } else if (auto bs = dynamic_cast<BodyStmt*>(s)) {
            recolectarDynamicArraysBody(bs->cuerpo, sizes);
        } else if (auto sw = dynamic_cast<SwitchStmt*>(s)) {
            for (auto& pr : sw->casos) recolectarDynamicArraysBody(pr.second, sizes);
            if (sw->default_caso) recolectarDynamicArraysBody(sw->default_caso, sizes);
        } else if (auto t = dynamic_cast<TryStmt*>(s)) {
            recolectarDynamicArraysBody(t->try_body, sizes);
            recolectarDynamicArraysBody(t->catch_body, sizes);
        } else if (auto d = dynamic_cast<DeferStmt*>(s)) {
            recolectarDynamicArrays(d->stmt, sizes);
        }
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

            std::unordered_map<std::string,int> dynamicArrsLocal;
            recolectarDynamicArraysBody(f->cuerpo, dynamicArrsLocal);
            for (auto& kv : dynamicArrsLocal) {
                dynamicArraySizes[f->nombre + "::" + kv.first] = kv.second;
            }
        }
        else if (Template* t = dynamic_cast<Template*>(dec)) {  
            std::string nombreCompleto = t->id1;
            std::unordered_set<std::string> vars;
            for (auto& p : t->id_parametros) vars.insert(p);
            recolectarVarsBody(t->block, vars);
            funcontador[nombreCompleto] = (int)vars.size();

            std::unordered_map<std::string,int> dynamicArrsLocal;
            recolectarDynamicArraysBody(t->block, dynamicArrsLocal);
            for (auto& kv : dynamicArrsLocal) {
                dynamicArraySizes[nombreCompleto + "::" + kv.first] = kv.second;
            }

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
    bool hasMain = false;
    for (auto dec : program->declist) {
        if (auto fundec = dynamic_cast<Fundec*>(dec)) {
            if (fundec->nombre == "main") {
                hasMain = true;
                break;
            }
        }
    }

    out << ".data\n";
    out << "print_int_fmt: .string \"%ld\\n\"\n";
    out << "print_str_fmt: .string \"%s\\n\"\n";
    out << "print_char_fmt: .string \"%c\\n\"\n";   // NUEVO
    out << "\n.text\n";

    for (auto dec : program->declist)
        dec->accept(this);

    if (!hasMain) {
        out << "\n.globl main\n";
        out << "main:\n";
        out << "pushq %rbp\n";
        out << "movq %rsp, %rbp\n";
        out << "movq $0, %rax\n";
        out << "leave\n";
        out << "ret\n";
    }

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
    out << "pushq %rbp\n";
    out << "movq %rsp, %rbp\n";

    int bytes = alignStackBytes(funcontador[nombreCompleto] * 8 + 8);
    callScratchOffset = 0;

    out << "subq $" << bytes << ", %rsp\n";

    for (size_t i = 0; i < t->id_parametros.size() && i < maxRegisterArgs(); i++) {
        int slot = getLocalSlot(t->id_parametros[i]);
        out << "movq " << argRegister(i) << ", " << (-8 * slot) << "(%rbp)\n";
    }

    if (t->block) t->block->accept(this);

    emitirDefers();
    out << "end_" << nombreCompleto << ":\n";
    out << "movq $0, %rax\n";
    out << "leave\n";
    out << "ret\n";
}

void GenCodeVisitor::visit(ConstDec *c) {
    globales[c->nombre] = true;
    globalNames[c->nombre] = c->nombre;

    bool esString = false;
    std::string valorStr;

    if (c->exp != nullptr) {
        esString = getConstString(c->exp, valorStr);
    }

    if (esString) {
        std::string labelLit = c->nombre + "_str";
        emitStringData(labelLit, valorStr);

        out << ".data\n";
        out << ".global " << c->nombre << "\n";
        out << c->nombre << ":\n";
        out << "    .quad " << labelLit << "\n";
        out << ".text\n";
    } else {
        long valor = 0;

        if (c->exp != nullptr) {
            int vi; double vf; char vc; bool vb;
            if (getConstInt(c->exp, vi))         valor = vi;
            else if (getConstFloat(c->exp, vf))  valor = (long)vf;
            else if (getConstChar(c->exp, vc))   valor = (long)vc;
            else if (getConstBool(c->exp, vb))   valor = vb ? 1 : 0;
        }

        out << ".data\n";
        out << ".global " << c->nombre << "\n";
        out << c->nombre << ":\n";
        out << "    .quad " << valor << "\n";
        out << ".text\n";
    }
}

void GenCodeVisitor::visit(VarDec *v) {
    if (auto unionType = dynamic_cast<UnionType*>(v->tipo)) {
        std::unordered_map<std::string,int> offsets;
        for (const auto& campo : unionType->campo_nombres)
            offsets[campo] = 0;
        unionFieldOffsets[unionType->nombre] = offsets;
        unionSizes[unionType->nombre] = 8;
        return;
    }

    std::string tipoDeclarado;
    if (v->tipo) {
        v->tipo->accept(this);
        tipoDeclarado = lastTypeName;
    }
    variableTypes[v->nombre] = tipoDeclarado;

    bool esString = isStringLikeType(tipoDeclarado);

    if (!this->dentroDeFuncion) {
        globales[v->nombre] = true;
        globalNames[v->nombre] = "g_" + v->nombre;

        if (esString) {
            std::string valorStr;
            if (v->exp != nullptr) getConstString(v->exp, valorStr);

            std::string labelLit = "g_" + v->nombre + "_str";
            emitStringData(labelLit, valorStr); 

            out << ".data\n";
            out << ".global g_" << v->nombre << "\n";
            out << "g_" << v->nombre << ":\n";
            out << "    .quad " << labelLit << "\n"; 
            out << ".text\n";
        }
        else {
    long valor = 0;

    if (v->exp != nullptr) {
        int vi; double vf; char vc; bool vb;
        if (getConstInt(v->exp, vi))         valor = vi;
        else if (getConstFloat(v->exp, vf))  valor = (long)vf;
        else if (getConstChar(v->exp, vc))   valor = (long)vc;
        else if (getConstBool(v->exp, vb))   valor = vb ? 1 : 0; 
    }

    out << ".data\n";
    out << ".global g_" << v->nombre << "\n";
    out << "g_" << v->nombre << ":\n";
    out << "    .quad " << valor << "\n";
    out << ".text\n";
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
            out << "movq %rax, " << offset << "(%rbp)\n"; 
        } else {
            out << "movq $0, " << offset << "(%rbp)\n"; 
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
    out << "pushq %rbp\n";
    out << "movq %rsp, %rbp\n";

    int bytes = alignStackBytes(funcontador[dec->nombre] * 8 + 8);
    callScratchOffset = 0;

    out << "subq $" << bytes << ", %rsp\n";

    for (size_t i = 0; i < dec->id_parametros.size() && i < maxRegisterArgs(); i++) {
        int slot = getLocalSlot(dec->id_parametros[i]);
        out << "movq " << argRegister(i) << ", " << (-8 * slot) << "(%rbp)\n";
    }

    if (dec->cuerpo) dec->cuerpo->accept(this);

    emitirDefers();
    out << "end_" << dec->nombre << ":" << endl;
    out << "movq $0, %rax\n"; 
    out << "leave\n";       
    out << "ret\n";
}

// -----------------------------------------------------------------------------
// exp-gencode
// -----------------------------------------------------------------------------

Value GenCodeVisitor::visit(BinaryExp *exp) {

     if (exp->isConstant) {
        out << "movq $" << exp->constantValue << ", %rax\n";
        return Value::makeInt(0);   // <-- FALTA ESTO
    }

    exp->left->accept(this);
    out << "pushq %rax\n";
    exp->right->accept(this);
    out << "movq %rax, %rcx\n";
    out << "popq %rax\n";

    switch (exp->op) {
    case PLUS_OP:  out << "addq %rcx, %rax\n";  break;
    case MINUS_OP: out << "subq %rcx, %rax\n";  break;
    case MUL_OP:   out << "imulq %rcx, %rax\n"; break;
    case DIV_OP:
            out << "cqto\n";             
            out << "idivq %rcx\n";       
            break;
            
    case MODULO_OP:
            out << "cqto\n";
            out << "idivq %rcx\n";        
            out << "movq %rdx, %rax\n";   
            break;

    case IGUALIGUAL:
        out << "cmpq %rcx, %rax\n";
        out << "movq $0, %rax\n";
        out << "sete %al\n";
        out << "movzbq %al, %rax\n";
        break;

    case DIFERENTE_OP:
        out << "cmpq %rcx, %rax\n";
        out << "movq $0, %rax\n";
        out << "setne %al\n";
        out << "movzbq %al, %rax\n";
        break;

    case MENOR:
        out << "cmpq %rcx, %rax\n";
        out << "movq $0, %rax\n";
        out << "setl %al\n";
        out << "movzbq %al, %rax\n";
        break;

    case MENORI:
        out << "cmpq %rcx, %rax\n";
        out << "movq $0, %rax\n";
        out << "setle %al\n";
        out << "movzbq %al, %rax\n";
        break;

    case MAYOR:
        out << "cmpq %rcx, %rax\n";
        out << "movq $0, %rax\n";
        out << "setg %al\n";
        out << "movzbq %al, %rax\n";
        break;

    case MAYORI:
        out << "cmpq %rcx, %rax\n";
        out << "movq $0, %rax\n";
        out << "setge %al\n";
        out << "movzbq %al, %rax\n";
        break;

    case AND: out << "andq %rcx, %rax\n"; break;
    case OR:  out << "orq %rcx, %rax\n";  break;

    default:
        break;
    }
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NumberExpDecimal *exp) {
    out << "movq $" << exp->value << ", %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NumberExpFlotante *exp) {
    out << "movq $" << (long)exp->value << ", %rax" << endl;
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(StringExp *exp) {
    std::string strLabel = "str_" + std::to_string(labelcont++);
    emitStringData(strLabel, exp->valor);
    out << "leaq " << strLabel << "(%rip), %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(CharExp *exp) {
    out << "movq $0, %rax\n";
    out << "movb $" << (int)exp->valor << ", %al\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(IdExp *exp) {
    auto typeIt = variableTypes.find(exp->value);
    if (typeIt != variableTypes.end()) {
        lastTypeName = stripPointerType(typeIt->second);
    }

    if (posicion.count(exp->value)) {
        int varOffset = posicion[exp->value];
        out << "movq " << (-8 * varOffset) << "(%rbp), %rax\n";
    } 
    else if (globales.count(exp->value)) {
        out << "movq " << globalNames[exp->value] << "(%rip), %rax\n";
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
            out << "cmpq $0, %rax\n";
            out << "movq $0, %rax\n";
            out << "sete %al\n";
            out << "movzbq %al, %rax\n";
            break;

        case UnaryExp::NEGATE: 
            out << "negq %rax\n";
            break;

        case UnaryExp::DEREF: 
            out << "movq (%rax), %rax\n"; 
            break;

        case UnaryExp::ADDRESS: 
            IdExp* id = dynamic_cast<IdExp*>(exp->exp);
            if (id != nullptr) {
                if (posicion.count(id->value)) {
                    out << "leaq " << (-8 * posicion[id->value]) << "(%rbp), %rax\n";
                } else if (globales.count(id->value)) {
                    out << "leaq " << globalNames[id->value] << "(%rip), %rax\n";
                }
            }
            break;
    }
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NewExp *e) {
    this->lastTypeName = "";

    if (auto arrayType = dynamic_cast<ArrayType*>(e->tipo)) {
        emitArrayElementCount(arrayType);
        out << "imulq $" << elementSizeBytes(arrayType->tipo) << ", %rax\n";
        out << "movq %rax, " << argRegister(0) << "\n";
        emitCall("malloc");
        return Value::makeInt(0);
    }

    if (e->tipo != nullptr) {
        e->tipo->accept(this);
    }

    int tamanoBytes = 8; 

    if (!this->lastTypeName.empty() && unionSizes.count(this->lastTypeName)) {
        tamanoBytes = unionSizes[this->lastTypeName];
    } else if (!this->lastTypeName.empty() && structFields.count(this->lastTypeName)) {
        int numeroCampos = structFields[this->lastTypeName];
        tamanoBytes = numeroCampos * 8;
    }
    out << "movq $" << tamanoBytes << ", " << argRegister(0) << "\n";
    emitCall("malloc");
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NullExp *e) {
    out << "movq $0, %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(UndefinedExp *e) {
    out << "movq $0, %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(ReferenceExp* exp) {
    if (auto id = dynamic_cast<IdExp*>(exp->exp)) {
        if (posicion.count(id->value))
            out << "leaq " << (-8 * posicion[id->value]) << "(%rbp), %rax" << endl;
        else if (globales.count(id->value))
            out << "leaq " << globalNames[id->value] << "(%rip), %rax" << endl;
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
    out << "movq %rax, %r10\n";

    int offsetCampo = 0;
    std::string baseType = lastTypeName;
    if (auto id = dynamic_cast<IdExp*>(e->exp)) {
        auto typeIt = variableTypes.find(id->value);
        if (typeIt != variableTypes.end()) baseType = stripPointerType(typeIt->second);
    }

    if (!baseType.empty() && unionFieldOffsets.count(baseType)) {
        auto& campos = unionFieldOffsets[baseType];
        auto it = campos.find(e->id);
        if (it != campos.end()) offsetCampo = it->second;
    } else if (!baseType.empty() && structFieldOffsets.count(baseType)) {
        auto& campos = structFieldOffsets[baseType];
        auto it = campos.find(e->id);
        if (it != campos.end()) offsetCampo = it->second;
    }

    out << "movq " << offsetCampo << "(%r10), %rax\n";
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
            out << "pushq %rax\n";
        } else {
            out << "pushq $0\n";
        }
    }

    for (size_t i = 0; i < n; i++) {
        out << "popq " << argRegister(i) << "\n";
    }
    emitCall(e->nombre);
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(LambdaExp *e) {
    int lbl = labelcont++;
    
    std::string nombreLambda = "lambda_func_" + std::to_string(lbl);
    std::string labelLambdaSkip = "lambda_skip_" + std::to_string(lbl);

    out << "jmp " << labelLambdaSkip << "\n";

    out << ".globl " << nombreLambda << "\n";
    out << nombreLambda << ":\n";
    out << "pushq %rbp\n";
    out << "movq %rsp, %rbp\n";

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

    out << "subq $" << bytes << ", %rsp\n";

    if (e->hayparametros) {
        for (size_t i = 0; i < e->id_parametros.size() && i < maxRegisterArgs(); i++) {
            int slot = getLocalSlot(e->id_parametros[i]);
            out << "movq " << argRegister(i) << ", " << (-8 * slot) << "(%rbp)\n";
        }
    }

    if (e->cuerpo) e->cuerpo->accept(this);

    out << "end_" << nombreLambda << ":\n";
    out << "movq $0, %rax\n"; 
    out << "leave\n";       
    out << "ret\n";

    out << labelLambdaSkip << ":\n";

    funcionActual = funcionPadre;
    callScratchOffset = scratchPadre;

    out << "    leaq " << nombreLambda << "(%rip), %rax\n";
    return Value::makeInt(0);
}

Value GenCodeVisitor::visit(NotExp *exp) {
    if (exp->exp) exp->exp->accept(this);
    out << "cmpq $0, %rax\n";
    out << "movq $0, %rax\n";
    out << "sete %al\n";
    out << "movzbq %al, %rax\n";
    return Value::makeInt(0);
}

// -----------------------------------------------------------------------------
// stm-gencode
// -----------------------------------------------------------------------------

void GenCodeVisitor::visit(IfStmt *stm) {
    int lbl = labelcont++;
    stm->condicion->accept(this);
    out << " cmpq $0, %rax\n";
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

    if (stm->tipoDeclarado) {
        stm->tipoDeclarado->accept(this);
        variableTypes[stm->variable] = lastTypeName;
    } else if (auto newExp = dynamic_cast<NewExp*>(stm->exp)) {
        newExp->tipo->accept(this);
        variableTypes[stm->variable] = "*" + lastTypeName;
    }

    bool expEmitida = false;
    if (auto newExp = dynamic_cast<NewExp*>(stm->exp)) {
        if (!dynamic_cast<ArrayType*>(newExp->tipo)) {
            auto sizeIt = dynamicArraySizes.find(funcionActual + "::" + stm->variable);
            if (sizeIt != dynamicArraySizes.end()) {
                out << "    movq $" << (sizeIt->second * 8) << ", " << argRegister(0) << "\n";
                emitCall("malloc");
                expEmitida = true;
            }
        }
    }

    if (!expEmitida) stm->exp->accept(this);
    if (globales.count(stm->variable) && !posicion.count(stm->variable)) {
        out << "movq %rax, " << globalNames[stm->variable] << "(%rip)" << endl;
    } else {
        int slot = getLocalSlot(stm->variable);
        out << "movq %rax, " << (-8 * slot) << "(%rbp)" << endl;
    }
}

void GenCodeVisitor::visit(PrintStmt* stm) {
    lastTypeName.clear();
    stm->exp->accept(this);

    bool printAsString = dynamic_cast<StringExp*>(stm->exp) != nullptr || isStringLikeType(lastTypeName);
    bool printAsChar   = dynamic_cast<CharExp*>(stm->exp) != nullptr   || isCharLikeType(lastTypeName);

    out << "movq %rax, " << argRegister(1) << endl;

    if (printAsString) {
        out << "leaq print_str_fmt(%rip), " << argRegister(0) << endl;
    } else if (printAsChar) {
        out << "leaq print_char_fmt(%rip), " << argRegister(0) << endl;
    } else {
        out << "leaq print_int_fmt(%rip), " << argRegister(0) << endl;
    }

    out << "movl $0, %eax" << endl;
    emitCall("printf");
}

void GenCodeVisitor::visit(ReturnStm* stm) {
    if (stm->exp) stm->exp->accept(this);
    else out << "movq $0, %rax" << endl;

    out << "pushq %rax\n";
    emitirDefers();
    out << "popq %rax\n";

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
    out << "pushq %rax\n";
    genAddress(stm->lval);
    out << "movq %rax, %rcx\n";
    out << "popq %rax\n";
    out << "movq %rax, (%rcx)\n";
}

// -----------------------------------------------------------------------------
// type-gencode
// -----------------------------------------------------------------------------

//no generan codigo
void GenCodeVisitor::visit(IdType* tipo) {
    lastTypeName = tipo->id;
}

void GenCodeVisitor::visit(PointerType* tipo) {
    if (tipo->tipo) tipo->tipo->accept(this);
    lastTypeName = "*" + lastTypeName;
}

void GenCodeVisitor::visit(ArrayType* tipo) {
    if (tipo->tipo) tipo->tipo->accept(this);
}

void GenCodeVisitor::visit(OptionalType* tipo) {
    if (tipo->tipo) tipo->tipo->accept(this);
}

void GenCodeVisitor::visit(ErrorType* tipo) {
    if (tipo->tipo) tipo->tipo->accept(this);
}

void GenCodeVisitor::visit(UnionType* tipo) {
    lastTypeName = tipo->nombre;
}

void GenCodeVisitor::visit(EnumType* tipo) {
    lastTypeName = tipo->nombre;
}

Value Op1Visitor::Opt1(Programa *program){
    program->accept(this);
    return Value::makeInt(0); 

}
Value Op1Visitor::visit(BinaryExp* exp){
    exp->left->accept(this);
    exp->right->accept(this);
    exp->isConstant = exp->left->isConstant && exp->right->isConstant;
    if (exp->isConstant){
       switch (exp->op) {
      case PLUS_OP:
       exp->constantValue = exp->left->constantValue + exp->right->constantValue;
       break;
      case MINUS_OP:
       exp->constantValue = exp->left->constantValue - exp->right->constantValue;
       break;
      case MUL_OP:
       exp->constantValue = exp->left->constantValue * exp->right->constantValue;
       break;
      case MODULO_OP:
       exp->constantValue = exp->left->constantValue % exp->right->constantValue;
       break;
      case DIV_OP:
         exp->constantValue = exp->left->constantValue / exp->right->constantValue;
         break;
      case IGUALIGUAL:
       exp->constantValue = exp->left->constantValue == exp->right->constantValue ? 1 : 0;
       break;
      case DIFERENTE_OP:
       exp->constantValue = exp->left->constantValue != exp->right->constantValue ? 1 : 0;
       break;
      case MENOR:
       exp->constantValue = exp->left->constantValue < exp->right->constantValue ? 1 : 0;
       break;
      case MENORI:
       exp->constantValue = exp->left->constantValue <= exp->right->constantValue ? 1 : 0;
       break;
      case MAYOR:
       exp->constantValue = exp->left->constantValue > exp->right->constantValue ? 1 : 0;
       break;
      case MAYORI:
       exp->constantValue = exp->left->constantValue >= exp->right->constantValue ? 1 : 0;
       break;
      case AND:
       exp->constantValue = exp->left->constantValue && exp->right->constantValue ? 1 : 0;
       break;
      case OR:
       exp->constantValue = exp->left->constantValue || exp->right->constantValue ? 1 : 0;
       break;
      }
    }
  return Value::makeInt(0);
}





Value Op1Visitor::visit(NumberExpDecimal* exp){
    exp->isConstant=true;
    exp->constantValue= exp->value;
    return Value::makeInt(exp->constantValue);
}
Value Op1Visitor::visit(NumberExpFlotante* exp){
    exp->isConstant=true;
    exp->constantValue= exp->value;
    return Value::makeFloat(exp->constantValue);
}
Value Op1Visitor::visit(StringExp* exp){
    exp->isConstant=true;
    return Value::makeString(exp->valor);
}
Value Op1Visitor::visit(CharExp* exp){
    exp->isConstant=true;
    return Value::makeChar(exp->valor);
}

Value Op1Visitor::visit(IdExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(BoolExp* exp){
    exp->isConstant=true;
    exp->constantValue= exp->booleano == "true" ? 1 : 0;
    return Value::makeInt(exp->constantValue);
}

Value Op1Visitor::visit(FcallExp* exp){
     for (auto arg : exp->argumentos)
        arg->accept(this);
    exp->isConstant = false;

    return Value::makeInt(0);
}
Value Op1Visitor::visit(UnaryExp* exp){
    exp->exp->accept(this);
    exp->isConstant = exp->exp->isConstant;
    if (exp->isConstant){
        switch (exp->op) {
        case UnaryExp::NEGATE:
            exp->constantValue = -exp->exp->constantValue;
            break;
        case UnaryExp::NOT_OP:
            exp->constantValue = !exp->exp->constantValue;
            break;
        default:
            break;
    }
}

return Value::makeInt(0);
}


Value Op1Visitor::visit(NewExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(NullExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(UndefinedExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(ReferenceExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(PunteroExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(AlgoconcorchetesylistaExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(AlgoconcorchetesExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(PuntoExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(LambdaExp* exp){
    exp->isConstant = false;
    return Value::makeInt(0);
}

Value Op1Visitor::visit(NotExp* exp){
    exp->isConstant = false;  
    return Value::makeInt(0);
}

void Op1Visitor::visit(IfStmt* stm){
    stm->condicion->accept(this);
}
void Op1Visitor::visit(WhileStmt* stm){
    stm->condicion->accept(this);
}
void Op1Visitor::visit(BodyStmt* stm){
}
void Op1Visitor::visit(AsignStmt* stm){
    stm->exp->accept(this);
}
void Op1Visitor::visit(PrintStmt* stm){
    stm->exp->accept(this);
}

void Op1Visitor::visit(ReturnStm* stm){
    if( stm->exp != nullptr) {
        stm->exp->accept(this);
    }
}

void Op1Visitor::visit(DeleteStm* stm){
    stm->exp->accept(this);
}

void Op1Visitor::visit(ContinueStm* stm){

}
void Op1Visitor::visit(BreakStmt* stm){
    if(stm->valor != nullptr) {
        stm->valor->accept(this);
    }
}

void Op1Visitor::visit(SwitchStmt* stm){
    stm->condicion->accept(this);
}
void Op1Visitor::visit(TryStmt* stm){
    if( stm->expr != nullptr) {
        stm->expr->accept(this);
    }
}
void Op1Visitor::visit(DeferStmt* stm){
    stm->stmt->accept(this);
}
void Op1Visitor::visit(ForStmt* stm){
    if( stm->condicion != nullptr) {
        stm->condicion->accept(this);
    }
    if( stm->incremento != nullptr) {
        stm->incremento->accept(this);
    }
}
void Op1Visitor::visit(DerefAssignStmt* e){
    e->lval->accept(this);
    e->rval->accept(this);
}


void Op1Visitor::visit(Fundec* fd){

    if(fd->cuerpo != nullptr){
       fd->cuerpo->accept(this);
    }
}
void Op1Visitor::visit(Structdec* sd){

}
void Op1Visitor::visit(VarDec* vd){
    if(vd->exp != nullptr){
        vd->exp->accept(this);
    }
}
void Op1Visitor::visit(ConstDec* cd){
    if(cd->exp != nullptr){
        cd->exp->accept(this);
    }
}
void Op1Visitor::visit(Template* t){

}

void Op1Visitor::visit(IdType* tipo){}
void Op1Visitor::visit(PointerType* tipo){}
void Op1Visitor::visit(ArrayType* tipo){}
void Op1Visitor::visit(OptionalType* tipo){}
void Op1Visitor::visit(ErrorType* tipo){}
void Op1Visitor::visit(UnionType* tipo){}
void Op1Visitor::visit(EnumType* tipo){}


void Op1Visitor::visit(Body* b){
    for(auto s : b->slist) {
        s->accept(this);
    }
}

void Op1Visitor::visit(Programa* p){
    for(auto d : p->declist) {
        d->accept(this);
    }
}


// Sethi-Ullman

Value Op2Visitor::Opt2(Programa *program){
    program->accept(this);
    return Value::makeInt(0);
}

Value Op2Visitor::visit(BinaryExp* exp){
    exp->left->accept(this);
    exp->right->accept(this);

    // Optimización: hoja constante como operando derecho puede
    // codificarse como inmediato -> no consume registro.
    // Como operando izquierdo sí necesita cargarse antes.
    if (exp->left->ishoja && exp->left->isConstant)
        exp->left->label = 1;
    if (exp->right->ishoja && exp->right->isConstant)
        exp->right->label = 0;

    if (exp->left->label == exp->right->label)
        exp->label = exp->left->label + 1;
    else
        exp->label = std::max(exp->left->label, exp->right->label);

    return Value::makeInt(0);
}

Value Op2Visitor::visit(NumberExpDecimal* exp){
    exp->ishoja = true;
    exp->label = 1;   // FIX: toda hoja necesita al menos 1 registro
    return Value::makeInt(0);
}
Value Op2Visitor::visit(NumberExpFlotante* exp){
    exp->ishoja = true;
    exp->label = 1;   // FIX
    return Value::makeInt(0);
}
Value Op2Visitor::visit(StringExp* exp){
    exp->ishoja = true;
    exp->label = 1;   // FIX
    return Value::makeInt(0);
}
Value Op2Visitor::visit(CharExp* exp){
    exp->ishoja = true;
    exp->label = 1;   // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(IdExp* exp){
    exp->ishoja = false;
    exp->label = 1;
    return Value::makeInt(0);
}

Value Op2Visitor::visit(BoolExp* exp){
    exp->ishoja = true;
    exp->label = 1;   // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(FcallExp* exp){
    int maxArgLabel = 0;
    for (auto arg : exp->argumentos) {
        arg->accept(this);
        maxArgLabel = std::max(maxArgLabel, arg->label);  // FIX: usar el resultado real
    }
    exp->ishoja = false;
    // Una llamada necesita al menos tantos registros como su argumento
    // más exigente, con un mínimo razonable para el propio call.
    exp->label = std::max(maxArgLabel, 1);
    return Value::makeInt(0);
}

Value Op2Visitor::visit(UnaryExp* exp){
    exp->exp->accept(this);
    exp->ishoja = false;
    exp->label = std::max(1, exp->exp->label);  // FIX: piso de 1 registro
    return Value::makeInt(0);
}

Value Op2Visitor::visit(NewExp* exp){
    exp->ishoja = false;
    exp->label = 1;   // FIX: resultado de malloc ocupa un registro
    return Value::makeInt(0);
}

Value Op2Visitor::visit(NullExp* exp){
    exp->ishoja = true;   // es un literal
    exp->label = 1;       // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(UndefinedExp* exp){
    exp->ishoja = true;
    exp->label = 1;   // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(ReferenceExp* exp){
    if (exp->exp) exp->exp->accept(this);   // FIX: faltaba recorrer
    exp->ishoja = false;
    exp->label = 1;   // FIX: leaq siempre cabe en 1 registro
    return Value::makeInt(0);
}

Value Op2Visitor::visit(PunteroExp* exp){
    exp->exp->accept(this);   // FIX: faltaba recorrer
    exp->ishoja = false;
    exp->label = std::max(1, exp->exp->label);   // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(AlgoconcorchetesylistaExp* exp){
    int maxLabel = 0;
    for (auto arg : exp->argumentos) {   // FIX: faltaba recorrer
        arg->accept(this);
        maxLabel = std::max(maxLabel, arg->label);
    }
    exp->ishoja = false;
    exp->label = std::max(1, maxLabel);   // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(AlgoconcorchetesExp* exp){
    exp->nombre->accept(this);      // FIX: faltaba recorrer
    exp->dentroexp->accept(this);   // FIX: faltaba recorrer
    exp->ishoja = false;

    // misma fórmula que BinaryExp: base + índice
    if (exp->nombre->label == exp->dentroexp->label)
        exp->label = exp->nombre->label + 1;
    else
        exp->label = std::max(exp->nombre->label, exp->dentroexp->label);

    return Value::makeInt(0);
}

Value Op2Visitor::visit(PuntoExp* exp){
    exp->exp->accept(this);   // FIX: faltaba recorrer
    exp->ishoja = false;
    exp->label = std::max(1, exp->exp->label);   // FIX
    return Value::makeInt(0);
}

Value Op2Visitor::visit(LambdaExp* exp){
    if (exp->cuerpo) exp->cuerpo->accept(this);   // FIX: faltaba recorrer
    exp->ishoja = false;
    exp->label = 1;   // FIX: leaq a la función siempre 1 registro
    return Value::makeInt(0);
}

Value Op2Visitor::visit(NotExp* exp){
    if (exp->exp) exp->exp->accept(this);   // FIX: faltaba recorrer
    exp->ishoja = false;
    exp->label = std::max(1, exp->exp ? exp->exp->label : 1);   // FIX
    return Value::makeInt(0);
}

void Op2Visitor::visit(IfStmt* stm){
    stm->condicion->accept(this);
}
void Op2Visitor::visit(WhileStmt* stm){
    stm->condicion->accept(this);
}
void Op2Visitor::visit(BodyStmt* stm){
}
void Op2Visitor::visit(AsignStmt* stm){
    stm->exp->accept(this);
}
void Op2Visitor::visit(PrintStmt* stm){
    stm->exp->accept(this);
}

void Op2Visitor::visit(ReturnStm* stm){
    if (stm->exp != nullptr) {
        stm->exp->accept(this);
    }
}

void Op2Visitor::visit(DeleteStm* stm){
    stm->exp->accept(this);
}

void Op2Visitor::visit(ContinueStm* stm){

}
void Op2Visitor::visit(BreakStmt* stm){
    if (stm->valor != nullptr) {
        stm->valor->accept(this);
    }
}

void Op2Visitor::visit(SwitchStmt* stm){
    stm->condicion->accept(this);
}
void Op2Visitor::visit(TryStmt* stm){
    if (stm->expr != nullptr) {
        stm->expr->accept(this);
    }
}
void Op2Visitor::visit(DeferStmt* stm){
    stm->stmt->accept(this);
}
void Op2Visitor::visit(ForStmt* stm){
    if (stm->condicion != nullptr) {
        stm->condicion->accept(this);
    }
    if (stm->incremento != nullptr) {
        stm->incremento->accept(this);
    }
}
void Op2Visitor::visit(DerefAssignStmt* e){
    e->lval->accept(this);
    e->rval->accept(this);
}

void Op2Visitor::visit(Fundec* fd){
    if (fd->cuerpo != nullptr){
        fd->cuerpo->accept(this);
    }
}
void Op2Visitor::visit(Structdec* sd){

}
void Op2Visitor::visit(VarDec* vd){
    if (vd->exp != nullptr){
        vd->exp->accept(this);
    }
}
void Op2Visitor::visit(ConstDec* cd){
    if (cd->exp != nullptr){
        cd->exp->accept(this);
    }
}
void Op2Visitor::visit(Template* t){

}

void Op2Visitor::visit(IdType* tipo){}
void Op2Visitor::visit(PointerType* tipo){}
void Op2Visitor::visit(ArrayType* tipo){}
void Op2Visitor::visit(OptionalType* tipo){}
void Op2Visitor::visit(ErrorType* tipo){}
void Op2Visitor::visit(UnionType* tipo){}
void Op2Visitor::visit(EnumType* tipo){}

void Op2Visitor::visit(Body* b){
    for (auto s : b->slist) {
        s->accept(this);
    }
}

void Op2Visitor::visit(Programa* p){
    for (auto d : p->declist) {
        d->accept(this);
    }
}
