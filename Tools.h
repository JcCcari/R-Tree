//
// Created by jhoelccarignome on 02/04/17.
//

#ifndef RTREE_TOOLS_H
#define RTREE_TOOLS_H

/// Minimal bounding rectangle (n-dimensional)
/* if is a leaf, it contain a point N dimensional
 * else contain the bounding rectangle
 * */
struct Mbr
{
    ELEMTYPE m_min[NUMDIMS];                      ///< Min dimensions of bounding box
    ELEMTYPE m_max[NUMDIMS];                      ///< Max dimensions of bounding box
};

/// May be data or may be another subtree
/// The parents level determines this.
/// If current level is 0, then this is data
struct Entry
{
    Mbr m_mbr;                                  ///< Bounds
    union{
        RNode* m_child;                              ///< Child node
        DATATYPE m_data;                            ///< Data Id or Ptr
    };
};

/// Node for each branch level
struct RNode
{
    //bool IsInternalNode()                         { return (m_level > 0); } // Not a leaf, but a internal node
    bool isLeaf()                                 { return (m_level == 0); } // A leaf, contains data

    int m_count;                                  ///< Number of entries
    int m_level;                                  ///< Leaf is zero, others positive
    Entry m_entry[];                      ///< Array of Entries
};

///

#endif //RTREE_TOOLS_H
