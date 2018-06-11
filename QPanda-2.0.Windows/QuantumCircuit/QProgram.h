﻿/*
Copyright (c) 2017-2018 Origin Quantum Computing. All Right Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _QPROGRAM_H_
#define _QPROGRAM_H_

#include <complex>
#include <initializer_list>
#include <vector>
#include <iterator>
#include <map>

#include "QuantumMachine/OriginClassicalExpression.h"
#include "QNode.h"
#include "QGate.h"
#include "ReadWriteLock.h"
#include "QuantumMeasure.h"
#include "ControlFlow.h"

typedef complex<double> QComplex;
using namespace std;
class QuantumDriver;

/*
*  Quantum gates node:QSingleGateNode and QDoubleGateNode
*
*
*/
class QNodeVector;
extern  QNodeVector _G_QNodeVector;
class NodeIter;

class QGateNode
{
public:
    virtual size_t getQuBitVector(vector<Qubit *> &) const = 0;
    virtual size_t getQuBitNum() const = 0;
    virtual QuantumGate * getQGate() const = 0;
    virtual bool isDagger() const = 0;
    virtual size_t getControlVector(vector<Qubit *> &) const = 0;
    virtual bool setDagger(bool) = 0;
    virtual bool setControl(vector<Qubit *> &) = 0;
    virtual ~QGateNode() {}
};

/*
*  Quantum single gate node: RX,RY,RZ,H,S,      CAN ADD OTHER GATES
*  gate:  gate type
*  opQuBit: qubit number
*
*/
class QGateNodeFactory;

class QGate : public QNode, public QGateNode
{
private:
    vector<Qubit *> m_QuBitVector;
    QuantumGate *m_pQGate;
    NodeType m_iNodeType;
    GateType m_iGateType;
    bool m_bIsDagger;
    vector<Qubit *> m_controlQuBitVector;
    QGate();
    QGate(QGate&);

public:
    int iPosition;

    QGate(Qubit*, QuantumGate *);
    QGate(Qubit*, Qubit *, QuantumGate *);
    NodeType getNodeType() const;
    size_t getQuBitVector(vector<Qubit *> &) const;
    size_t getQuBitNum() const;
    QuantumGate * getQGate() const;
    GateType getQGateType() const;
    int getPosition() const;
    bool setDagger(bool);
    bool setControl(vector < Qubit *> &);
    bool isDagger() const;
    size_t getControlVector(vector<Qubit *> &) const;

};

class AbstractQuantumProgram
{
public:
    virtual NodeIter  getFirstNodeIter() = 0;
    virtual NodeIter  getLastNodeIter() = 0;
    virtual NodeIter  getEndNodeIter() = 0;
    virtual NodeIter getHeadNodeIter() = 0;
    virtual void pushBackNode(QNode *) = 0;
    virtual ~AbstractQuantumProgram() {};
    virtual void clear() = 0;
};

class QuantumCircuit;
class AbstractQuantumCircuit
{
public:
    virtual NodeIter  getFirstNodeIter() = 0;
    virtual NodeIter  getLastNodeIter() = 0;
    virtual NodeIter  getEndNodeIter() = 0;
    virtual NodeIter getHeadNodeIter() = 0;
    virtual void pushBackNode(QNode *) = 0;
    virtual ~AbstractQuantumCircuit() {};
    virtual bool isDagger() const = 0;
    virtual bool getControlVector(vector<Qubit *> &) = 0;
    virtual void  subDagger() {};
    virtual void  subControl(vector<Qubit *> &) {};
};

class  Item
{
public:
    virtual Item * getNext()const = 0;
    virtual Item * getPre() const = 0;
    virtual QNode * getNode() const = 0;
    virtual void setNext(Item *) = 0;
    virtual void setPre(Item *) = 0;
    virtual void setNode(QNode *) = 0;
    virtual ~Item() {};
};

class  OriginItem : public Item
{
private:
    Item * m_pNext;
    Item * m_pPre;
    int    m_iNodeNum;
public:

    Item * getNext()const;
    Item * getPre()const;
    QNode * getNode() const;
    void setNext(Item * pItem);
    void setPre(Item * pItem);
    void setNode(QNode * pNode);
};

class QuantumCircuit : public QNode, public AbstractQuantumCircuit
{
private:
    AbstractQuantumCircuit * m_pQuantumCircuit;
    int m_iPosition;
public:
    QuantumCircuit();
    QuantumCircuit(const QuantumCircuit &);
    ~QuantumCircuit();
    void pushBackNode(QNode *);
    QuantumCircuit & operator << ( QGate &);
    QuantumCircuit & operator << ( QuantumMeasure &);
    QuantumCircuit & dagger();
    QuantumCircuit & control(vector<Qubit *> &);
    NodeType getNodeType() const;
    bool isDagger() const;
    bool getControlVector(vector<Qubit *> &);
    NodeIter  getFirstNodeIter();
    NodeIter  getLastNodeIter();
    NodeIter  getEndNodeIter();
    NodeIter getHeadNodeIter();
    int getPosition() const;
};

typedef AbstractQuantumCircuit * (*CreateQCircuit)();
class QuantumCircuitFactory
{
public:

    void registClass(string name, CreateQCircuit method);
    AbstractQuantumCircuit * getQuantumCircuit(std::string &);

    static QuantumCircuitFactory & getInstance()
    {
        static QuantumCircuitFactory  s_Instance;
        return s_Instance;
    }
private:
    map<string, CreateQCircuit> m_QCirciutMap;
    QuantumCircuitFactory() {};

};

class QuantumCircuitRegisterAction {
public:
    QuantumCircuitRegisterAction(string className, CreateQCircuit ptrCreateFn) {
        QuantumCircuitFactory::getInstance().registClass(className, ptrCreateFn);
    }

};

#define REGISTER_QCIRCUIT(className)                                             \
    AbstractQuantumCircuit* QWhileCreator##className(){      \
        return new className();                    \
    }                                                                   \
    QuantumCircuitRegisterAction g_qWhileCreatorDoubleRegister##className(                        \
        #className,(CreateQCircuit)QWhileCreator##className)


class OriginCircuit : public QNode, public AbstractQuantumCircuit
{
private:
    Item * m_pHead;
    Item * m_pEnd;
    SharedMutex m_sm;
    NodeType m_iNodeType;
    bool m_bIsDagger;
    vector<Qubit *> m_controlQuBitVector;
    OriginCircuit(QuantumCircuit &);

public:
    OriginCircuit() : m_iNodeType(CIRCUIT_NODE), m_pHead(nullptr), m_pEnd(nullptr), m_bIsDagger(false)
    {
        m_controlQuBitVector.resize(0);
    };
    ~OriginCircuit();
    void pushBackNode(QNode *);

    void subDagger();
    void subControl(vector<Qubit *> &);
    NodeType getNodeType() const;
    bool isDagger() const;
    bool getControlVector(vector<Qubit *> &);
    NodeIter  getFirstNodeIter();
    NodeIter  getLastNodeIter();
    NodeIter  getEndNodeIter();
    NodeIter getHeadNodeIter();
    int getPosition() const;
};


/*
*  QProg:  quantum program,can construct quantum circuit,data struct is linked list
*  QListHeadNode:  QProg's head pointer.
*  QListLastNode:  QProg's last pointer.
*  QProg & operator<<(const T &)：
*    if T is QSingleGateNode/QDoubleGateNode/QIfEndNode,
*    deep copy T and insert it into left QProg;
*    if T is QIfProg/QWhileProg/QProg,deepcopy
*    IF/WHILE/QProg circuit and insert it into left QProg;
*/
class QProg : public QNode, public AbstractQuantumProgram
{
private:
    AbstractQuantumProgram * m_pQuantumProgram;
    int m_iPosition;
public:
    QProg();
    QProg(const QProg&);
    ~QProg();
    void pushBackNode(QNode *);

    QProg & operator << ( QIfProg );
    QProg & operator << ( QWhileProg );
    QProg & operator << (QuantumMeasure );
    QProg & operator << ( QProg );
    QProg & operator << ( QGate &);
    QProg & operator << ( QuantumCircuit );
    NodeIter getFirstNodeIter();
    NodeIter getLastNodeIter();
    NodeIter  getEndNodeIter();
    NodeIter getHeadNodeIter();
    NodeType getNodeType() const;
    void clear();
    int getPosition() const;
};

typedef AbstractQuantumProgram * (*CreateQProgram)();
class QuantumProgramFactory
{
public:

    void registClass(string name, CreateQProgram method);
    AbstractQuantumProgram * getQuantumCircuit(std::string &);

    static QuantumProgramFactory & getInstance()
    {
        static QuantumProgramFactory  s_Instance;
        return s_Instance;
    }
private:
    map<string, CreateQProgram> m_QProgMap;
    QuantumProgramFactory() {};

};

class QuantumProgramRegisterAction {
public:
    QuantumProgramRegisterAction(string className, CreateQProgram ptrCreateFn) {
        QuantumProgramFactory::getInstance().registClass(className, ptrCreateFn);
    }

};

#define REGISTER_QPROGRAM(className)                                             \
    AbstractQuantumProgram* QProgCreator##className(){      \
        return new className();                    \
    }                                                                   \
    QuantumProgramRegisterAction g_qProgCreatorDoubleRegister##className(                        \
        #className,(CreateQProgram)QProgCreator##className)




class OriginProgram :public QNode, public AbstractQuantumProgram
{
private:
    Item * m_pHead;
    Item * m_pEnd;
    SharedMutex m_sm;
    NodeType m_iNodeType;
    OriginProgram(OriginProgram&);
public:
    ~OriginProgram();
    OriginProgram();
    void pushBackNode(QNode *);
    NodeIter getFirstNodeIter();
    NodeIter getLastNodeIter();
    NodeIter  getEndNodeIter();
    NodeIter getHeadNodeIter();
    NodeType getNodeType() const;
    void clear();
    int getPosition() const;
};

class NodeIter
{
private:
    Item * m_pCur;
public:
    NodeIter(Item * pItem)
    {
        m_pCur = pItem;
    }

    NodeIter(const NodeIter & oldIter)
    {
        this->m_pCur = oldIter.getPCur();   
    }



    Item * getPCur() const
    {
        return this->m_pCur;
    } 


    NodeIter()
    {
        m_pCur = nullptr;
    }

    Item * getItem() const
    {
        return m_pCur;
    }
    NodeIter & operator ++();
    QNode * operator *();
    NodeIter & operator --();
    bool operator != (NodeIter);
    bool operator  == (NodeIter);
};

extern QProg  CreateEmptyQProg();


extern QuantumCircuit  CreateEmptyCircuit();


class QGateNodeFactory
{
public:
    static QGateNodeFactory * getInstance()
    {
        static QGateNodeFactory s_gateNodeFactory;
        return &s_gateNodeFactory;
    }

    QGate & getGateNode(string & name, Qubit *);
    QGate & getGateNode(string & name, Qubit *, double);
    QGate & getGateNode(string & name, Qubit *, Qubit*);
    QGate & getGateNode(double alpha, double beta, double gamma, double delta, Qubit *);
    QGate & getGateNode(double alpha, double beta, double gamma, double delta, Qubit *, Qubit *);

private:
    QGateNodeFactory()
    {
        m_pGateFact = QGateFactory::getInstance();
    }
    QGateFactory * m_pGateFact;

};


#endif
