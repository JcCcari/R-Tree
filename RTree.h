//
// Created by jhoelccarignome on 01/04/17.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H
#include <vector>
using namespace std;
//#include "Tools.h"

/// DATATYPE Referenced data, should be int, void*, obj* etc. no larger than sizeof<void*> and simple type
/// ELEMTYPE Type of element such as int or float
/// NUMDIMS Number of dimensions such as 2 or 3
#define RTREE_TEMPLATE template<class DATATYPE, class ELEMTYPE, int NUMDIMS, int TMAXNODES, int TMINNODES>
#define RTREE_QUAL RTree<DATATYPE, ELEMTYPE, NUMDIMS, TMAXNODES, TMINNODES>

template<class DATATYPE, class ELEMTYPE, int NUMDIMS, int TMAXNODES= 8, int TMINNODES=TMAXNODES/2>
class RTree {
public:
    class RNode;
    struct NMbr;
    struct NEntry;
    static int const MINNODES = TMINNODES;
    static int const MAXNODES = TMAXNODES;
//public:
    RTree()     { this->root = nullptr; this->high= 0;}
    ~RTree(){}
    //bool insert(NMbr m_mbr, DATATYPE m_data);
    bool Insert(const ELEMTYPE a_min[NUMDIMS],const ELEMTYPE a_max[NUMDIMS], DATATYPE data);
private:
    /// Insert Methods
    RNode* chooseLeaf(NMbr mbr){
        RNode* node = this->root;
        int index = 0;  /// posicion en RNode en que que vamos a insertar
        float auxArea, minArea, tmpArea, tmpEnlargement;
        while( not node->isLeaf() ){ /// mientras sea un nodo no hoja, buscamos en su entradas
            /// empezamos a buscar en cada Entry del RNode que estamos visitando
            /// suponemos que el primer NEntry tiene el area minima
            auxArea = node->m_entry[0].m_mbr->calArea();
            minArea =  node->m_entry[0].m_mbr->calEnlargement(mbr) - auxArea; /// hallamos el enlargamiento( diferencia de areas)
            for(int i=1; i< node->m_count; ++i){
                tmpArea = node->m_entry[i].m_mbr->calArea();
                tmpEnlargement=  node->m_entry[i].m_mbr->calEnlargement(mbr) - tmpArea;
                if(tmpEnlargement < minArea)        /// Comparamos los enlargamientos, si el nuevo es menor que el anterior, cambiamos la posicion de index
                    index = i;
                else if( tmpEnlargement == minArea){/// Si tienen el mismo area de enlargamiento
                    if( tmpArea < auxArea )         /// escogemos el que tenga menor area en su MBR
                        index = i;
                }
            }
            node = node->m_entry[index].m_child;
        };
        return node; /// si el nodo es una hoja, la devolvemos
    }

    void splitNode(RNode& L, RNode& LL, NEntry* newEntry);
    void pickSeeds(int* seed1, int* seed2, vector<NEntry>& entries);
    void pickNext(int* next, NMbr* L, NMbr* LL, vector<NEntry> entries);
    bool adjustTree(RNode* L, RNode* LL);
    int findEntry(RNode* N, RNode* parent);

public:
    /// Minimal bounding rectangle (n-dimensional)
/* if is a leaf, it contain a point N dimensional
 * else contain the bounding rectangle
 * */
    struct NMbr{
        ELEMTYPE m_min[NUMDIMS];                      ///< Min dimensions of bounding box
        ELEMTYPE m_max[NUMDIMS];                      ///< Max dimensions of bounding box
        NMbr(ELEMTYPE min[NUMDIMS],ELEMTYPE max[NUMDIMS]){
            //m_min = new int[NUMDIMS]; m_max = new int[NUMDIMS];
            m_min = min;
            m_max = max;
        }
        float calArea();
        float calEnlargement(NMbr mbr);
        void updateMBR(NMbr* mbr);
    };

/// May be data or may be another subtree
/// The parents level determines this.
/// If current level is 0, then this is data
    struct NEntry
    {
        NMbr* m_mbr;                                  ///< Bounds

        union{
            RNode* m_child;                              ///< Child node
            DATATYPE m_data;                            ///< Data Id or Ptr
        };

        NEntry(DATATYPE data, NMbr* mbr= nullptr){
            this->m_mbr = mbr;
            this->m_data = data;
        }
        NEntry(RNode* child, NMbr* mbr = nullptr){
            this->m_mbr = mbr;
            this->m_child = child;
        }
        NEntry(){
            this->m_mbr = nullptr;
        }
        void updateMBR(RNode* node);
    };

/// Node for each branch level
    struct RNode
    {
        //bool IsInternalNode()                         { return (m_level > 0); } // Not a leaf, but a internal node
        bool isLeaf()                                 { return (m_level == 0); } // A leaf, contains data

        RNode* m_parent;                              ///< Parent Pointer
        int m_count;                                  ///< Number of entries
        int m_level;                                  ///< Leaf is zero, others positive
        NEntry m_entry[MAXNODES];                     ///< Array of Entries
        RNode(){
            m_count=0;
            m_level=0;
            m_parent = nullptr;
        }
        void clear(){
            m_entry = nullptr;
            m_count = m_level = 0;
        }

    };

    RNode* root;
    int high;
};

RTREE_TEMPLATE
bool RTREE_QUAL::Insert(const ELEMTYPE a_min[], const ELEMTYPE a_max[], DATATYPE data) {
    ELEMTYPE* min= new ELEMTYPE[NUMDIMS];
    min = const_cast<int*>(a_min);
    ELEMTYPE* max= new ELEMTYPE[NUMDIMS];
    max = const_cast<int*>(a_max);

    NMbr* mbr = new NMbr(min,max);
    RNode* choosedLeaf = chooseLeaf(*mbr);
    NEntry* entry = new NEntry(data, mbr);
    if ( choosedLeaf->m_count < MAXNODES){     /// aun tenemos espacio para insertar m_data
        choosedLeaf->m_entry[choosedLeaf->m_count+1] = *entry;
        choosedLeaf->m_count += 1;
        choosedLeaf->m_level += 1;
        // call adjustTree
        return true;
    }else{                  /// como no hay espacio, entonces partimos el nodo L
        RNode* L = new RNode();
        RNode* LL = new RNode();

        splitNode(*L,*LL,entry);                ///< Invocamos al metodo splitNode() para distribuir las entradas en nodos L y LL

        if( adjustTree(L, LL) ){
            this->root = new RNode();
            NEntry* entry1 = new NEntry(L);
            entry1->updateMBR(L);
            NEntry* entry2 = new NEntry(LL);
            entry2->updateMBR(LL);
            root->m_entry[root->m_count] = *entry1;
            root->m_count += 1;
            root->m_entry[root->m_count] = *entry2;
            root->m_count += 1;

            this->high += 1;
        }
    }
    return false;
    return false;
}

///<<< INSERT_METHODS
/*
RTREE_TEMPLATE
RNode* RTREE_QUAL::chooseLeaf(RTree::NMbr mbr){
    RNode* node = this->root;
    int index = 0;  /// posicion en RNode en que que vamos a insertar
    float auxArea, minArea, tmpArea, tmpEnlargement;
    while( not node->isLeaf() ){ /// mientras sea un nodo no hoja, buscamos en su entradas
        /// empezamos a buscar en cada Entry del RNode que estamos visitando
        /// suponemos que el primer NEntry tiene el area minima
        auxArea = node->m_entry[0].m_mbr.calArea();
        minArea =  node->m_entry[0].m_mbr.calEnlargement(mbr) - auxArea; /// hallamos el enlargamiento( diferencia de areas)
        for(int i=1; i< node->m_count; ++i){
            tmpArea = node->m_entry[i].m_mbr.calArea();
            tmpEnlargement=  node->m_entry[i].m_mbr.calEnlargement(mbr) - tmpArea;
            if(tmpEnlargement < minArea)        /// Comparamos los enlargamientos, si el nuevo es menor que el anterior, cambiamos la posicion de index
                index = i;
            else if( tmpEnlargement == minArea){/// Si tienen el mismo area de enlargamiento
                if( tmpArea < auxArea )         /// escogemos el que tenga menor area en su MBR
                    index = i;
            }
        }
        node = node->m_entry[index].m_child;
    };
    return node; /// si el nodo es una hoja, la devolvemos
}
*/

RTREE_TEMPLATE
void RTREE_QUAL::splitNode(RNode& L, RNode& LL, NEntry *newEntry) {
    vector<NEntry> entries;
    for(int i=0; i< L.m_count; ++i)
        entries.push_back(L.m_entry[i]);
    entries.push_back(*newEntry);

    int* seed1; int* seed2;                     ///< posiciones de las semillas
    L.clear();                                  /// Borramos toda la informacion de L(Nodo lleno) para volver a insertar
    pickSeeds(seed1, seed2, entries);           ///< Invocamos a pickSeed que escogera los dos NEntry mas alejados

    L.m_entry[0] = entries[*seed1];             ///< Insertamos cada semilla en los nodos L y LL
    LL.m_entry[0] = entries[*seed2];

    entries.erase( entries.begin()+ *seed2  );  /// Borramos seed2 primero porque seed2 > seed1
    entries.erase( entries.begin()+ *seed1 );

    while( !entries.empty() ){
        if(L.m_count + entries.size() == MAXNODES){
            for(int i=0; i< entries.size(); ++i)
                L.m_entry[L.m_count+i] = entries[i];
            entries.clear();        ///Nos aseguramos que todas las entradas hayan sido asignadas
        }else if(LL.m_count + entries.size() == MAXNODES){
            for(int i=0; i< entries.size(); ++i)
                LL.m_entry[LL.m_count+i] = entries[i];
            entries.clear();        ///Nos aseguramos que todas las entradas hayan sido asignadas
        }else{                        ///< Invocamos al pickNext
            int pick;
            pickNext(&pick, L.m_entry->m_mbr, LL.m_entry->m_mbr, entries);
            float areaL = L.m_entry[0].m_mbr->calArea();
            float areaLL = LL.m_entry[0].m_mbr->calArea();
            float enlargementL = L.m_entry[0].m_mbr->calEnlargement(entries[pick]) - areaL;
            float enlargementLL = LL.m_entry[0].m_mbr->calEnlargement(entries[pick]) - areaLL;

            if(enlargementL < enlargementLL){
                L.m_count += 1;
                L.m_entry[L.m_count] = entries[pick];
                L.m_entry[L.m_count].m_mbr->updateMBR(entries[pick].m_mbr);    ///actualizar el MBR de L
            }
            else if(enlargementL > enlargementLL ){ // si enlargementLL < enlargementL
                L.m_count +=1;
                LL.m_entry[L.m_count] = entries[pick];
                LL.m_entry[LL.m_count].m_mbr->updateMBR(entries[pick].m_mbr);    ///actualizar el MBR de LL
            }else{ /// enlargementL == enlargementLL
                ///< Las doa rareas de enlargamiento son iguales, asi que:
                /// 1) asignamos al nodo con MBR menor(area menor)
                if(areaL < areaLL){
                    L.m_count += 1;
                    L.m_entry[L.m_count] = entries[pick];
                    L.m_entry[L.m_count].m_mbr->updateMBR(entries[pick].m_mbr);    ///actualizar el MBR de L
                }else if( areaL > areaLL){
                    LL.m_count += 1;
                    LL.m_entry[L.m_count] = entries[pick];
                    LL.m_entry[LL.m_count].m_mbr->updateMBR(entries[pick].m_mbr);    ///actualizar el MBR de LL
                }
                ///< si aun tenemos problemas, asignamos al nodo que tenga menos elementos
                else{
                    if(L.m_count <= LL.m_count){
                        L.m_count += 1;
                        L.m_entry[L.m_count] = entries[pick];
                        L.m_entry[L.m_count].m_mbr->updateMBR(entries[pick].m_mbr);    ///actualizar el MBR de L
                    }else { //if(L.m_count > LL.m_count)
                        LL.m_count += 1;
                        LL.m_entry[L.m_count] = entries[pick];
                        LL.m_entry[LL.m_count].m_mbr->updateMBR(entries[pick].m_mbr);    ///actualizar el MBR de LL
                    }
                }
            }
            ///eliminamos entries[pick] de entries, poruqe ya ha sido asignado al nodo L o LL
            entries.erase(entries.begin()+pick);
        }
    }
}

RTREE_TEMPLATE
void RTREE_QUAL::pickSeeds(int* seed1, int* seed2, vector<NEntry> &entries) {
    float maxEnlargement, tmpEnlargement;
    maxEnlargement = entries[0].m_mbr.calEnlargement(entries[1].m_mbr) - entries[0].m_mbr.calArea();
    for(int i=0; i<entries.size(); ++i){
        for(int j=i+1 ; j<entries.size(); ++j){
            tmpEnlargement = entries[i].m_mbr.calEnlargement(entries[j].m_mbr) - entries[i].m_mbr.calArea();
            if ( tmpEnlargement > maxEnlargement){
                maxEnlargement = tmpEnlargement;
                *seed1 = i;
                *seed2 = j;
            }
        }
    }
}

RTREE_TEMPLATE
void RTREE_QUAL::pickNext(int *next, NMbr *L, NMbr *LL, vector<NEntry> entries) {
    float d1 = L->calEnlargement(entries[0].m_mbr) - L->calArea();
    float d2 = LL->calEnlargement(entries[0].m_mbr) - LL->calArea();
    float increase = d2 <d1 ? d2-d1: d1-d2;

    for(int i=1; i<entries.size(); ++i){
        d1 = L->calEnlargement(entries[i].m_mbr) - L->calArea();
        d2 = LL->calEnlargement(entries[i].m_mbr) - LL->calArea();
        float tmpIncrease = d2 <d1 ? d2-d1: d1-d2;
        if(tmpIncrease > increase){
            increase = tmpIncrease;
            *next = i;
        }
    }
}

RTREE_TEMPLATE
bool RTREE_QUAL::adjustTree(RNode *L, RNode *LL){
    bool rootSplited =false;       /// suponemos que no habra particion de nodo

    RNode* N = L;
    //if( LL != nullptr ) /// preguntamos si es nodo
    RNode* NN = LL; /// si es nodo suponemos que anteriormente hemos llamado a splitNode()
    RNode* PP; /// en caso de que llamemos al metodo splitNode()

    while( N != this->root){
        RNode* P = N->m_parent;  /// Seteamos P como padre del nodo N
        int index = findEntry(N,P);  ///buscamos N en P
        P->m_entry[index].updateMBR(P->m_entry[index].m_child);

        if(NN != nullptr){      /// como existe NN entonces ha habido una particion
            NEntry* entry = new NEntry(NN);
            entry->updateMBR(NN);
            if(P->m_count < MAXNODES){ /// por tanto debemos insertar una entrada en P con su puntero child apuntado a LL
                P->m_entry[P->m_count+1] = *entry;
                P->m_count +=1 ;
                N = P;
                NN = nullptr;
                //rootSplited = false;
            }else{  /// Como no hay espacio llamamos a la funcion splitNode()
                PP = new RNode();
                splitNode(*P,*PP,entry);
                N = P;
                NN = PP;
                if (N == this->root )
                    rootSplited = true;
            }
        }
        else {
            N = P;
            NN = nullptr;
        }
    }
    /// apuntamos L a root y LL al otro nodo que se haya creado
    L = N;
    LL = NN;
    return false;
}

RTREE_TEMPLATE
int RTREE_QUAL::findEntry(RNode *N, RNode* Parent) {
    for(int i=0; i< Parent->m_count; ++i ){
        if( Parent->m_entry[i].m_child == N )
            return i; // retorna el indice en el que se encuentra
    }
    return 0; //si retorna 0 no se encontro entry en el nodo parent
}


/// NODE_METHODS


///<<<< MBR_METHODS
RTREE_TEMPLATE
float RTREE_QUAL::NMbr::calArea(){
    float area = 1;
    for(int i=0; i<NUMDIMS; ++i)
        area *= m_max[i] - m_min[i];
    return area;
}

RTREE_TEMPLATE
float RTREE_QUAL::NMbr::calEnlargement(NMbr mbr) {
    float enlargement;
    int max, min;
    for(int i=0; i< NUMDIMS; ++i){
        max = mbr.m_max[i] - m_max[i];
        min = mbr.m_min[i] - m_min[i];
        enlargement *= max - min;
    }
    return enlargement;
}

RTREE_TEMPLATE
void RTREE_QUAL::NMbr::updateMBR(NMbr* mbr) {
    for(int i=0; i<NUMDIMS; ++i){
        this->m_min[i] = this->m_min[i] < mbr->m_min[i] ? this->m_min[i]: mbr->m_min[i];
        this->m_max[i] = this->m_max[i] < mbr->m_max[i] ? this->m_max[i]: mbr->m_max[i];
    }
}

///<<<< ENTRY_METHODS
RTREE_TEMPLATE
void RTREE_QUAL::NEntry::updateMBR(RNode *node) {
    /// Actualizamos el MBR de la entrada con todos los MBRs de todos los Entries de node
    for(int i=0; i<node->m_count; ++i)
        this->m_mbr->updateMBR(node->m_entry[i].m_mbr);
}
#endif //RTREE_RTREE_H
