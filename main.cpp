#include <iostream>
#include "RTree.h"

#define ELEMTYPE int
#define NUMDIMS 2

using namespace std;
struct City{
    int* m_min;                      ///< Min dimensions of bounding box
    int* m_max;                      ///< Max dimensions of bounding box
    City(int* min, int* max){
        m_min = new int[NUMDIMS];
        m_max = new int[NUMDIMS];
        m_min = min;
        m_max = max;
    }
};


int main() {

    RTree<int, int, 2, 8, 4> *rtree;
    int min[NUMDIMS] = {1,3};
    int max[NUMDIMS] = {3,3};
    City* city1 = new City(min,max);

    rtree->Insert(city1->m_min,city1->m_max ,1);

    cout << "Hello, World!" << endl;
    return 0;
}