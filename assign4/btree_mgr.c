#include "btree_mgr.h"
#include "string.h"

// init and shutdown index manager

extern RC initIndexManager (void *mgmtData){
  return RC_OK;
}
extern RC shutdownIndexManager (){
  return RC_OK;
}

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n){
  printf("Info btree_mgr.c - createBtree: creating btree files\n");
  FILE *tree = fopen(idxId, "wb");
  if (tree == NULL) return RC_WRITE_FAILED;
  fclose(tree);
  char buffer[80], fileName[50];
  strcpy(fileName, idxId);
  strcpy(buffer, "MetaData.txt");
  strcat(fileName, buffer);
  FILE *fp = fopen(fileName, "wb");
  if (fp == NULL) return RC_WRITE_FAILED;

  // Write Meta Data Structure
  MetaData *md  = (MetaData *) malloc(sizeof(MetaData));
  md->treeOrder = n;
  md->numLeafs  = 0;
  md->numNodes  = 0;
  md->kt        = keyType;
  md->keys      = malloc(sizeof(Value) * KEY_LIMIT);
  md->root      = createNode(n, LEAF);
// tree->mgmtData->md->root = createNode(int n, int type);
  // write tree meta data to the first page
  fwrite(md, sizeof(MetaData), 1, fp);
  // free any reserved space
  fclose(fp);

  printf("Info btree_mgr.c - createBtree: finished\n");
  return RC_OK;
}

extern RC openBtree (BTreeHandle **tree, char *idxId){
  printf("Info btree_mgr.c - openBtree: Opening btree file\n");
  char buffer[80], fileName[50];
  strcpy(fileName, idxId);
  strcpy(buffer, "MetaData.txt");
  strcat(fileName, buffer);
  FILE *fp = fopen(fileName, "rb+");
  if (fp == NULL) return RC_WRITE_FAILED;
  // read in the MetaData from file
  printf("Info btree_mgr.c - openBtree: Reading MetaData File\n");
  MetaData *md  = (MetaData *) malloc(sizeof(MetaData));
  fread(md, sizeof(MetaData), 1, fp);
  fclose(fp);
  printf("Info btree_mgr.c - openBtree: Closing MetaData File\n");
  //initialize BT_Mgr
  (*tree) = (BTreeHandle *) malloc(sizeof(BTreeHandle));
  (*tree)->idxId = idxId;
  (*tree)->keyType = md->kt;
  BT_Mgr *bt = malloc(sizeof(BT_Mgr));
  bt->bm     = MAKE_POOL();
  bt->page   = MAKE_PAGE_HANDLE();
  //bt->md     = (MetaData *) malloc(sizeof(MetaData));
  bt->md     = md;
  //bt->md->keys   = malloc(sizeof(Value) * KEY_LIMIT);
  (*tree)->mgmtData = bt;
  //free(md);
  printf("Info btree_mgr.c - openBtree: Initializing the Buffer Manager\n");
  RC res = initBufferPool(bt->bm, idxId, BufferPool_Size, RS_LRU, NULL);
  if (res != RC_OK) return res;
  printf("Info btree_mgr.c - openBtree: finished\n");
  return RC_OK;
}
extern RC closeBtree (BTreeHandle *tree){
  BT_Mgr *bt = GET_TREE_INFO(tree);
  char buffer[80], fileName[50];
  strcpy(fileName, tree->idxId);
  strcpy(buffer, "MetaData.txt");
  strcat(fileName, buffer);
  FILE *fp = fopen(fileName, "rb+");
  if (fp == NULL) return RC_WRITE_FAILED;
  //write metadata to page
  fwrite(bt->md, sizeof(MetaData), 1, fp);
  fclose(fp);
  //free
  RC res = shutdownBufferPool(bt->bm);
  if (res != RC_OK) return res;
  free(bt->bm);
  free(bt->page);
  free(bt->md->root);
  for (int i = 0; i < KEY_LIMIT; i++){
    free(bt->md->keys[i]);
  }
  free(bt->md);
  free(bt);
  return RC_OK;
}
extern RC deleteBtree (char *idxId){
  char buffer[80], fileName[50];
  strcpy(fileName, idxId);
  strcpy(buffer, "MetaData.txt");
  strcat(fileName, buffer);
  destroyPageFile(fileName);
  destroyPageFile(idxId);
  return RC_OK;
}

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result)
{
  *result = tree->mgmtData->md->numNodes;
  return RC_OK;
}
extern RC getNumEntries (BTreeHandle *tree, int *result)
{
  *result = tree->mgmtData->md->numLeafs;
  return RC_OK;
}
extern RC getKeyType (BTreeHandle *tree, DataType *result)
{
  *result = tree->mgmtData->md->kt;
  return RC_OK;
}

// Utility functions
node_t *createNode(int n, int type)
{
  node_t *node = malloc(sizeof(node_t));
  node->keys   = malloc(sizeof(Value ) * n);            // TODO: IMPORTANT!!! I am assuming all the data types are ints. int needs to be adjusted
  node->ptrs   = malloc(sizeof(void *) * (n+1));// Can change the void pointer to a node_t pointer if this is only used for non leaf nodes
  node->type   = type;
  node->occupancy = 0;

  for (int i = 0; i < n; i++){
    node->keys[i].v.intV = 0;
    node->keys[i].dt   = DT_INT;
    node->ptrs[i]      = NULL;
  }
  node->ptrs[n] = NULL;

  return node;
}
void deleteNode(node_t *node)
{
  free(node->ptrs);
  free(node->keys);
  free(node);
}
node_t *getLeftSibling(){}
node_t *getRightSibling(){}
node_t *splitNode(node_t *fullNode, Value *key, void *ptr, int n, int type)
{
  if (DEBUG) printf("INFO  btree_mgr.c - splitNode: Creating new node\n");
  node_t *newNode = createNode(n, type);
  // Check if n is odd
  if (n & 1){
    if (DEBUG) printf("INFO  btree_mgr.c - splitNode: Applying n is odd rules\n");


    for (int i = n; i > (n+1)/2; i--){
      //fullNode->key[i];
      insertValue(newNode, &fullNode->keys[i-1], fullNode->ptrs[i], n);
      deleteValue(fullNode, &fullNode->keys[i-1], n);
    }

    if (isGreaterThan(*key, fullNode->keys[fullNode->occupancy - 1])){
      insertValue(newNode, key, ptr, n);
    }else{
      insertValue(newNode, &fullNode->keys[fullNode->occupancy - 1], fullNode->ptrs[fullNode->occupancy], n);
      deleteValue(fullNode, &fullNode->keys[fullNode->occupancy - 1], n);
      insertValue(fullNode, key, ptr, n);
    }


    if (newNode->type == NONLEAF){
      // The  new node will not have any pointer in the very first pointer spot
      // This is only true if it is a non leaf node
      // Need to do the math that this deletion will not cause an underflow TODO
      newNode->ptrs[0] = fullNode->ptrs[fullNode->occupancy];
      deleteValue(fullNode, &fullNode->keys[fullNode->occupancy - 1], n);
    }

    if (DEBUG) printf("TODO  btree_mgr.c - splitNode: Need to perform split\n");
    return newNode;
  }else{
    if (DEBUG) printf("INFO  btree_mgr.c - splitNode: Applying n is even rules\n");
    // n is even
    // TODO: split appropriate values into each node
    //       put the larger values into newNode please

    for (int i = n; i > n/2; i--){
      insertValue(newNode, &fullNode->keys[i-1], fullNode->ptrs[i], n);
      deleteValue(fullNode, &fullNode->keys[i-1], n);
    }

    if (newNode->type == NONLEAF){
      // The  new node will not have any pointer in the very first pointer spot
      //       // This is only true if it is a non leaf node
      //        Need to do the math that this deletion will not cause an underflow TODO
      newNode->ptrs[0] = fullNode->ptrs[fullNode->occupancy];
      deleteValue(fullNode, &fullNode->keys[fullNode->occupancy - 1], n);
    }


    if (DEBUG) printf("INFO  btree_mgr.c - splitNode: Nodes are split left = %d, right = %d, n = %d\n", fullNode->occupancy, newNode->occupancy, n);
    return newNode;
  }

}


int areValuesEqual(Value left, Value right)
{
  switch(left.dt){
    case DT_INT:
      if (left.v.intV == right.v.intV) return 1;
      else return 0;
      break;
    case DT_STRING:
      printf("TODO  btree_mgr.c - isGreaterThan: How can you order a string?\n");
      return 0;
      break;
    case DT_FLOAT:
      if (left.v.floatV == right.v.floatV) return 1;
      else return 0;
      break;
    case DT_BOOL:
      if (left.v.boolV == right.v.boolV) return 1;
      else return 0;
      break;
  }
}


int isGreaterThan(Value left, Value right)
{
  switch(left.dt){
    case DT_INT:
      if (left.v.intV >= right.v.intV) return 1;
      else return 0;
      break;
    case DT_STRING:
      printf("TODO  btree_mgr.c - isGreaterThan: How can you order a string?\n");
      return 0;
      break;
    case DT_FLOAT:
      if (left.v.floatV >= right.v.floatV) return 1;
      else return 0;
      break;
    case DT_BOOL:
      printf("TODO  btree_mgr.c - isGreaterThan: How can you order a boolean?\n");
      if (left.v.boolV >= right.v.boolV) return 1;
      else return 0;
      break;
  }
}


int isLessThan(Value left, Value right)
{
  switch(left.dt){
    case DT_INT:
      if (left.v.intV < right.v.intV) return 1;
      else return 0;
      break;
    case DT_STRING:
      printf("TODO  btree_mgr.c - isGreaterThan: How can you order a string?\n");
      return 0;
      break;
    case DT_FLOAT:
      if (left.v.floatV < right.v.floatV) return 1;
      else return 0;
      break;
    case DT_BOOL:
      printf("TODO  btree_mgr.c - isGreaterThan: How can you order a boolean?\n");
      if (left.v.boolV < right.v.boolV) return 1;
      else return 0;
      break;
  }
}



// Insert a value into a node that is not full
void insertValue(node_t *node, Value *key, void *ptr, int n)
{
  if (DEBUG) printf("INFO  btree_mgr.c - insertValue: Initializing values\n");
  Value curKey  = *key;
  void *curPtr  =  ptr;
  Value tempKey;
  void *tempPtr;
  int   adder   = node->type; // 0 for leaf and 1 for nonleaf

  if (node->occupancy >= n) return;

  if (DEBUG) printf("INFO  btree_mgr.c - insertValue: Iterating to keep entries in order\n");
  for (int i=0; i <= node->occupancy; i++){
    if (i == node->occupancy){
      node->keys[i] = curKey;
      node->ptrs[i + adder] = curPtr;
      node->occupancy = node->occupancy + 1;
      break;
    }
    if (isGreaterThan( node->keys[i],  curKey)){
      if (DEBUG) printf("INFO  btree_mgr.c - insertValue: Found key that needs to be moved\n");
      // Move the value down
      tempKey = node->keys[i];
      node->keys[i] = curKey;
      curKey  = tempKey;

      // Move the pointer down
      tempPtr = node->ptrs[i + adder];
      node->ptrs[i + adder] = curPtr;
      curPtr  = tempPtr;
    }
  }
}


// remove a value from a node that is not empty
void deleteValue(node_t *node, Value *key, int n)
{
  int adder = node->type;

  for (int i = 0; i < node->occupancy; i++){
    if (areValuesEqual(node->keys[i], *key)){
      if (DEBUG) printf("INFO  btree_mgr.c - deleteValue: Deleting value at pos %d\n", i);
      for(int a = i; a < node->occupancy - 1; a++){
        node->keys[i]   = node->keys[i+1];
        node->ptrs[i+adder] = node->ptrs[i+1+adder];
      }
      node->occupancy = node->occupancy - 1;
      break;
    }
  }
}


node_t *findChild(node_t *node, Value *key, int n)
{
  if (DEBUG) printf("INFO  btree_mgr.c - findChild: Finding relevent child\n");
  for (int i = 0; i < node->occupancy; i++){
    if(isGreaterThan(node->keys[i], *key)){
      continue;
    }else{
      return node->ptrs[i];
    }
  }
  return node->ptrs[node->occupancy];


}


node_t *insertIntoNode(node_t *node, Value *key, int n)
{
  if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: Initializing values\n");
  node_t *newNode;
  node_t *child;
  node_t *newChild;


  // base case of recurrsion
  // Keep going until leaf node is found
  if(node->type == LEAF){
    if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: Found leaf to insert value\n");
    // Check for overflow
    if (node->occupancy == n){
       if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: Overflow detected in leaf (n = %d)\n", n);
       return splitNode(node, key, NULL, n, LEAF);
    } else {
      //No overflow
      if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: Inserting value into leaf\n");
      insertValue(node, key, NULL, n);
      return NULL;
    }
  }



  child    = findChild(node, key, n);
  newChild = insertIntoNode(child, key, n);
  if (newChild == NULL){
    // No new nodes added
    if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: No new children, nothing to do\n");
    return NULL;
  }else {
    if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: There are new children that need to be added to parent\n");
    if(node->occupancy == n){
      // Overflow detected
      if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: Overflow detected in parent\n");
      newNode = splitNode(node, newChild->keys, newChild, n, NONLEAF); // The second value should be changed to the smallestValue in the subtree

      return newNode;
    }else{
      if (DEBUG) printf("INFO  btree_mgr.c - insertIntoNode: Adding new child to parent\n");
      insertValue(node, newChild->keys, newChild, n); // TODO fix null pointer and give the correct 2nd argument
      return NULL;
    }
  }

}
Value *findSmallestValue(node_t *node)
{
  // Given a subtree
  // This function should keep searching the left value until a min is found
  return node->keys;
}


node_t *findLeaf(node_t *node, Value *key, int n)
{
  if (node->type == LEAF){
    // base case found
    return node;
  }

  for (int i = 0; i < node->occupancy; i++){
    if (isLessThan(*key, node->keys[i])){
      return findLeaf(node->ptrs[i], key, n);
    }
  }
  return findLeaf(node->ptrs[node->occupancy], key, n);
}


// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result){}
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
  if (DEBUG) printf("INFO  btree_mgr.c - insertKey: Initializing for key insertion\n");
  // Insert key into tree, starting at root
  int     order   = tree->mgmtData->md->treeOrder;
  node_t *ogRoot  = tree->mgmtData->md->root;
  node_t *newNode = insertIntoNode(ogRoot, key, order);

  // determine if new root needs to be made
  if (newNode != NULL){
    // create new root
    if (DEBUG) printf("INFO  btree_mgr.c - insertKey: Creating new root\n");
    node_t *newRoot = createNode(order, NONLEAF);

    // insert the one value into the root and ppoint to the old root and new node
    if (DEBUG) printf("INFO  btree_mgr.c - insertKey: Getting values for new root\n");
    node_t *leftPtr  = ogRoot;
    node_t *rightPtr = newNode; // TODO: make sure rightNode is > leftPtrs
    Value  *minValue = findSmallestValue(rightPtr); // TODO: finish this function

    if (DEBUG) printf("INFO  btree_mgr.c - insertKey: Setting values in new root\n");
    newRoot->occupancy = 1;
    newRoot->keys[0] = *minValue;
    newRoot->ptrs[0] = leftPtr;
    newRoot->ptrs[1] = rightPtr;
    newRoot->type    = NONLEAF;

    tree->mgmtData->md->root = newRoot;
  }
  if (DEBUG) printf("INFO  btree_mgr.c - insertKey: Insertion completed\n");
  if (DEBUG) printf("--------------------------------------------------------------\n");
  return RC_OK;
}
extern RC deleteKey (BTreeHandle *tree, Value *key){}
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle){
  BT_Mgr *bt = GET_TREE_INFO(tree);
  (*handle)->tree = tree;
  (*handle)->scanned = 0; //set the amount of leafs scanned to be 0
  (*handle)->key = bt->md->keys[0]; //Have the inital key be the lowest.
  return RC_OK;
}
extern RC nextEntry (BT_ScanHandle *handle, RID *result){
  BT_Mgr *bt = GET_TREE_INFO(handle->tree);
  // check to see if there are no more entries to scanned.
  if(handle->scanned == bt->md->numLeafs) return RC_IM_NO_MORE_ENTRIES;
  // retrieve the key to serach for.
  Value *k   = handle->key;
  RC res;
  // retrieve the record
  res = findKey(handle->tree, k, result);
  if (res != RC_OK) return res;
  handle->scanned++;
  // get next the next key.
  int i;
  for(i = 0; i < bt->md->numLeafs; i++){
    if (bt->md->keys[i] == k && bt->md->keys[i+1] != NULL){
     handle->key = bt->md->keys[i++];
     break;
    }
  }
  // return RC_OK
  return RC_OK;
}
extern RC closeTreeScan (BT_ScanHandle *handle){
  handle->scanned = 0;
  handle->key = NULL;
  handle->tree = NULL;
  return RC_OK;
}

// debug and test functions
void *printLeaf(node_t *node, char *buffer, int count){
  char buf[80];
  char tmp[20];
  RID *rid;
  Value val;
  sprintf(buf, "(%d)[", count);
  for (int i = 0; i < node->occupancy; i++){
    rid = (RID *)node->ptrs[i];
    val = node->keys[i];
    sprintf(tmp, "%d.%d, %d", rid->page, rid->slot, val.v.intV);
    strcat(buf, tmp);
    if(i+1 < node->occupancy) strcat(buf, ", ");
  }
  sprintf(tmp, "%d]\n", count+1);
  strcat(buf, tmp);
  strcat(buffer, buf);
}

void *printNonLeaf(node_t *node, char *buffer, int count){
  char buf[80];
  char tmp[20];
  sprintf(buf, "(%d)[", count);
  Value val;
  int i;
  for(i = 0; i < node->occupancy; i++){
    val = node->keys[i];
    sprintf(tmp, "%d, %d", count+i, val.v.intV);
    strcat(buf, tmp);
    if(i+1 < node->occupancy) strcat(buf, ", ");
  }
  if (node->ptrs[i+1] != NULL){
    sprintf(tmp, "%d", i+count);
    strcat(buf, tmp);
  }
  strcat(buf, "]\n");
  strcat(buffer, buf);
}

void *printRoot(node_t *root, char *buffer){
  char buf[80];
  char tmp[20];
  Value val;
  sprintf(buf, "(0)[");
  int i;
  for(i = 0; i < root->occupancy; i++){
    val = root->keys[i];
    sprintf(tmp, "%d, %d", i+1, val.v.intV);
    if (i+1 < root->occupancy) strcat(tmp, ", ");
    strcat(buf, tmp);
  }
  if (root->ptrs[i+1] != NULL){
    sprintf(tmp, "%d", i+1);
    strcat(buf, tmp);
  }
  strcat(buf, "]\n");
  strcat(buffer, buf);
}

char *printTreeNodes(node_t *node, char *buffer, int count, int height){
  char tmp[80];
  if(count == 0){
    printRoot(node, buffer);
    count++;
    for (int i = 0; i < node->occupancy; i++){
      strcat(buffer, printTreeNodes(node->ptrs[i], buffer, count + (i*((n+1)*height)), height));
      count++;
    }
    return buffer;
  }

  else if(node->type == LEAF){
    printLeaf(node, buffer, count);
    return buffer;
  }
  //node->type == NONLEAF
  else{
    for(int i = 0; i < node->occupancy; i++){
      printTreeNodes(node->ptrs[i], buffer, count);
      count++;
    }
    printNonLeaf(node, buffer, count);
    return buffer;
  }
}

void *printRootLeaf(node_t *root, char *buffer){
  char buf[80];
  char tmp[20];
  sprintf(buf, "(0)[");
  RID *rid;
  Value val;
  int i;
  for(i = 0; i < root->occupancy; i++){
    rid = (RID *)root->ptrs[i];
    val = root->keys[i];
    sprintf(tmp, "%d.%d, %d", rid->page, rid->slot, val.v.intV);
    strcat(buf, tmp);
    if (i+1 < root->occupancy) strcat(buf, ", ");
  }
  if (root->ptrs[i+1] != NULL){
    sprintf(tmp, "%d", i+1);
    strcat(buf, tmp);
  }
  strcat(buf, "]\n");
  strcpy(buffer, buf);
}

int get_height(node_t *node){
  if (node->type == LEAF){
    return 1
  }else{
    return 1 + get_height(node->ptrs[0]);
  }
}

// debug and test functions use sprintf()
extern char *printTree (BTreeHandle *tree){
  BT_Mgr *bt = GET_TREE_INFO(tree);
  node_t *node = bt->md->root;
  int height = get_height(node->ptrs[0]);
  char *buffer = malloc(sizeof(char) * 120);
  int count = 0;
  if (node->type == LEAF){
    printRootLeaf(node, buffer);
    return buffer;
  }
  return printTreeNodes(node, buffer, count, height);
}
