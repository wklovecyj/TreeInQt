#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include <QVariantList>

enum NodeType
{
    NUM,
    OP
};

struct treeNode
{
    NodeType type = NUM;
    union
    {
        double num; // for NUM
        char op;    // for OP (ASCII operator: + - * /)
    } v;
    int id;    // 作为标识和相对位置的x
    int depth; // 作为深度和相对位置的y
    treeNode *left = nullptr;
    treeNode *right = nullptr;

    treeNode() { v.num = 0.0; }
};

// 其实本质思路就是，一个栈式符号栈，还有一个式运算数的节点栈，
// 弹出符号，弹出两个节点，包装符号为一个新节点，然后把新节点压入节点栈

class TreeBackend : public QObject
{
    Q_OBJECT
public:
    explicit TreeBackend(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE void createTree(const QString &exp);
    Q_INVOKABLE QVariantList denseNode(int width, int height);
    Q_INVOKABLE QVariantList denseLine();
    Q_INVOKABLE QString sum();

    signals:
    void errorOccurred(const QString &message);

private:
    treeNode *root = nullptr;
    double res = 0;
    int idCounter = 0; // 所有id从0开始
    QVariantList nodes;
    QVariantList lines;
    QVector<treeNode *> nodeStack = QVector<treeNode *>(); // 节点栈
    QVector<QChar> opStack = QVector<QChar>();             // 符号栈
    void parseExpression(const QString &exp);              // 解析塞入两栈然后建树
    void reduceOnce();                                     // 传入一个新的，弹出一个运算符，连接两个节点成子树，并可选计算数值
    int precedence(QChar op);                              // 运算符优先级
    double evaluate(treeNode *node);                 // 计算
    void putDepth(treeNode *node, int depth);              // 赋予深度
    void putId(treeNode *node);
    void setLocation(treeNode *node, int width, int height);
    void setFromTo(treeNode *root);
    int getMaxDepth(treeNode *root);
};
