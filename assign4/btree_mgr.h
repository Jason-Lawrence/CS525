#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dberror.h"
#include "tables.h"
#include "buffer_mgr.h"

#define LEAF    0
#define NONLEAF 1

// structure for accessing btrees
typedef struct node_t{
  int      type;
  int      occupancy;
  Value   *keys;
  void   **ptrs;
}node_t;
typedef struct MetaData{
  int treeOrder;
  int numLeafs;
  int numNodes;
  DataType kt;
  node_t  *root;
  Value   **keys;
}MetaData;
typedef struct BT_Mgr{
  BM_BufferPool *bm;
  BM_PageHandle *page;
  MetaData      *md;
}BT_Mgr;
typedef struct BTreeHandle {
  DataType  keyType;
  char     *idxId;
  BT_Mgr   *mgmtData;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  int scanned;
  Value *key;
  void *mgmtData;
} BT_ScanHandle;


// init and shutdown index manager
extern RC initIndexManager (void *mgmtData);
extern RC shutdownIndexManager ();

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n);
extern RC openBtree (BTreeHandle **tree, char *idxId);
extern RC closeBtree (BTreeHandle *tree);
extern RC deleteBtree (char *idxId);

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result);
extern RC getNumEntries (BTreeHandle *tree, int *result);
extern RC getKeyType (BTreeHandle *tree, DataType *result);

// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey (BTreeHandle *tree, Value *key);
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry (BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan (BT_ScanHandle *handle);

// debug and test functions
extern char *printTree (BTreeHandle *tree);

// custume functions
node_t *createNode(int n, int type);
void insertValue(node_t *node, Value *key, void *ptr, int n);
void deleteValue(node_t *node, Value *key, int n);
int isGreaterThan(Value left, Value right);

#define BufferPool_Size 25
#define KEY_LIMIT 100

#define GET_TREE_INFO(tree) ((BT_Mgr *)tree->mgmtData)
#define GET_METADATA(bt)    ((MetaData *)bt->md)

#endif // BTREE_MGR_H