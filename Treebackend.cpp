#include "TreeBackend.hpp"
#include <algorithm>
#include <QDebug>

void TreeBackend::createTree(const QString &exp)
{
    nodes.clear();
    lines.clear();
    idCounter = 0;
    nodeStack.clear();
    opStack.clear();
    root = nullptr;    
    parseExpression(exp);
    putDepth(root, 0);
    putId(root);
    res = evaluate(root);
}

QVariantList TreeBackend::denseNode(int width, int height)
{ // 给出node列表
    // id, x, y, label
    int maxDepth = getMaxDepth(root);
    int singleWidth = width / (idCounter + 1);
    int singleHeight = height / (maxDepth + 1);
    setLocation(root, singleWidth, singleHeight);
    return nodes;
}

QVariantList TreeBackend::denseLine()
{ // 给出line列表
    // from, to
    setFromTo(root);
    return lines;
}

QString TreeBackend::sum() {
    return QString::number(res);
}

void TreeBackend::parseExpression(const QString &exp)
{ // 解析数据并塞入两个栈
    for (int i = 0; i < exp.length(); i++)
    {
        QChar ch = exp[i];
        if (ch.isSpace())
            continue; // 跳过空格
        if (ch.isDigit())
        { // 这里先玩简单的，十以内的数字
            treeNode *node = new treeNode();
            node->type = NUM;
            node->v.num = ch.digitValue();
            nodeStack.push_back(node);
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
        {
            // 当栈顶运算符的优先级高于或等于当前运算符时，需要先进行归约，保证左结合
            while (!opStack.isEmpty() && precedence(opStack.back()) >= precedence(ch))
            {
                reduceOnce();
            }
            opStack.push_back(ch);
        }
    }
    while (!opStack.isEmpty())
    {
        reduceOnce();
    }
    root = nodeStack.back();
    nodeStack.pop_back();
}

void TreeBackend::reduceOnce()
{
    if (opStack.isEmpty() || nodeStack.size() < 2) return;
    QChar op = opStack.back();
    opStack.pop_back();
    treeNode *right = nodeStack.back();
    nodeStack.pop_back();
    treeNode *left = nodeStack.back();
    nodeStack.pop_back();
    
    treeNode *parent = new treeNode();
    parent->type = OP;
    parent->v.op = op.toLatin1(); //此处是QChar转char，我不知道原理
    parent->left = left;
    parent->right = right;

    nodeStack.push_back(parent);
}

int TreeBackend::precedence(QChar op)
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    return 0;
}

double TreeBackend::evaluate(treeNode *node)
{
    if (node->type == NUM)
    {
        return node->v.num;
    }
    else
    {
        double left = evaluate(node->left);
        double right = evaluate(node->right);
        switch (node->v.op)
        {
        case '+':
            return left + right;
        case '-':
            return left - right;
        case '*':
            return left * right;
        case '/':
            return left / right;
        default:
            return 0.0; // TODO 如果出现 x/0的情况呢，抛出异常？
        }
    }
}

void TreeBackend::putDepth(treeNode *node, int depth)
{ // 赋予深度
    if (node == nullptr)
        return;
    node->depth = depth;
    putDepth(node->left, depth + 1);
    putDepth(node->right, depth + 1);
}

void TreeBackend::putId(treeNode *node)
{
    if (node == nullptr)
        return;
    putId(node->left);
    node->id = idCounter++;
    putId(node->right);
}

void TreeBackend::setLocation(treeNode *root, int singleWight, int singleHeight)
{
    if (root == nullptr)
        return;
    int x = (root->id + 1) * singleWight;
    int y = (root->depth + 1) * singleHeight;
    nodes.append(QVariantMap{{"id", root->id}, {"x", x}, {"y", y}, {"label", root->type == NUM ? QString::number(root->v.num) : QString(root->v.op)}});
    setLocation(root->left, singleWight, singleHeight);
    setLocation(root->right, singleWight, singleHeight);
}

void TreeBackend::setFromTo(treeNode *root)
{
    treeNode *left = root->left;
    treeNode *right = root->right;
    if (left)
    {
        int from = root->id;
        int to = left->id;
        lines.append(QVariantMap{{"from", from}, {"to", to}});
        setFromTo(left);
    }
    if (right)
    {
        int from = root->id;
        int to = right->id;
        lines.append(QVariantMap{{"from", from}, {"to", to}});
        setFromTo(right);
    }
}

int TreeBackend::getMaxDepth(treeNode *root)
{
    if (root == nullptr)
        return 0;
    return 1 + std::max(getMaxDepth(root->left), getMaxDepth(root->right));
}
