#ifndef _INTERVALTREE_H_
#define _INTERVALTREE_H_

#include "log.h"
#include <queue>
#include <cmath>

DEFINE_LOGGER(ITREE_LOG, "itree");

namespace clusterlib {

/* Forward declaration of IntervalTreeNode and << operator */
template<typename R, typename D> 
class IntervalTreeNodeImpl;
template<typename R, typename D>
std::ostream & operator<< (std::ostream &stream, 
                           const IntervalTreeNodeImpl<R, D> &node);

/**
 * The interface for a node in an interval tree.
 *
 * R = the range type 
 * D = the data container
 */
template<typename R, typename D> 
class IntervalTreeNode {
  public:
    /**
     * Nodes can be red or black
     */
    enum Color {
        NONE = 0,
	RED = 1,
	BLACK = 2
    };

    /**
     * Get the color as a string
     * 
     * @param color the color to get a string for
     * @return the stringified color
     */
    static std::string getColorString(Color color);

    /**
     * Get the start of the interval (inclusive)
     *
     * @return the start of the interval range
     */
    virtual R getStartRange() const = 0;

    /** 
     * Get the end of the interval (inclusive)
     *
     * @return the end of the interval range
     */
    virtual R getEndRange() const = 0;

    /** 
     * Get the max end range (the maximum endRange of any child of
     * this node)
     *
     * @return the maximum end itself and its children's range
     */
    virtual R getEndRangeMax() const = 0;

    /**
     * Get the data in this node
     * 
     * @return the data from the constructor
     */
    virtual const D &getData() const = 0;

    /**
     * Get the color of this node
     *
     * @return the color of this node
     */
    virtual Color getColor() const = 0;

    /**
     * Get the parent of this node
     *
     * @return the parent pointer or NULL if head
     */
    virtual const IntervalTreeNode<R, D> *getParent() const = 0;

    /**
     * Get the left child of this node (clients)
     *
     * @return the left child pointer, could be the sentinel node
     */
    virtual const IntervalTreeNode<R, D> *getLeftChild() const = 0;

    /**
     * Get the right child of this node (clients)
     *
     * @return the right child pointer, could be the sentinel node
     */
    virtual const IntervalTreeNode<R, D> *getRightChild() const = 0;

    /**
     * Is the sentinel?
     *
     * @return true if sentinel, otherwise false;
     */
    virtual bool isSentinel() const = 0 ;

    /**
     * Virtual destructor
     */
    virtual ~IntervalTreeNode() {}
};

/**
 * The implementation of a node in an interval tree
 */
template<typename R, typename D> 
class IntervalTreeNodeImpl : public IntervalTreeNode<R, D> {
  public:
    virtual R getStartRange() const;

    virtual R getEndRange() const;

    virtual R getEndRangeMax() const;

    virtual const D &getData() const;

    virtual typename IntervalTreeNode<R, D>::Color getColor() const;

    virtual const IntervalTreeNode<R, D> *getParent() const;

    virtual const IntervalTreeNode<R, D> *getLeftChild() const;

    virtual const IntervalTreeNode<R, D> *getRightChild() const ;

    virtual bool isSentinel() const ;

  public:
    /**
     * Print out useful data from <<.
     */
    friend std::ostream & operator<< <R, D>(
        std::ostream &stream, 
        const IntervalTreeNodeImpl<R, D> &node);

    /**
     * Generic constructor
     *
     * @param isSentinel if true, this node is a sentinel
     */
    IntervalTreeNodeImpl(bool isSentinel = false);

    /** 
     * Constructor 
     * 
     * @param startRange the start of the interval range (inclusive)
     * @param endRange the end of the interval range (inclusive) the 
     *        endMaxRange is also initialized to this value
     * @param data the data stored by this object
     * @param isSentinel if true, this node is a sentinel
     */
    IntervalTreeNodeImpl(R startRange, 
                     R endRange, 
                     D data, 
                     bool isSentinel = false);

    /** 
     * Set the start range.
     *
     * @param startRange the start range (inclusive)
     */
    virtual void setStartRange(R startRange);

    /** 
     * Set the end range.
     *
     * @param endRange the end range (inclusive)
     */
    virtual void setEndRange(R endRange);

    /** 
     * Set the end range max.
     *
     * @param endRangeMax the end range max (inclusive)
     */
    virtual void setEndRangeMax(R endRangeMax);

    /** 
     * Set the color of the node
     *
     * @param color the new color of the node
     */
    virtual void setColor(typename IntervalTreeNode<R, D>::Color color);

    /** 
     * Set the parent of the node
     *
     * @param parentP the new parent
     */
    virtual void setParent(IntervalTreeNodeImpl<R, D> *parentP);

    /** 
     * Set the left child of the node
     *
     * @param leftChildP of the node
     */
    virtual void setLeftChild(IntervalTreeNodeImpl<R, D> *leftChildP);

    /** 
     * Set the right child of the node
     *
     * @param rightChildP of the node
     */
    virtual void setRightChild(IntervalTreeNodeImpl<R, D> *rightChildP);

    /**
     * Set the data in this node
     * 
     * @param data the data to be set
     */
    void setData(D data);    

    /**
     * Get the parent of this node
     *
     * @return the parent pointer or NULL if head
     */
    const IntervalTreeNodeImpl<R, D> *getParentImpl() const;

    /**
     * Get the parent of this node
     *
     * @return the parent pointer or NULL if head
     */
    IntervalTreeNodeImpl<R, D> *getParentImpl();

    /**
     * Get the left child of this node
     *
     * @return the left child pointer, could be the sentinel node
     */
    const IntervalTreeNodeImpl<R, D> *getLeftChildImpl() const;

    /**
     * Get the left child of this node
     *
     * @return the left child pointer, could be the sentinel node
     */
    IntervalTreeNodeImpl<R, D> *getLeftChildImpl();

    /**
     * Get the right child of this node
     *
     * @return the right child pointer, could be the sentinel node
     */
    const IntervalTreeNodeImpl<R, D> *getRightChildImpl() const;

    /**
     * Get the right child of this node
     *
     * @return the right child pointer, could be the sentinel node
     */
    IntervalTreeNodeImpl<R, D> *getRightChildImpl();

  private:
    /** The color of this node (red, black, uninitialized?) */
    typename IntervalTreeNode<R, D>::Color m_color;

    /** The start of the interval (inclusive) */
    R m_startRange;
    
    /** The end of the interval (inclusive) */
    R m_endRange;

    /** The maximum end of the interval (inclusive) */
    R m_endRangeMax;

    /** The parent of this node */
    IntervalTreeNodeImpl<R, D> *m_parentP;

    /** The left child of this node */
    IntervalTreeNodeImpl<R, D> *m_leftChildP;
    
    /** The right child of this node */
    IntervalTreeNodeImpl<R, D> *m_rightChildP;

    /** The data stored by this node */
    D m_data;

    /** Is sentinel? */
    bool m_sentinel;
};

template<typename R, typename D>
std::string
IntervalTreeNode<R, D>::getColorString(
    IntervalTreeNode<R, D>::Color color)
{
    if (color == IntervalTreeNode<R, D>::NONE) {
        return "none";
    }
    else if (color == IntervalTreeNode<R, D>::RED) {
        return "red";
    }
    else if (color == IntervalTreeNode<R, D>::BLACK) {
        return "black";
    }
    else {
        return "unknown";
    }
}

template<typename R, typename D>
std::ostream &
operator<<(std::ostream &stream, const IntervalTreeNodeImpl<R, D> &node)
{
    stream << "[s=" << node.getStartRange()
           << ",e=" << node.getEndRange() << ",m=" << node.getEndRangeMax()
           << ",c=" << node.getColorString(node.getColor()) 
           << ",p=" << node.getParent() << ",(" << &node
           << "),l=" << node.getLeftChild() << ",r=" << node.getRightChild();

    return stream;
}

template<typename R, typename D>
IntervalTreeNodeImpl<R, D>::IntervalTreeNodeImpl(bool isSentinel) 
    : m_color(IntervalTreeNodeImpl<R, D>::NONE),
      m_parentP(this),
      m_leftChildP(this),
      m_rightChildP(this),
      m_sentinel(isSentinel) 
{
    if (isSentinel) {
        m_color = IntervalTreeNode<R, D>::BLACK;
    }
}

template<typename R, typename D>
IntervalTreeNodeImpl<R, D>::IntervalTreeNodeImpl(R startRange, 
                                         R endRange, 
                                         D data, 
                                         bool isSentinel)
    : m_color(IntervalTreeNodeImpl<R, D>::NONE),
      m_startRange(startRange),
      m_endRange(endRange),
      m_endRangeMax(endRange),
      m_parentP(this),
      m_leftChildP(this),
      m_rightChildP(this),
      m_data(data),
      m_sentinel(isSentinel) 
{
    if (isSentinel) {
        m_color = IntervalTreeNode<R, D>::BLACK;
    }
}

template<typename R, typename D>
R 
IntervalTreeNodeImpl<R, D>::getStartRange() const
{
    if (isSentinel()) {
        throw Exception("getStartRange: Called on sentinel!");
    }
    return m_startRange;
}

template<typename R, typename D>
void
IntervalTreeNodeImpl<R, D>::setStartRange(R startRange)
{
    m_startRange = startRange;
}

template<typename R, typename D>
R 
IntervalTreeNodeImpl<R, D>::getEndRange() const
{
    if (isSentinel()) {
        throw Exception("getEndRange: Called on sentinel!");
    }
    return m_endRange;
}

template<typename R, typename D>
void
IntervalTreeNodeImpl<R, D>::setEndRange(R endRange)
{
    m_endRange = endRange;
}

template<typename R, typename D>
R 
IntervalTreeNodeImpl<R, D>::getEndRangeMax() const
{
    return m_endRangeMax;
}

template<typename R, typename D>
void 
IntervalTreeNodeImpl<R, D>::setEndRangeMax(R endRangeMax)
{
    m_endRangeMax = endRangeMax;
}

template<typename R, typename D>
const D &
IntervalTreeNodeImpl<R, D>::getData() const
{
    if (isSentinel()) {
        throw Exception("getData: Called on sentinel!");
    }
    return m_data;
}

template<typename R, typename D>
void
IntervalTreeNodeImpl<R, D>::setData(D data)
{
    m_data = data;
}

/* 
 * 'typename IntervalTreeNode<R,D>::Color' is necessary since the
 * compiler doesn't know if it is a type (i.e. could be a data member)
 */
template<typename R, typename D>
typename IntervalTreeNode<R, D>::Color
IntervalTreeNodeImpl<R, D>::getColor() const
{
    return m_color;
}

template<typename R, typename D>
void 
IntervalTreeNodeImpl<R, D>::setColor(
    typename IntervalTreeNode<R, D>::Color color) 
{
    if ((isSentinel()) && (color == IntervalTreeNode<R, D>::RED)) {
        throw Exception("setColor: Called on sentinel with RED!");
    }
    m_color = color;
}

template<typename R, typename D>
const IntervalTreeNodeImpl<R, D> *
IntervalTreeNodeImpl<R, D>::getParentImpl() const
{
    return m_parentP;
}

template<typename R, typename D>
IntervalTreeNodeImpl<R, D> *
IntervalTreeNodeImpl<R, D>::getParentImpl()
{
    return m_parentP;
}

template<typename R, typename D>
const IntervalTreeNode<R, D> *
IntervalTreeNodeImpl<R, D>::getParent() const
{
    return getParentImpl();
}

template<typename R, typename D>
void
IntervalTreeNodeImpl<R, D>::setParent(IntervalTreeNodeImpl<R, D> *parentP)
{
    m_parentP = parentP;
}

template<typename R, typename D> 
const IntervalTreeNodeImpl<R, D> *
IntervalTreeNodeImpl<R, D>::getLeftChildImpl() const
{
    if (isSentinel()) {
        throw Exception("getLeftChildImpl: Called on sentinel!");
    }
    return m_leftChildP;
}

template<typename R, typename D> 
IntervalTreeNodeImpl<R, D> *
IntervalTreeNodeImpl<R, D>::getLeftChildImpl()
{
    if (isSentinel()) {
        throw Exception("getLeftChildImpl: Called on sentinel!");
    }
    return m_leftChildP;
}

template<typename R, typename D> 
const IntervalTreeNode<R, D> *
IntervalTreeNodeImpl<R, D>::getLeftChild() const
{
    return getLeftChildImpl();
}

template<typename R, typename D>
void
IntervalTreeNodeImpl<R, D>::setLeftChild(
    IntervalTreeNodeImpl<R, D> *leftChildP)
{
    m_leftChildP = leftChildP;
}

template<typename R, typename D> 
const IntervalTreeNodeImpl<R, D> *
IntervalTreeNodeImpl<R, D>::getRightChildImpl() const
{
    if (isSentinel()) {
        throw Exception("getRightChildImpl: Called on sentinel!");
    }
    return m_rightChildP;
}

template<typename R, typename D> 
IntervalTreeNodeImpl<R, D> *
IntervalTreeNodeImpl<R, D>::getRightChildImpl()
{
    if (isSentinel()) {
        throw Exception("getRightChildImpl: Called on sentinel");
    }
    return m_rightChildP;
}

template<typename R, typename D> 
const IntervalTreeNode<R, D> *
IntervalTreeNodeImpl<R, D>::getRightChild() const
{
    return getRightChildImpl();
}

template<typename R, typename D>
void
IntervalTreeNodeImpl<R, D>::setRightChild(
    IntervalTreeNodeImpl<R, D> *rightChildP)
{
    m_rightChildP = rightChildP;
}

template<typename R, typename D>
bool
IntervalTreeNodeImpl<R, D>::isSentinel() const
{
    return m_sentinel;
}

/** 
 * The interval tree class 
 */
template<typename R, typename D> 
class IntervalTree {
  public:
    /** 
     * Constructor
     */
    IntervalTree();

    /** 
     * Find a particular node that matches all the parameters
     *
     * @param startRange the start of the range (inclusive)
     * @param endRange the end of the range (inclusive)
     * @param data the data contained
     * @return a const reference to that node or NULL if not found
     */
    IntervalTreeNode<R, D> *nodeSearch(R startRange,
                                       R endRange,
                                       const D data);

    /** 
     * Find a node that overlaps this inclusive range
     *
     * @param startRange the start of the range (inclusive)
     * @param endRange the end of the range (inclusive)
     * @return a const reference to that node or NULL if not found
     */
    IntervalTreeNode<R, D> *intervalSearch(R startRange,
                                           R endRange);
    
    /** 
     * Remove a node from the tree.  The node is deallocated by the
     * object here.  Since the node is deallocated here, do not reuse
     * the input pointer.  The data from this node can no longer be
     * accessed by this object (should have been extracted prior to
     * this call).
     *
     * @param nodeP the node to be deleted
     */
    void deleteNode(IntervalTreeNode<R, D> * nodeP);

    /** 
     * Insert a node into the tree (allocated by object).  The
     * balanced tree is maintained by the object.  The user is
     * responsible for managing the memory used for data (if any).
     */
    void insertNode(R startRange,
                    R endRange, 
                    D data);

    /**
     * Verify the correctness of the tree A sentinel node is valid to
     * check and will return true.
     *
     * @param headNodeP the head to start checking from (if NULL, then 
     *        start from the head of this tree)
     * @return true if correct, false otherwise
     */
    bool verifyTree(
        IntervalTreeNode<R, D> *headNodeP = NULL) const;

    /**
     * Print out the tree nodes (intervals, colors, pointers) for
     * debugging depth-first search.
     *
     * @param headNodeP a pointer to the node to begin the output with
     *        (NULL means start at the head)
     */
    void printDepthFirstSearch(
        const IntervalTreeNode<R, D> *headNodeP = NULL) const;

    /**
     * Print out the tree nodes (intervals, colors, pointers) for
     * debugging breadth-first-search.
     * 
     * @param headNodeP a pointer to the node to begin the output with
     *        (NULL means start at the head)
     */
    void printBreadthFirstSearch(
        const IntervalTreeNode<R, D> *headNodeP = NULL) const;

    /**
     * Get the head of the tree.
     *
     * @return the head of the tree, NULL if empty
     */
    IntervalTreeNode<R, D> *getTreeHead();

    /**
     * Get the node with the minimum start range.
     * 
     * @param headNodeP the start of the tree to search (NULL is the tree head)
     * @return the node with the minimum start range or NULL if empty
     */
    IntervalTreeNode<R, D> *getTreeMinStartRangeNode(
        IntervalTreeNode<R, D> *headNodeP = NULL);

    /**
     * Get the node with the maximum start range.
     * 
     * @param headNodeP the start of the tree to search (NULL is the tree head)
     * @return the node with the maximum start range or NULL if empty
     */
    IntervalTreeNode<R, D> *getTreeMaxStartRangeNode(
        IntervalTreeNode<R, D> *headNodeP = NULL);

    /** Iterator support for interval trees */
    class iterator : public std::iterator<std::forward_iterator_tag,
                     IntervalTreeNodeImpl<R, D> > {
      public:
        iterator() :
            m_treeP(NULL),
            m_nodeP(NULL) {}

        iterator(IntervalTree<R, D> *treeP, 
                 IntervalTreeNodeImpl<R, D> *nodeP) 
            : m_treeP(treeP),
              m_nodeP(nodeP) {}
        
        bool operator==(const iterator &rhs)
        {
            return (m_nodeP == rhs.m_nodeP);
        }

        bool operator!=(const iterator &rhs)
        {
            return (m_nodeP != rhs.m_nodeP);
        }

        iterator &operator++()
        {
            m_nodeP = m_treeP->getSuccesor(m_nodeP);
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        IntervalTreeNodeImpl<R, D> &operator*()
        {
            return (*m_nodeP);
        }

        IntervalTreeNodeImpl<R, D> *operator->()
        {
            return m_nodeP;
        }

      private:
        IntervalTree<R, D> *m_treeP;
        IntervalTreeNodeImpl<R, D> *m_nodeP;
    };

    /**
     * Return an iterator to the IntervalTreeNodeImpl with the lowest
     * start range.
     */
    iterator begin() 
    {
        return iterator(this, getMinStartRangeNode(m_headNodeP));
    }

    /**
     * Return an iterator to sentinel node.
     */
    iterator end()
    {
        return iterator(this, &m_sentinelNode);
    }

    /**
     * Is the tree empty?
     */
    bool empty() const;

  private:
    /**
     * Update end range max for a node based on its children.
     *
     * @param nodeP the node that will have its end range max updated
     */ 
    void updateEndRangeMax(IntervalTreeNodeImpl<R, D> *nodeP);

    /** 
     * Get the head node pointer.
     * 
     * @return a pointer to the head node
     */
    const IntervalTreeNodeImpl<R, D> *getHeadNode() const;
    
    /** 
     * Get the head node pointer.
     * 
     * @return a pointer to the head node
     */
    IntervalTreeNodeImpl<R, D> *getHeadNode();

    /**
     * Set the head node pointer.
     *
     * @param nodeP the new head node
     */
    void setHeadNode(IntervalTreeNode<R, D> *nodeP);

    /** 
     * Insert a node into the tree, but does not balance the tree, or
     * give the node a color.  This function is used by insertNode().
     *
     * @param startRange the start of the range (inclusive)
     * @param endRange the end of the range (inclusive)
     * @param data the data to be added to the node
     * @return a pointer to the newly inserted node
     */
    IntervalTreeNodeImpl<R, D> *insertNodeIntoTree(R startRange,
                                                   R endRange,
                                                   D data);

    /** 
     * Rotate a tree to the left around the node.
     *
     * @param nodeP the node to pivot left
     */
    void rotateLeft(IntervalTreeNodeImpl<R, D> *nodeP);

    /** 
     * Rotate a tree to the right around the node.
     *
     * @param nodeP the node to pivot right
     */
    void rotateRight(IntervalTreeNodeImpl<R, D> *nodeP);

    /**
     * Fix the endRangeMax values for all ancestors of any node.
     *
     * @param nodeP the node that will have all its ancestor's values
     *        fixed (not itself)
     */
    void endRangeMaxUpdateAncestors(IntervalTreeNodeImpl<R, D> *nodeP);

    /**
     * Fix the red-black properties of the tree after a deletion.
     *
     * @param nodeP the node that is used to head up and fix the tree
     */
    void deleteFixUp(IntervalTreeNodeImpl<R, D> *nodeP);

    /**
     * Get the sentinel node 
     *
     * @return the pointer to the sentinel node
     */
    const IntervalTreeNodeImpl<R, D> *getSentinelNode() const;

    /**
     * Get the sentinel node 
     *
     * @return the pointer to the sentinel node
     */
    IntervalTreeNodeImpl<R, D> *getSentinelNode();

    /**
     * Get the minimum start range node
     *
     * @param nodeP the node to start the search at
     * @return the pointer to the minimum start range node
     */
    IntervalTreeNodeImpl<R, D> *getMinStartRangeNode(
        IntervalTreeNodeImpl<R, D> *nodeP);

    /**
     * Get the maximum start range node
     *
     * @param nodeP the node to start the search at
     * @return the pointer to the maximum start range node
     */
    IntervalTreeNodeImpl<R, D> *getMaxStartRangeNode(
        IntervalTreeNodeImpl<R, D> *nodeP);

    /**
     * Get the succesor of this node
     *
     * @param nodeP the node to find the sucessor of
     * @return the pointer to the succesor of the input node
     */
    IntervalTreeNodeImpl<R, D> *getSuccesor(
        IntervalTreeNodeImpl<R, D> *nodeP);

    /**
     * Check the tree for correct ordering of color, startRange, and
     * endRangeMaxs (debugging function).  A sentinel node is valid to
     * check and will return true.
     *
     * @param headNodeP the head to start checking
     * @param depth input value of the depth of this node
     * @param maxDepth output vale of the maximum depth of this tree
     * @param nodeCount output value of the depth of this node
     * @return true if correct, false otherwise
     */
    bool checkTree(const IntervalTreeNodeImpl<R, D> *headNodeP,
                   int32_t depth,
                   int32_t &maxDepth,
                   int32_t &nodeCount) const;

  private:
    /** Sentinel node that all leaf nodes point to */
    IntervalTreeNodeImpl<R, D> m_sentinelNode;

    /** Head node of the tree */
    IntervalTreeNodeImpl<R, D> *m_headNodeP;
};

template<typename R, typename D> 
IntervalTree<R, D>::IntervalTree()
    : m_sentinelNode(true),
      m_headNodeP(&m_sentinelNode) {}



template<typename R, typename D> 
IntervalTreeNode<R, D> *
IntervalTree<R, D>::nodeSearch(R startRange,
                               R endRange,
                               const D data)
{
    IntervalTreeNodeImpl<R, D> *xP = getHeadNode();

    while (xP != getSentinelNode()) {
        if ((xP->getStartRange() == startRange) &&
            (xP->getEndRange() == endRange) &&
            (xP->getData() == data)) {
            return xP;
        }
        
        if (startRange < xP->getStartRange()) {
            xP = xP->getLeftChildImpl();
        }
        else {
            xP = xP->getRightChildImpl();
        }
    }

    return NULL;
}


template<typename R, typename D> 
IntervalTreeNode<R, D> *
IntervalTree<R, D>::intervalSearch(R startRange,
                                   R endRange)
{
    IntervalTreeNodeImpl<R, D> *xP = getHeadNode();

    while ((xP != getSentinelNode()) && 
           ((startRange > xP->getEndRange()) ||
            (endRange < xP->getStartRange()))) {
        if ((xP->getLeftChildImpl() != getSentinelNode()) &&
            (xP->getLeftChildImpl()->getEndRangeMax() >= startRange)) {
            xP = xP->getLeftChildImpl();
        }
        else {
            xP = xP->getRightChildImpl();
        }
    }

    if (xP != getSentinelNode()) {
        return xP;
    }
    else {
        return NULL;
    }
}

template<typename R, typename D> 
void
IntervalTree<R, D>::deleteNode(IntervalTreeNode<R, D> *nodeP)
{
    IntervalTreeNodeImpl<R, D> *xP = getSentinelNode();
    IntervalTreeNodeImpl<R, D> *yP = getSentinelNode();
    IntervalTreeNodeImpl<R, D> *zP = 
        dynamic_cast<IntervalTreeNodeImpl<R, D> *>(nodeP);

    if ((zP == NULL) || (zP == getSentinelNode())) {
        std::stringstream ss;
        ss << "deleteNode: zP is bad (" << zP << ")";
        throw Exception(ss.str());
    }

    if ((zP->getLeftChildImpl() == getSentinelNode()) ||
        (zP->getRightChildImpl() == getSentinelNode())) {
        yP = zP;
    }
    else {
        yP = getSuccesor(zP);
        assert(yP != getSentinelNode());
    }

    if (yP->getLeftChildImpl() != getSentinelNode()) {
        xP = yP->getLeftChildImpl();
    }
    else {
        xP = yP->getRightChildImpl();
    }

    xP->setParent(yP->getParentImpl());
    if (yP->getParentImpl() == getSentinelNode()) {
        setHeadNode(xP);
    }
    else {
        if (yP == yP->getParentImpl()->getLeftChildImpl()) {
            yP->getParentImpl()->setLeftChild(xP);
        }
        else {
            yP->getParentImpl()->setRightChild(xP);
        }
    }

    if (yP != zP) {
        /*
         * Copy over the start range, end range, end max range (a
         * little special), and data (need to overload the copy
         * constructor if this object is special).
         */
        zP->setStartRange(yP->getStartRange());
        zP->setEndRange(yP->getEndRange());

        /* Preserve the endMaxRange semantics */
        updateEndRangeMax(zP);
        zP->setData(yP->getData());
    }

    /* Preserve the endMaxRange semantics */
    endRangeMaxUpdateAncestors(yP);
    
    if (yP->getColor() == IntervalTreeNode<R, D>::BLACK) {
        deleteFixUp(xP);
    }

    /* 
     * Deallocate the memory associated with the IntervalTreeNode at
     * this point.  If data was stored, it is gone now from this
     * object.
     */
    delete yP;
}

template<typename R, typename D> 
void 
IntervalTree<R, D>::insertNode(R startRange,
                               R endRange,
                               D data)
{
    IntervalTreeNodeImpl<R, D> *yP = NULL;
    IntervalTreeNodeImpl<R, D> *xP = insertNodeIntoTree(startRange, 
                                                        endRange, 
                                                        data);

    xP->setColor(IntervalTreeNode<R, D>::RED);
    /* 
     * Safe to get parent of parent of Xp in this code block since:
     * 1. If xP is head node, won't pass while condition
     * 2. If Xp is a child of the head node, the head node's color is 
     *    black and also won't pass while condition.
     */
    while ((xP != getHeadNode()) && 
           (xP->getParentImpl()->getColor() == IntervalTreeNode<R, D>::RED)) {
        if (xP->getParentImpl() == xP->getParentImpl()->getParentImpl()->getLeftChildImpl()) {
            yP = xP->getParentImpl()->getParentImpl()->getRightChildImpl();
            if (yP->getColor() == IntervalTreeNode<R, D>::RED) {
                xP->getParentImpl()->setColor(IntervalTreeNode<R, D>::BLACK);
                yP->setColor(IntervalTreeNode<R, D>::BLACK);
                xP->getParentImpl()->getParentImpl()->setColor(
                    IntervalTreeNode<R, D>::RED);
                xP = xP->getParentImpl()->getParentImpl();
            }
            else {
                if (xP == xP->getParentImpl()->getRightChildImpl()) {
                    xP = xP->getParentImpl();
                    rotateLeft(xP);
                }
                xP->getParentImpl()->setColor(IntervalTreeNode<R, D>::BLACK);
                xP->getParentImpl()->getParentImpl()->setColor(
                    IntervalTreeNode<R, D>::RED);
                rotateRight(xP->getParentImpl()->getParentImpl());
            }
        }
        else {
            yP = xP->getParentImpl()->getParentImpl()->getLeftChildImpl();
            if (yP->getColor() == IntervalTreeNode<R, D>::RED) {
                xP->getParentImpl()->setColor(IntervalTreeNode<R, D>::BLACK);
                yP->setColor(IntervalTreeNode<R, D>::BLACK);
                xP->getParentImpl()->getParentImpl()->setColor(
                    IntervalTreeNode<R, D>::RED);
                xP = xP->getParentImpl()->getParentImpl();
            }
            else {
                if (xP == xP->getParentImpl()->getLeftChildImpl()) {
                    xP = xP->getParentImpl();
                    rotateRight(xP);
                }
                xP->getParentImpl()->setColor(IntervalTreeNode<R, D>::BLACK);
                xP->getParentImpl()->getParentImpl()->setColor(
                    IntervalTreeNode<R, D>::RED);
                rotateLeft(xP->getParentImpl()->getParentImpl());
            }
        }
    }

    getHeadNode()->setColor(IntervalTreeNode<R, D>::BLACK);
}

template<typename R, typename D> 
bool
IntervalTree<R, D>::verifyTree(
    IntervalTreeNode<R, D> *headNodeP) const
{
    const IntervalTreeNodeImpl<R, D> *checkTreeHeadP = 
        dynamic_cast<IntervalTreeNodeImpl<R, D> *>(headNodeP);
    if (headNodeP == NULL) {
        checkTreeHeadP = getHeadNode();
    }

    int32_t treeMaxDepth = 0;
    int32_t nodeCount = 0;
    if (checkTree(checkTreeHeadP, 1, treeMaxDepth, nodeCount) != true) {
        return false;
    }

    int32_t maxDepth = static_cast<int32_t>(ceil(2.0 * log2(nodeCount + 1)));
    if (maxDepth < treeMaxDepth) {
        LOG_ERROR(ITREE_LOG, 
                  "verifyTree: Tree depth is %d and max depth is %d",
                  treeMaxDepth,
                  maxDepth);
        return false;
    }

    LOG_INFO(ITREE_LOG, 
             "verifyTree: Depth of %d (max depth %d) and node count %d", 
             treeMaxDepth, 
             maxDepth,
             nodeCount);

    return true;
}

template<typename R, typename D>
bool
IntervalTree<R, D>::empty() const
{
    if (getHeadNode () == getSentinelNode()) {
        return true;
    }
    else {
        return false;
    }
}

template<typename R, typename D> 
void 
IntervalTree<R, D>::updateEndRangeMax(IntervalTreeNodeImpl<R, D> *nodeP)
{
    if (nodeP->isSentinel()) {
        return;
    }

    nodeP->setEndRangeMax(nodeP->getEndRange());
    if (nodeP->getLeftChildImpl() != getSentinelNode()) {
        if (nodeP->getLeftChildImpl()->getEndRangeMax() > 
            nodeP->getEndRangeMax()) {
            nodeP->setEndRangeMax(nodeP->getLeftChildImpl()->getEndRangeMax());
        }
    }
    if (nodeP->getRightChildImpl() != getSentinelNode()) {
        if (nodeP->getRightChildImpl()->getEndRangeMax() > 
            nodeP->getEndRangeMax()) {
            nodeP->setEndRangeMax(nodeP->getRightChildImpl()->getEndRangeMax());
        }
    }
}

template<typename R, typename D>
bool
IntervalTree<R, D>::checkTree(const IntervalTreeNodeImpl<R, D> *headNodeP,
                              int32_t depth,
                              int32_t &maxDepth,
                              int32_t &nodeCount) const
{
    /* NULL indicates check the whole tree */
    if (headNodeP == NULL) {
        headNodeP = getHeadNode();
    }

    if (headNodeP == getSentinelNode()) {
        if (headNodeP->getColor() != IntervalTreeNode<R, D>::BLACK) {
            LOG_ERROR(ITREE_LOG, "checkTree: Sentinel is not black!");
            return false;
        }
        return true;
    }    
    nodeCount++;
    if (depth > maxDepth) {
        maxDepth = depth;
    }

    /* Recurse */
    if (checkTree(headNodeP->getLeftChildImpl(), depth + 1, maxDepth, nodeCount) 
        == false) {
        return false;
    }
    if (checkTree(headNodeP->getRightChildImpl(), depth + 1, maxDepth, nodeCount) 
        == false) {
        return false;
    }

    /* Check that the nodes's color is valid, if red, has black children. */
    if ((headNodeP->getColor() != IntervalTreeNode<R, D>::RED) &&
        (headNodeP->getColor() != IntervalTreeNode<R, D>::BLACK)) {
        std::stringstream ss;
        ss << "checkTree: color '"
           << IntervalTreeNode<R, D>::getColorString(headNodeP->getColor())
           << "' is invalid!";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
        
    }
    if (headNodeP->getColor() == IntervalTreeNode<R, D>::RED) {
        if (headNodeP->getColor() == headNodeP->getLeftChildImpl()->getColor()) {
            std::stringstream ss;
            ss << "checkTree: color '" 
               << IntervalTreeNode<R, D>::getColorString(headNodeP->getColor())
               << "' == left child color '"
               << IntervalTreeNode<R, D>::getColorString(
                   headNodeP->getLeftChildImpl()->getColor()) << "'";
            LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
            return false;
        }
        if (headNodeP->getColor() == headNodeP->getRightChildImpl()->getColor()) {
            std::stringstream ss;
            ss << "checkTree: color'" << headNodeP->getColor()
               << "' < right child color '"
               << headNodeP->getRightChildImpl()->getColor() << "'";
            LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
            return false;
        }
    }

    /* Check startRange of self against children */
    if ((headNodeP->getLeftChildImpl() != getSentinelNode()) &&
        (headNodeP->getStartRange() <= 
         headNodeP->getLeftChildImpl()->getStartRange())) {
        std::stringstream ss;
        ss << "checkTree: start range '" << headNodeP->getStartRange()
           << "' <= start range '"
           << headNodeP->getLeftChildImpl()->getStartRange() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    if ((headNodeP->getRightChildImpl() != getSentinelNode()) &&
        (headNodeP->getStartRange() > 
         headNodeP->getRightChildImpl()->getStartRange())) {
        std::stringstream ss;
        ss << "checkTree: start range '" << headNodeP->getStartRange()
           << "' <= start range '"
           << headNodeP->getRightChildImpl()->getStartRange() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    
    /* Done if this is the head of the tree. */
    if (headNodeP->getParentImpl() == getSentinelNode()) {
        return true;
    }

    /* Check startRange of self against parent */
    if ((headNodeP->getStartRange() >= 
         headNodeP->getParentImpl()->getStartRange()) &&
        (headNodeP->getParentImpl()->getLeftChildImpl() == headNodeP)) {
        std::stringstream ss;
        ss << "checkTree: start range '" << headNodeP->getStartRange()
           << "' >= start range '" 
           << headNodeP->getParentImpl()->getStartRange() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    if ((headNodeP->getStartRange() < 
         headNodeP->getParentImpl()->getStartRange()) &&
        (headNodeP->getParentImpl()->getRightChildImpl() == headNodeP)) {
        std::stringstream ss;
        ss << "checkTree: start range '" << headNodeP->getStartRange()
           << "' < start range '" 
           << headNodeP->getParentImpl()->getStartRange() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    
    /* Check endRangeMax of self against self and parents */
    if (headNodeP->getEndRangeMax() < headNodeP->getEndRange()) {
        std::stringstream ss;
        ss << "checkTree: end range max'" << headNodeP->getEndRangeMax()
           << "' < end range '"
           << headNodeP->getEndRange() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    if ((headNodeP->getLeftChildImpl() != getSentinelNode()) &&
        (headNodeP->getEndRangeMax() < 
         headNodeP->getLeftChildImpl()->getEndRangeMax())) {
        std::stringstream ss;
        ss << "checkTree: end range max'" << headNodeP->getEndRangeMax()
           << "' < end range max '"
           << headNodeP->getLeftChildImpl()->getEndRangeMax() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    if ((headNodeP->getRightChildImpl() != getSentinelNode()) &&
        (headNodeP->getEndRangeMax() < 
         headNodeP->getRightChildImpl()->getEndRangeMax())) {
        std::stringstream ss;
        ss << "checkTree: end range max'" << headNodeP->getEndRangeMax()
           << "' < end range max '"
           << headNodeP->getRightChildImpl()->getEndRangeMax() << "'";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    bool foundEndRangeMaxSource = false;
    if (headNodeP->getEndRangeMax() == headNodeP->getEndRange()) {
        foundEndRangeMaxSource = true;
    }
    else if ((headNodeP->getLeftChildImpl() != getSentinelNode()) &&
             (headNodeP->getEndRangeMax() == 
              headNodeP->getLeftChildImpl()->getEndRangeMax())) {
        foundEndRangeMaxSource = true;
    }
    else if ((headNodeP->getRightChildImpl() != getSentinelNode()) &&
             (headNodeP->getEndRangeMax() == 
              headNodeP->getRightChildImpl()->getEndRangeMax())) {
        foundEndRangeMaxSource = true;
    }
    if (foundEndRangeMaxSource == false) {
        std::stringstream ss;
        ss << "checkTree: end range max '" << headNodeP->getEndRangeMax()
           << "' != end range, left end range, or right end range";
        LOG_ERROR(ITREE_LOG, "%s", ss.str().c_str());
        return false;
    }
    
    return true;
}

template<typename R, typename D> 
void
IntervalTree<R, D>::printDepthFirstSearch(
    const IntervalTreeNode<R, D> *headNodeP) const
{
    if (headNodeP == NULL) {
        headNodeP = getHeadNode();
    }

    if (headNodeP == getSentinelNode()) {
        return;
    }

    printDepthFirstSearch(headNodeP->getLeftChild());
    std::stringstream ss;
    ss << *headNodeP;
    LOG_INFO(ITREE_LOG, "%s", ss.str().c_str());
    printDepthFirstSearch(headNodeP->getRightChild());
}

template<typename R, typename D> 
void
IntervalTree<R, D>::printBreadthFirstSearch(
    const IntervalTreeNode<R, D> *headNodeP) const
{
    if (headNodeP == NULL) {
        headNodeP = getHeadNode();
    }

    std::queue<const IntervalTreeNode<R, D> *> nodeQueue;
    if (headNodeP != getSentinelNode()) {
        nodeQueue.push(headNodeP);
    }
    
    int32_t count = 0;
    int32_t currentLevel = 0;
    int32_t currentLevelNodesSeen = 0;
    int32_t currentLevelNodes = 1;
    int32_t nextLevelNodes = 0;
    const IntervalTreeNode<R, D> *tmpNodeP = NULL;
    while (nodeQueue.empty() == false) {
        tmpNodeP = nodeQueue.front();
        /* Add the children of tmpNodeP to the queue */
        if (tmpNodeP->getLeftChild() != getSentinelNode()) {
            nodeQueue.push(tmpNodeP->getLeftChild());
            nextLevelNodes++;
        }
        if (tmpNodeP->getRightChild() != getSentinelNode()) {
            nodeQueue.push(tmpNodeP->getRightChild());
            nextLevelNodes++;
        }
        
        std::stringstream ss;
        ss << *(dynamic_cast<const IntervalTreeNodeImpl<R, D> *>(tmpNodeP));
        LOG_DEBUG(ITREE_LOG, "%s", ss.str().c_str());

        currentLevelNodesSeen++;
        if (currentLevelNodesSeen == currentLevelNodes) {
            currentLevel++;
            currentLevelNodesSeen = 0;
            currentLevelNodes = nextLevelNodes;
            nextLevelNodes = 0;
            LOG_DEBUG(ITREE_LOG, "Level %d: ", currentLevel);
        }
        
        nodeQueue.pop();
        count++;
    }

    LOG_DEBUG(ITREE_LOG, "\n%d nodes found", count);
}

template<typename R, typename D> 
IntervalTreeNode<R, D> *
IntervalTree<R, D>::getTreeHead()
{
    if (m_headNodeP == getSentinelNode()) {
        return NULL;
    }
    return dynamic_cast<IntervalTreeNode<R, D> *>(m_headNodeP);
}

template<typename R, typename D> 
IntervalTreeNode<R, D> *
IntervalTree<R, D>::getTreeMinStartRangeNode(IntervalTreeNode<R, D> *headNodeP)
{
    
    IntervalTreeNodeImpl<R, D> *retNodeP = NULL;
    if (headNodeP == NULL) {
        retNodeP = getMinStartRangeNode(getHeadNode());
    }
    else {
        retNodeP = getMinStartRangeNode(
            dynamic_cast<IntervalTreeNodeImpl<R, D> *>(headNodeP));
    }
    
    if (retNodeP == getSentinelNode()) {
        return NULL;
    }
    else {
        return dynamic_cast<IntervalTreeNode<R, D> *>(retNodeP);
    }
}

template<typename R, typename D> 
IntervalTreeNode<R, D> *
IntervalTree<R, D>::getTreeMaxStartRangeNode(IntervalTreeNode<R, D> *headNodeP)
{
    
    IntervalTreeNodeImpl<R, D> *retNodeP = NULL;
    if (headNodeP == NULL) {
        retNodeP = getMaxStartRangeNode(getHeadNode());
    }
    else {
        retNodeP = getMaxStartRangeNode(
            dynamic_cast<IntervalTreeNodeImpl<R, D> *>(headNodeP));
    }
    
    if (retNodeP == getSentinelNode()) {
        return NULL;
    }
    else {
        return dynamic_cast<IntervalTreeNode<R, D> *>(retNodeP);
    }
}

template<typename R, typename D> 
IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getHeadNode()
{
    return m_headNodeP;
}

template<typename R, typename D> 
const IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getHeadNode() const
{
    return m_headNodeP;
}

template<typename R, typename D> 
void
IntervalTree<R, D>::setHeadNode(IntervalTreeNode<R, D> *nodeP)
{
    m_headNodeP = dynamic_cast<IntervalTreeNodeImpl<R, D> *>(nodeP);
}

template<typename R, typename D> 
IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::insertNodeIntoTree(R startRange,
                                       R endRange,
                                       D data)
{
    assert(startRange <= endRange);

    IntervalTreeNodeImpl<R, D> *zP = 
        new IntervalTreeNodeImpl<R, D>(startRange, endRange, data);
    
    /* All links initially point to the sentinel */
    zP->setParent(getSentinelNode());
    zP->setLeftChild(getSentinelNode());
    zP->setRightChild(getSentinelNode());

    /* 
     * Find the proper location to add the node while updating the
     * endMaxRange for intermediate nodes
     */
    IntervalTreeNodeImpl<R, D> *xP = getHeadNode();
    IntervalTreeNodeImpl<R, D> *yP = getSentinelNode();
    while (xP != getSentinelNode()) {
        yP = xP;

        if (xP->getEndRangeMax() < zP->getEndRangeMax()) {
            xP->setEndRangeMax(zP->getEndRangeMax()); 
        }

        if (zP->getStartRange() < xP->getStartRange()) {
            xP = xP->getLeftChildImpl();
        }
        else {
            xP = xP->getRightChildImpl();
        }
    }
    zP->setParent(yP);
    
    if (yP == getSentinelNode()) {
        setHeadNode(zP);
    }
    else {
        if (zP->getStartRange() < yP->getStartRange()) {
            yP->setLeftChild(zP);
        }
        else {
            yP->setRightChild(zP);
        }
    }

    return zP;
}

template<typename R, typename D> 
void
IntervalTree<R, D>::rotateLeft(IntervalTreeNodeImpl<R, D> *nodeP)
{
    IntervalTreeNodeImpl<R, D> *xP = nodeP;
    IntervalTreeNodeImpl<R, D> *yP = xP->getRightChildImpl();

    if ((xP == NULL) || (xP == getSentinelNode())) {
        std::stringstream ss;
        ss << "rotateLeft: xP is bad (" << xP << ")";
        throw Exception(ss.str());
    }
    if ((yP == NULL) || (yP == getSentinelNode())) {
        std::stringstream ss;
        ss << "rotateLeft: yP is bad (" << yP << ")";
        throw Exception(ss.str());
    }

    xP->setRightChild(yP->getLeftChildImpl());
    if (yP->getLeftChildImpl() != getSentinelNode()) {
        yP->getLeftChildImpl()->setParent(xP);
    }

    yP->setParent(xP->getParentImpl());
    if (xP->getParentImpl() == getSentinelNode()) {
        setHeadNode(yP);
    }
    else {
        if (xP == xP->getParentImpl()->getLeftChildImpl()) {
            xP->getParentImpl()->setLeftChild(yP);
        }
        else {
            xP->getParentImpl()->setRightChild(yP);
        }
    }
            
    yP->setLeftChild(xP);
    xP->setParent(yP);

    /* Preserve the endMaxRange semantics */
    updateEndRangeMax(xP);
    updateEndRangeMax(yP);
}

template<typename R, typename D> 
void
IntervalTree<R, D>::rotateRight(IntervalTreeNodeImpl<R, D> *nodeP)
{
    IntervalTreeNodeImpl<R, D> *xP = nodeP;
    IntervalTreeNodeImpl<R, D> *yP = xP->getLeftChildImpl();

    if ((xP == NULL) || (xP == getSentinelNode())) {
        std::stringstream ss;
        ss << "xP is bad (" << xP << ")";
        throw Exception(ss.str());
    }
    if ((yP == NULL) || (yP == getSentinelNode())) {
        std::stringstream ss;
        ss << "yP is bad (" << yP << ")";
        throw Exception(ss.str());
    }

    xP->setLeftChild(yP->getRightChildImpl());
    if (yP->getRightChildImpl() != getSentinelNode()) {
        yP->getRightChildImpl()->setParent(xP);
    }

    yP->setParent(xP->getParentImpl());
    if (xP->getParentImpl() == getSentinelNode()) {
        setHeadNode(yP);
    }
    else {
        if (xP == xP->getParentImpl()->getRightChildImpl()) {
            xP->getParentImpl()->setRightChild(yP);
        }
        else {
            xP->getParentImpl()->setLeftChild(yP);
        }
    }
            
    yP->setRightChild(xP);
    xP->setParent(yP);

    /* Preserve the endMaxRange semantics */
    updateEndRangeMax(xP);
    updateEndRangeMax(yP);
}

template<typename R, typename D> 
void
IntervalTree<R, D>::endRangeMaxUpdateAncestors(
    IntervalTreeNodeImpl<R, D> *nodeP)
{
    IntervalTreeNodeImpl<R, D> *tmpNodeP = nodeP;
    R newEndRangeMax;

    while (tmpNodeP->getParentImpl() != getSentinelNode()) {
        tmpNodeP = tmpNodeP->getParentImpl();
        
        /*
         * Start with the end range of this node and then compare
         * aginst the max end range of each of its children.
         */
        newEndRangeMax = tmpNodeP->getEndRange();
        if ((tmpNodeP->getLeftChildImpl() != getSentinelNode()) &&
            (tmpNodeP->getEndRange() < 
             tmpNodeP->getLeftChildImpl()->getEndRangeMax())) {
            newEndRangeMax = tmpNodeP->getLeftChildImpl()->getEndRangeMax();
        }
        if ((tmpNodeP->getRightChildImpl() != getSentinelNode()) &&
            (tmpNodeP->getEndRange() < 
             tmpNodeP->getRightChildImpl()->getEndRangeMax())) {
            newEndRangeMax = tmpNodeP->getRightChildImpl()->getEndRangeMax();
        }

        /* 
         * If everything is the same, then no need to proceed up
         * further.
         */
        if (newEndRangeMax == tmpNodeP->getEndRangeMax()) {
            break;
        }
        else {
            tmpNodeP->setEndRangeMax(newEndRangeMax);
        }
    }
}

template<typename R, typename D> 
void
IntervalTree<R, D>::deleteFixUp(IntervalTreeNodeImpl<R, D> *nodeP)
{
    IntervalTreeNodeImpl<R, D> *xP = nodeP;
    IntervalTreeNodeImpl<R, D> *wP = getSentinelNode();
    
    while ((xP != getHeadNode()) && 
           (xP->getColor() == IntervalTreeNode<R, D>::BLACK)) {
        if (xP == xP->getParentImpl()->getLeftChildImpl()) { 
            wP = xP->getParentImpl()->getRightChildImpl();
            if (wP->getColor() == 
                IntervalTreeNode<R, D>::RED) {
                wP->setColor(IntervalTreeNode<R, D>::BLACK);
                xP->getParentImpl()->setColor(IntervalTreeNode<R, D>::RED);
                rotateLeft(xP->getParentImpl());
                wP = xP->getParentImpl()->getRightChildImpl();
            }
            if ((wP->getLeftChildImpl()->getColor() == 
                 IntervalTreeNode<R, D>::BLACK) &&
                (wP->getRightChildImpl()->getColor() == 
                 IntervalTreeNode<R, D>::BLACK)) {
                wP->setColor(IntervalTreeNode<R, D>::RED);
                xP = xP->getParentImpl();
            }
            else {
                if (wP->getRightChildImpl()->getColor() ==
                    IntervalTreeNode<R, D>::BLACK) {
                    wP->getLeftChildImpl()->setColor(
                        IntervalTreeNode<R, D>::BLACK);
                    wP->setColor(IntervalTreeNode<R, D>::RED);
                    rotateRight(wP);
                    wP = xP->getParentImpl()->getRightChildImpl();
                }
                wP->setColor(xP->getParentImpl()->getColor());
                xP->getParentImpl()->setColor(
                    IntervalTreeNode<R, D>::BLACK);
                wP->getRightChildImpl()->setColor(
                    IntervalTreeNode<R, D>::BLACK);
                rotateLeft(xP->getParentImpl());
                xP = getHeadNode();
            }
        }
        else {
            wP = xP->getParentImpl()->getLeftChildImpl();
            if (wP->getColor() == 
                IntervalTreeNode<R, D>::RED) {
                wP->setColor(IntervalTreeNode<R, D>::BLACK);
                xP->getParentImpl()->setColor(IntervalTreeNode<R, D>::RED);
                rotateRight(xP->getParentImpl());
                wP = xP->getParentImpl()->getLeftChildImpl();
            }
            if ((wP->getRightChildImpl()->getColor() == 
                 IntervalTreeNode<R, D>::BLACK) &&
                (wP->getLeftChildImpl()->getColor() == 
                 IntervalTreeNode<R, D>::BLACK)) {
                wP->setColor(IntervalTreeNode<R, D>::RED);
                xP = xP->getParentImpl();
            }
            else {
                if (wP->getLeftChildImpl()->getColor() ==
                    IntervalTreeNode<R, D>::BLACK) {
                    wP->getRightChildImpl()->setColor(
                        IntervalTreeNode<R, D>::BLACK);
                    wP->setColor(IntervalTreeNode<R, D>::RED);
                    rotateLeft(wP);
                    wP = xP->getParentImpl()->getLeftChildImpl();
                }
                wP->setColor(xP->getParentImpl()->getColor());
                xP->getParentImpl()->setColor(
                    IntervalTreeNode<R, D>::BLACK);
                wP->getLeftChildImpl()->setColor(
                    IntervalTreeNode<R, D>::BLACK);
                rotateRight(xP->getParentImpl());
                xP = getHeadNode();
            }
        }
    }

    xP->setColor(IntervalTreeNode<R, D>::BLACK);
}

template<typename R, typename D> 
const IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getSentinelNode() const
{
    return &m_sentinelNode;
}

template<typename R, typename D> 
IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getSentinelNode()
{
    return &m_sentinelNode;
}

template<typename R, typename D>
IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getMinStartRangeNode(IntervalTreeNodeImpl<R, D> *nodeP)
{
    while ((nodeP != getSentinelNode()) &&
           (nodeP->getLeftChildImpl() != getSentinelNode())) {
        nodeP = nodeP->getLeftChildImpl();
    }
    
    return nodeP;
}

template<typename R, typename D>
IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getMaxStartRangeNode(IntervalTreeNodeImpl<R, D> *nodeP)
{
    while ((nodeP != getSentinelNode()) &&
           nodeP->getRightChildImpl() != getSentinelNode()) {
        nodeP = nodeP->getRightChildImpl();
    }
    
    return nodeP;
}

template<typename R, typename D> 
IntervalTreeNodeImpl<R, D> *
IntervalTree<R, D>::getSuccesor(IntervalTreeNodeImpl<R, D> *nodeP)
{
    if ((nodeP == NULL) || (nodeP == getSentinelNode())) {
        std::stringstream ss;
        ss << "getSuccesor: nodeP is bad (" << nodeP << ")";
        throw Exception(ss.str());
    }
    IntervalTreeNodeImpl<R, D> *xP = nodeP;

    if (xP->getRightChildImpl() != getSentinelNode()) {
        return getMinStartRangeNode(xP->getRightChildImpl());
    }

    IntervalTreeNodeImpl<R, D> *yP = xP->getParentImpl();
    while ((yP != getSentinelNode()) && (xP == yP->getRightChildImpl())) {
        xP = yP;
        yP = yP->getParentImpl();
    }
    
    return yP;
}

}   /* end of 'namespace clusterlib' */

#endif
