#include "NodeComparator.h"

int NodeComparator::operator()(Node node1, Node node2) {
    return node1.getF() > node2.getF();
}
