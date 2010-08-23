#include "clusterlibinternal.h"
#include "testparams.h"
#include "MPITestFixture.h"

extern TestParams globalTestParams;

using namespace clusterlib;
using namespace std;
using namespace boost;

class ClusterlibIntervalTree : public MPITestFixture
{
    CPPUNIT_TEST_SUITE(ClusterlibIntervalTree);
    CPPUNIT_TEST(testIntervalTree1);
    CPPUNIT_TEST(testIntervalTree2);
    CPPUNIT_TEST(testIntervalTree3);
    CPPUNIT_TEST(testIntervalTree4);
    CPPUNIT_TEST(testIntervalTree5);
    CPPUNIT_TEST(testIntervalTree6);
    CPPUNIT_TEST(testIntervalTree7);
    CPPUNIT_TEST_SUITE_END();

  public:
    
    ClusterlibIntervalTree() 
        : MPITestFixture(globalTestParams) {}

    /* Runs prior to each test */
    virtual void setUp() 
    {
    }

    /* Runs after each test */
    virtual void tearDown() 
   {
        cleanAndBarrierMPITest(NULL, false);
    }

    /* 
     * Simple test to add 20 nodes,
     */
    void testIntervalTree1()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree1");
        
        IntervalTree<int, int *> tree(-1, NULL);
        for (int i = 0; i < 20; i ++) {
            tree.insertNode(i, i + 20, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
    }

    /* 
     * Simple test to add/remove 10 contiguous node.  Remove them one
     * by one in-order and backwards.
     */
    void testIntervalTree2()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree2");

        IntervalTreeNode<int, int *> *node = NULL;        
        IntervalTree<int, int *> tree(-1, NULL);
        for (int i = 0; i < 10*10; i += 10) {
            tree.insertNode(i, i + 9, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        for (int i = 0; i < 10*10; i += 10) {
            node = tree.intervalSearch(i, i);
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(
                (node->getStartRange() + 9) == node->getEndRange());
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }        
        for (int i = 0; i < 10*10; i += 10) {
            tree.insertNode(i, i + 9, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        for (int i = 90; i >= 0; i -= 10) {
            node = tree.intervalSearch(i, i);
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(
                (node->getStartRange() + 9) == node->getEndRange());
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }        
    }

    /* 
     * Simple test to add/remove 10 contiguous node.  Remove them one
     * by one from the head.
     */
    void testIntervalTree3()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree3");
        
        IntervalTree<int, int *> tree(-1, NULL);
        for (int i = 0; i < 10*10; i += 10) {
            tree.insertNode(i, i + 9, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        IntervalTreeNode<int, int *> *node;
        for (int i = 0; i < 10; i++) {
            node = tree.getTreeHead();
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(
                (node->getStartRange() + 9) == node->getEndRange());
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }        
    }

    /* 
     * Simple test to add/remove 41 contiguous node.  Remove them one
     * by one with the min and the max functions.
     */
    void testIntervalTree4()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree4");
        
        IntervalTree<int, int *> tree(-1, NULL);
        IntervalTreeNode<int, int *> *node = NULL;
        for (int i = 0; i < 41; i++) {
            tree.insertNode(i*10, i*10 + 9, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        for (int i = 0; i < 41; i++) {
            node = tree.getTreeMinStartRangeNode();
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(
                (node->getStartRange() + 9) == node->getEndRange());
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }        
        for (int i = 0; i < 41; i++) {
            tree.insertNode(i*10, i*10 + 9, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        for (int i = 0; i < 41; i++) {
            node = tree.getTreeMaxStartRangeNode();
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(
                (node->getStartRange() + 9) == node->getEndRange());
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }        
    }

    /* 
     * Iterator tests.
     */
    void testIntervalTree5()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree3");
        
        IntervalTree<int, int *> tree(-1, NULL);
        for (int i = 0; i < 10*10; i += 10) {
            tree.insertNode(i, i + 9, -1, NULL);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        IntervalTree<int, int *>::iterator it = tree.begin();
        int count = 0;
        while (it != tree.end()) {
            MPI_CPPUNIT_ASSERT(it->getStartRange() == count * 10);
            MPI_CPPUNIT_ASSERT(it->getEndRange() == ((count * 10) + 9));
            ++it;
            count++;
        }
        
        MPI_CPPUNIT_ASSERT(count == 10);
    }

    /* 
     * Simple test to add/remove 10 identical nodes with the same
     * data.  Remove them one by one from the min.
     */
    void testIntervalTree6()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree6");
        
        IntervalTree<int, int > tree(-1, -1);
        for (int i = 0; i < 5; ++i) {
            tree.insertNode(0, 0, -1, i);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        for (int i = 9; i >= 5; --i) {
            tree.insertNode(0, 0, -1, i);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        IntervalTreeNode<int, int> *node = NULL;
        for (int i = 0; i < 10; ++i) {
            node = tree.getTreeMinStartRangeNode();
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(node->getData() == i);
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
    }

    /* 
     * Simple test to add/remove 10 identical nodes with the same
     * data.  Remove them one by one from the max.
     */
    void testIntervalTree7()
    {
        initializeAndBarrierMPITest(-1, 
                                    true, 
                                    NULL,
                                    false, 
                                    "testIntervalTree7");
        
        IntervalTree<int, int > tree(-1, -1);
        for (int i = 0; i < 5; ++i) {
            tree.insertNode(0, 0, -1, i);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        for (int i = 9; i >= 5; --i) {
            tree.insertNode(0, 0, -1, i);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
        IntervalTreeNode<int, int> *node = NULL;
        for (int i = 9; i >= 0; --i) {
            node = tree.getTreeMaxStartRangeNode();
            MPI_CPPUNIT_ASSERT(node);
            MPI_CPPUNIT_ASSERT(node->getData() == i);
            cerr << "node=" << node->getData() << ",i=" << i << endl;;
            delete tree.deleteNode(node);
            MPI_CPPUNIT_ASSERT(tree.verifyTree());
        }
    }
};

/* Registers the fixture into the 'registry' */
CPPUNIT_TEST_SUITE_REGISTRATION(ClusterlibIntervalTree);

