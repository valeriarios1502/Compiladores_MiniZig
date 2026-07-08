#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include "environment.h"


class BinaryExp;
class NumberExpDecimal;
class NumberExpFlotante;
class StringExp;
class CharExp;
class IdExp;
class BoolExp;
class NotExp;
class FcallExp;
class UnaryExp;
class NewExp;
class NullExp;
class UndefinedExp;
class ReferenceExp;
class PunteroExp;
class AlgoconcorchetesylistaExp;
class AlgoconcorchetesExp;
class PuntoExp;
class LambdaExp;
class IfStmt;
class WhileStmt;
class BodyStmt;
class AsignStmt;
class PrintStmt;
class ReturnStm;
class DeleteStm;
class ContinueStm;
class BreakStmt;
class SwitchStmt;
class TryStmt;
class DeferStmt;
class ForStmt;
class Fundec;
class Structdec;
class VarDec;
class ConstDec;
class Template;
class IdType;
class PointerType;
class ArrayType;
class OptionalType;
class ErrorType;
class UnionType;
class EnumType;
class Body;
class Programa;
class TypeCheckerVisitor;

void opt(Programa* program);

struct CFValue {
    bool      is_const = false;
    long long int_val  = 0;
    double    dbl_val  = 0.0;
 
    CFValue() = default;
    explicit CFValue(long long v) : is_const(true), int_val(v),    dbl_val((double)v) {}
    explicit CFValue(double    v) : is_const(true), int_val((long long)v), dbl_val(v) {}
};

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual Value visit(BinaryExp* exp)                = 0;
    virtual Value visit(NumberExpDecimal* exp)          = 0;
    virtual Value visit(NumberExpFlotante* exp)         = 0;
    virtual Value visit(StringExp* exp)                 = 0;
    virtual Value visit(CharExp* exp)                   = 0;
    virtual Value visit(IdExp* exp)                     = 0;
    virtual Value visit(BoolExp* exp)                   = 0;
    virtual Value visit(FcallExp* exp)                  = 0;
    virtual Value visit(UnaryExp* exp)                  = 0;
    virtual Value visit(NewExp* exp)                    = 0;
    virtual Value visit(NullExp* exp)                   = 0;
    virtual Value visit(UndefinedExp* exp)              = 0;
    virtual Value visit(ReferenceExp* exp)              = 0;
    virtual Value visit(PunteroExp* exp)                = 0;
    virtual Value visit(AlgoconcorchetesylistaExp* exp) = 0;
    virtual Value visit(AlgoconcorchetesExp* exp)       = 0;
    virtual Value visit(PuntoExp* exp)                  = 0;
    virtual Value visit(LambdaExp* exp)                 = 0;
    virtual Value visit(NotExp* exp)                    = 0;


    virtual void visit(IfStmt* stm)        = 0;
    virtual void visit(WhileStmt* stm)     = 0;
    virtual void visit(BodyStmt* stm)      = 0;
    virtual void visit(AsignStmt* stm)     = 0;
    virtual void visit(PrintStmt* stm)     = 0;
    virtual void visit(ReturnStm* stm)     = 0;
    virtual void visit(DeleteStm* stm)     = 0;
    virtual void visit(ContinueStm* stm)   = 0;
    virtual void visit(BreakStmt* stm)     = 0;
    virtual void visit(SwitchStmt* stm)    = 0;
    virtual void visit(TryStmt* stm)       = 0;
    virtual void visit(DeferStmt* stm)     = 0;
    virtual void visit(ForStmt* stm)       = 0;
    virtual void visit(DerefAssignStmt* e) = 0;


    virtual void visit(Fundec* fd)    = 0;
    virtual void visit(Structdec* sd) = 0;
    virtual void visit(VarDec* vd)    = 0;
    virtual void visit(ConstDec* cd)  = 0;
    virtual void visit(Template* t)   = 0;


    virtual void visit(IdType* tipo)       = 0;
    virtual void visit(PointerType* tipo)  = 0;
    virtual void visit(ArrayType* tipo)    = 0;
    virtual void visit(OptionalType* tipo) = 0;
    virtual void visit(ErrorType* tipo)    = 0;
    virtual void visit(UnionType* tipo)    = 0;
    virtual void visit(EnumType* tipo)     = 0;


    virtual void visit(Body* b)     = 0;
    virtual void visit(Programa* p) = 0;
};

class GenCodeVisitor : public Visitor {

private:
    std::ostream &out; 

public:

    std::unordered_map<std::string, int> funcontador;
    std::unordered_map<std::string, int> structFields;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> structFieldOffsets;
    std::unordered_map<std::string, int> unionSizes;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> unionFieldOffsets;
    std::unordered_map<std::string, int> memoria;
    std::unordered_map<std::string, std::string> variableTypes;
    std::unordered_map<std::string, bool> structAllocated;
    std::unordered_map<std::string, int> matrixColumns;
    std::unordered_map<std::string, std::vector<std::string>> funParamNames;
    std::unordered_map<std::string, std::vector<std::string>> funParamTypes;
    std::unordered_map<std::string, std::string> currentMatrixParamLabels;
    std::unordered_map<std::string, bool> memoriaGlobal;
    int offset = -8;              
    int labelcont = 0;             
    bool entornoFuncion = false;  
    std::string currentBreakLabel; 
    std::string currentContinueLabel;
    std::string nombreFuncion;     
    std::string lastTypeName = "";
    std::unordered_map<std::string, int> posicion;
    std::unordered_map<std::string, bool> globales;
    std::unordered_map<std::string, std::string> globalNames;

    TypeCheckerVisitor* tiposPtr;

    explicit GenCodeVisitor(std::ostream &out = std::cout); 
    ~GenCodeVisitor();

    int getLocalSlot(const string& nombre); 
    int contador = 1;

    void emitirDefers(); 
    string funcionActual = "";
    int callScratchOffset = 0;

    vector<Stmt*> deferStack;
    
    bool dentroDeFuncion = false;

    std::unordered_map<std::string, int> arrayLocals; 
    std::unordered_map<std::string, int> dynamicArraySizes;
    
    void genAddress(Exp* lval);
    int alignStackBytes(int bytes) const;
    int elementSizeBytes(Type* tipo) const;
    void emitArrayElementCount(ArrayType* tipo);
    size_t maxRegisterArgs() const;
    const char* argRegister(size_t index) const;
    void emitCall(const std::string& nombre);
    void emitStringData(const std::string& label, const std::string& value);
    
    void gencode(Programa* program);

    Value visit(BinaryExp* exp)                override;
    Value visit(NumberExpDecimal* exp)          override;
    Value visit(NumberExpFlotante* exp)         override;
    Value visit(StringExp* exp)                 override;
    Value visit(CharExp* exp)                   override;
    Value visit(IdExp* exp)                     override;
    Value visit(BoolExp* exp)                   override;
    Value visit(UnaryExp* exp)                  override;
    Value visit(NewExp* exp)                    override;
    Value visit(NullExp* exp)                   override;
    Value visit(UndefinedExp* exp)              override;
    Value visit(ReferenceExp* exp)              override;
    Value visit(PunteroExp* exp)                override;

    Value visit(PuntoExp* exp)                  override;
    Value visit(AlgoconcorchetesylistaExp* exp) override;
    Value visit(AlgoconcorchetesExp* exp)       override;
    Value visit(FcallExp* exp)                  override;
    Value visit(LambdaExp* exp)                 override;
    Value visit(NotExp* exp) override;


    void visit(IfStmt* stm)        override;
    void visit(WhileStmt* stm)     override;
    void visit(BodyStmt* stm)      override;
    void visit(AsignStmt* stm)     override;
    void visit(PrintStmt* stm)     override;
    void visit(ReturnStm* stm)     override;
    void visit(DeleteStm* stm)     override;
    void visit(ContinueStm* stm)   override;
    void visit(BreakStmt* stm)     override;
    void visit(SwitchStmt* stm)    override;
    void visit(TryStmt* stm)       override;
    void visit(DeferStmt* stm)     override;
    void visit(ForStmt* stm)       override;
    void visit(DerefAssignStmt* stm) override;



    void visit(Fundec* fd)    override;
    void visit(Structdec* sd) override;
    void visit(VarDec* vd)    override;
    void visit(ConstDec* cd)  override;
    void visit(Template* t)   override;

 

    void visit(IdType* tipo)       override;
    void visit(PointerType* tipo)  override;
    void visit(ArrayType* tipo)    override;
    void visit(OptionalType* tipo) override;
    void visit(ErrorType* tipo)    override;
    void visit(UnionType* tipo)    override;
    void visit(EnumType* tipo)     override;


    void visit(Body* b)     override;
    void visit(Programa* p) override;
};


// Constant folding visitor


class Op1Visitor : public Visitor {
public:
    Value Opt1(Programa *program);

    Value visit(BinaryExp* exp);
    Value visit(NumberExpDecimal* exp);
    Value visit(NumberExpFlotante* exp);
    Value visit(StringExp* exp);
    Value visit(CharExp* exp);
    Value visit(IdExp* exp);
    Value visit(BoolExp* exp);
    Value visit(FcallExp* exp);
    Value visit(UnaryExp* exp);
    Value visit(NewExp* exp);
    Value visit(NullExp* exp);
    Value visit(UndefinedExp* exp);
    Value visit(ReferenceExp* exp);
    Value visit(PunteroExp* exp);
    Value visit(AlgoconcorchetesylistaExp* exp);
    Value visit(AlgoconcorchetesExp* exp);
    Value visit(PuntoExp* exp);
    Value visit(LambdaExp* exp);
    Value visit(NotExp* exp);


    void visit(IfStmt* stm);
    void visit(WhileStmt* stm);
    void visit(BodyStmt* stm);
    void visit(AsignStmt* stm);
    void visit(PrintStmt* stm);
    void visit(ReturnStm* stm);
    void visit(DeleteStm* stm);
    void visit(ContinueStm* stm);
    void visit(BreakStmt* stm);
    void visit(SwitchStmt* stm);
    void visit(TryStmt* stm);
    void visit(DeferStmt* stm);
    void visit(ForStmt* stm);
    void visit(DerefAssignStmt* e);


    void visit(Fundec* fd);
    void visit(Structdec* sd);
    void visit(VarDec* vd);
    void visit(ConstDec* cd);
    void visit(Template* t);


    void visit(IdType* tipo);
    void visit(PointerType* tipo);
    void visit(ArrayType* tipo);
    void visit(OptionalType* tipo);
    void visit(ErrorType* tipo);
    void visit(UnionType* tipo);
    void visit(EnumType* tipo);


    virtual void visit(Body* b);
    virtual void visit(Programa* p);
};


// Selthi-Ulman visitor
class Op2Visitor : public Visitor {
public:

    Value Opt2(Programa *program);
    Value visit(BinaryExp* exp);
    Value visit(NumberExpDecimal* exp);
    Value visit(NumberExpFlotante* exp);
    Value visit(StringExp* exp);
    Value visit(CharExp* exp);
    Value visit(IdExp* exp);
    Value visit(BoolExp* exp);
    Value visit(FcallExp* exp);
    Value visit(UnaryExp* exp);
    Value visit(NewExp* exp);
    Value visit(NullExp* exp);
    Value visit(UndefinedExp* exp);
    Value visit(ReferenceExp* exp);
    Value visit(PunteroExp* exp);
    Value visit(AlgoconcorchetesylistaExp* exp);
    Value visit(AlgoconcorchetesExp* exp);
    Value visit(PuntoExp* exp);
    Value visit(LambdaExp* exp);
    Value visit(NotExp* exp);


    void visit(IfStmt* stm);
    void visit(WhileStmt* stm);
    void visit(BodyStmt* stm);
    void visit(AsignStmt* stm);
    void visit(PrintStmt* stm);
    void visit(ReturnStm* stm);
    void visit(DeleteStm* stm);
    void visit(ContinueStm* stm);
    void visit(BreakStmt* stm);
    void visit(SwitchStmt* stm);
    void visit(TryStmt* stm);
    void visit(DeferStmt* stm);
    void visit(ForStmt* stm);
    void visit(DerefAssignStmt* e);


    void visit(Fundec* fd);
    void visit(Structdec* sd);
    void visit(VarDec* vd);
    void visit(ConstDec* cd);
    void visit(Template* t);


    void visit(IdType* tipo);
    void visit(PointerType* tipo);
    void visit(ArrayType* tipo);
    void visit(OptionalType* tipo);
    void visit(ErrorType* tipo);
    void visit(UnionType* tipo);
    void visit(EnumType* tipo);


    virtual void visit(Body* b);
    virtual void visit(Programa* p);
};



#endif // VISITOR_H
