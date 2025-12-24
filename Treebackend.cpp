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

QString TreeBackend::sum()
{
    return QString::number(res);
}

void TreeBackend::parseExpression(const QString &exp)
{ // 解析数据并塞入两个栈（支持一元 +/-，如 -3、2*-3、-(1+2)）
    double temp = 0.0;

    bool expectOperand = true; // 当前是否期望读取“操作数”（数字/左括号/一元符号）
    int sign = +1;             // 收集一元 +/-，允许连续：--3、+-+3

    for (int i = 0; i < exp.length(); i++) //做大循环，对数字，括号，运算符分别处理
    {
        QChar ch = exp[i];
        if (ch.isSpace())
            continue;

        // 1) 一元 +/-（仅在期望操作数时生效）
        if (expectOperand && (ch == '+' || ch == '-')) //在一开始就判断期待数
        {
            if (ch == '-')
                sign = -sign;
            continue;
        }

        // 2) 数字（含小数）
        if (ch.isDigit() || ch == '.')
        {
            bool dotSeen = false;
            double factor = 0.1;
            temp = 0.0;

            while (i < exp.length() && (exp[i].isDigit() || exp[i] == '.'))
            {
                if (exp[i].isDigit() && !dotSeen)
                {
                    temp = temp * 10 + exp[i].digitValue();
                }
                else if (exp[i].isDigit() && dotSeen)
                {
                    temp = temp + exp[i].digitValue() * factor;
                    factor *= 0.1;
                }
                else if (exp[i] == '.')
                {
                    if (dotSeen)
                    {
                        qWarning() << "Invalid number format";
                        break;
                    }
                    dotSeen = true;
                }
                i++;
            }
            i--; // while 多走了一格，回退

            temp *= sign;
            sign = +1;

            treeNode *node = new treeNode();
            node->type = NUM;
            node->v.num = temp;
            nodeStack.push_back(node);

            expectOperand = false; //数字后面没有期待数
            continue;
        }

        // 3) 左括号
        if (ch == '(')
        {
            if (sign == -1) //！！！！！！，如果-是在()前面出现的，就自己加一个0节点，形成0-()的形式，ai666啊
            {
                treeNode *zero = new treeNode();
                zero->type = NUM;
                zero->v.num = 0.0;
                nodeStack.push_back(zero);

                // 压入一个二元减号，遵循优先级/左结合
                while (!opStack.isEmpty() && opStack.back() != '(' &&
                       precedence(opStack.back()) >= precedence('-'))
                {
                    reduceOnce();
                }
                opStack.push_back('-');
                sign = +1;
            }

            opStack.push_back('(');
            expectOperand = true; //左括号后面有期待数
            continue;
        }

        // 4) 右括号
        if (ch == ')')
        {
            while (!opStack.isEmpty() && opStack.back() != '(')
                reduceOnce();

            if (!opStack.isEmpty() && opStack.back() == '(')
                opStack.pop_back();
            else
                qWarning() << "Mismatched parentheses";

            expectOperand = false; //右括号后面没有
            continue;
        }

        // 5) 二元运算符
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
        {
            if (expectOperand) //实际上这里不会出现+和-的情况，因为在一开始就判断过，这里只有*/遇见期待数的情况
            {
                qWarning() << "Operator appears where operand expected:" << ch; //TODO 处理
                continue;
            }

            while (!opStack.isEmpty() && opStack.back() != '(' &&  //如果碰到左括号就停
                   precedence(opStack.back()) >= precedence(ch))
            {
                reduceOnce();
            }
            opStack.push_back(ch);
            expectOperand = true; //符号后面有期待数
            continue;
        }

        qWarning() << "Unexpected character:" << ch;
    }

    // 扫尾归约
    while (!opStack.isEmpty())
    {
        if (opStack.back() == '(')
        {
            qWarning() << "Mismatched parentheses";
            opStack.pop_back();
            continue;
        }
        reduceOnce();
    }

    if (!nodeStack.isEmpty())
    {
        root = nodeStack.back();
        nodeStack.pop_back();
    }
    else
    {
        root = nullptr;
        qWarning() << "Empty expression?";
    }
}

void TreeBackend::reduceOnce()
{
    if (opStack.isEmpty() || nodeStack.size() < 2)
        return;
    QChar op = opStack.back();
    opStack.pop_back();
    treeNode *right = nodeStack.back();
    nodeStack.pop_back();
    treeNode *left = nodeStack.back();
    nodeStack.pop_back();

    treeNode *parent = new treeNode();
    parent->type = OP;
    parent->v.op = op.toLatin1(); // 此处是QChar转char，我不知道原理
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
    if (root == nullptr)
        return;
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
