#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "tables.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

RM_Record_Mgr *rm;

RC initRecordManager (void *mgmtData){
  // in the test file they only ever pass in NULL so i don't know what to do with this
  //rm = (RM_Record_Mgr *) malloc(sizeof(RM_Record_Mgr));
  //rm->metadataPage = malloc(sizeof(BM_PageHandle));
  //rm->rel = malloc(sizeof(RM_TableData));
  return RC_OK;
}

RC shutdownRecordManager(){
  if(DEBUG) printf("record_mgr.c - shutdownRecordManager: Shutting everything down and free all space\n");
  free(rm->metadata);
  free(rm->bm);
  free(rm);
  return RC_OK;
}
RC createTable (char *name, Schema *schema){
  RC status;
  int r_size = getRecordSize(schema);
  int recsPerPage = PAGE_SIZE / r_size;
  if(DEBUG) printf("record_mgr.c - createTable: initailizing table\n");
  //RM_Record_Mgr *rm = malloc(sizeof(RM_Record_Mgr));
  BM_BufferPool *bm = MAKE_POOL();
  if(DEBUG) printf("record_mgr.c - createTable: initializing BufferPool\n");
  initBufferPool(bm, name, BufferPool_Size, RS_LRU, NULL);
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  pinPage(bm, h, 0);
  if(DEBUG) printf("record_mgr.c - createTable: Writing Metadata\n");
  char metadata[PAGE_SIZE];
  h->data  = metadata;
  *(int*)h->data = 0; //Number of Tuples
  h->data += sizeof(int);
  *(int*)h->data = recsPerPage;
  h->data += sizeof(int);
  *(int*)h->data = schema->numAttr; //stores the number of attributes in a table
  h->data += sizeof(int);
  *(int*)h->data = schema->keySize; //Stores the size of the key.
  h->data += sizeof(int);
  printf("record_mgr.c - createTable: writing attrnames\n");
  for(int j = 0; j < schema->numAttr; j++){
    //printf("%s\n", schema->attrNames[j]);
    strncpy(h->data, schema->attrNames[j], Attr_Size);
    h->data += Attr_Size;
    *(int*)h->data = schema->dataTypes[j];
    h->data += sizeof(int);
    *(int*)h->data = schema->typeLength[j];
    h->data += sizeof(int);
    *(int*)h->data = schema->keyAttrs[j];
    h->data += sizeof(int);
  } 
  printf("record_mgr.c - createTable: Done building Metadata\n");
  //sprintf(h->data, "%s", metadata);
  markDirty(bm, h);
  unpinPage(bm, h);
  forcePage(bm, h);
  if(DEBUG) printf("record_mgr.c - createTable: MetaData Page Written\n");
  free(h);
  shutdownBufferPool(bm);
  free(bm);
  //free(rm->bm);
  if(DEBUG) printf("record_mgr.c - createTable: Done\n");
  
  return RC_OK;  
}
RC openTable (RM_TableData *rel, char *name){ 
  int inc = sizeof(int);
  RM_Record_Mgr *rm = malloc(sizeof(RM_Record_Mgr));
  rm->bm = MAKE_POOL();
  rm->metadata = MAKE_PAGE_HANDLE();
  rm->page = MAKE_PAGE_HANDLE();
  rel->mgmtData = rm;
  if(DEBUG) printf("record_mgr.c - openTable: Initializing the BufferPool\n");
  initBufferPool(rm->bm, name, BufferPool_Size, RS_LRU, NULL);
  if(DEBUG) printf("record_mgr.c - openTable: Initializing Relation\n");
  Schema *schema = malloc(sizeof(Schema));
  rel->schema = schema;
  rel->name = name;
  if(DEBUG) printf("record_mgr.c - openTable: Pinning MetaData Page\n");
  pinPage(rm->bm, rm->metadata, 0);

  //Begin Reading metadata
  if(DEBUG) printf("record_mgr.c - openTable: Begin Reading MetaData and Initialize data struct\n");
  char *metadata = rm->metadata->data;
  printf("Probably gonna segfault\n");
  rm->numTup = *(int*)metadata;
  metadata += inc;
  rm->recordsPerPage = *(int*)metadata;
  metadata += inc;
  rel->schema->numAttr = *(int*)metadata;
  metadata += inc; 
  rel->schema->keySize = *(int*)metadata;
  metadata += inc;
  
  // malloc schema fields
  if(DEBUG) printf("record_mgr.c - openTable: Mallocing fields for Schema\n");
  rel->schema->attrNames  = (char**) malloc(sizeof(char*) * rel->schema->numAttr);
  rel->schema->dataTypes  = (DataType*) malloc(sizeof(DataType) * rel->schema->numAttr);
  rel->schema->typeLength = (int*) malloc(sizeof(int) * rel->schema->numAttr);
  rel->schema->keyAttrs   = (int*) malloc(sizeof(int) * rel->schema->numAttr); 
  for(int i = 0; i < rel->schema->numAttr; i++){
    rel->schema->attrNames[i] = (char*) malloc(Attr_Size);
  }
  
  // Initializing schema fields
  if (DEBUG) printf("record_mgr.c - openTable: Initializing Schema Fields\n");
  for(int j = 0; j < rel->schema->numAttr; j++){
    strncpy(rel->schema->attrNames[j], metadata, Attr_Size);
    metadata += Attr_Size;
    rel->schema->dataTypes[j] = *(int*) metadata;
    metadata += inc;
    rel->schema->typeLength[j] = *(int*) metadata;
    metadata += inc;
    rel->schema->keyAttrs[j] = *(int*) metadata;
    metadata += inc;
  }
  // unpin the page from bufferpool
  if(DEBUG) printf("record_mgr.c - openTable: Done initializing schema from Metadata Info\n");
  unpinPage(rm->bm, rm->metadata);
  
  return RC_OK;
}
RC closeTable (RM_TableData *rel){
  int inc = sizeof(int);
  RM_Record_Mgr * rm = GET_RM_INFO(rel);
  pinPage(rm->bm, rm->metadata, 0);
  //char *start = rm->metadata->data;
  *(int*)rm->metadata->data = rm->numTup; //Number of Tuples
  rm->metadata->data += inc;
  *(int*)rm->metadata->data = rm->recordsPerPage;
  rm->metadata->data +=inc;
  *(int*)rm->metadata->data = rel->schema->numAttr; //stores the number of attributes in a table
  rm->metadata->data += inc;
  *(int*)rm->metadata->data = rel->schema->keySize; //Stores the size of the key.
  rm->metadata->data += inc;
  printf("record_mgr.c - closeTable: writing attrnames\n");
  for(int j = 0; j < rel->schema->numAttr; j++){
    printf("hello");
    printf("%s\n", rel->schema->attrNames[j]);
    strncpy(rm->metadata->data, rel->schema->attrNames[j], Attr_Size);
    rm->metadata->data += Attr_Size;
    *(int*)rm->metadata->data = rel->schema->dataTypes[j];
    rm->metadata->data += inc;
    *(int*)rm->metadata->data = rel->schema->typeLength[j];
    rm->metadata->data += inc;
    *(int*)rm->metadata->data = rel->schema->keyAttrs[j];
    rm->metadata->data += inc;
  }  
  markDirty(rm->bm, rm->metadata);
  unpinPage(rm->bm, rm->metadata);
  forcePage(rm->bm, rm->metadata);
  shutdownBufferPool(rm->bm);
  return RC_OK;
}
RC deleteTable (char *name){
  destroyPageFile(name);
  return RC_OK;
}
int getNumTuples (RM_TableData *rel){
  RM_Record_Mgr *rm = GET_RM_INFO(rel);
  return rm->numTup;
}

// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record){
  if(DEBUG) printf("Inserting Record\n");
  RM_Record_Mgr *rm = GET_RM_INFO(rel);
  int r_size = getRecordSize(rel->schema);
  int num;
  int recsPerPage = PAGE_SIZE / r_size;
  bool unSlotted = true;
  int pageNum = 1; 
  int slotNum = PAGE_DATA_FULL;
  printf("records per page is %d\n", recsPerPage);
  while(unSlotted){
    pinPage(rm->bm, rm->page, pageNum);
    //printf("segfault\n");
    char *page = rm->page->data;
    for(int i = 0; i < (recsPerPage); i++){
      //printf("stored in slot %d is %s\n", i, (char *)page[i * r_size]);
      if(page[i * r_size] != '$'){ 
        slotNum = i;
        unSlotted = false;
        break;
      }
    }
    // No Empty Slot was found so the page data is full and the next page should be checked
    if(slotNum == PAGE_DATA_FULL){
      printf("Page was full\n");
      //scanf("%d", &num);
      pageNum++;
      unpinPage(rm->bm, rm->page);
    }
  }
  //char *start = rm->page->data;
  rm->page->data  += slotNum * r_size;
  *rm->page->data = RECORD_START;
  rm->page->data += 1;
  memcpy(rm->page->data, record->data+1, r_size-1);
  rm->numTup++;
  //rm->page->data = start;
  record->id.page = rm->page->pageNum;
  record->id.slot = slotNum;
  markDirty(rm->bm, rm->page);
  unpinPage(rm->bm, rm->page);
  forcePage(rm->bm, rm->page);
  return RC_OK;    
}
RC deleteRecord (RM_TableData *rel, RID id){
  RM_Record_Mgr *rm = GET_RM_INFO(rel);
  int pageNum = id.page;
  pinPage(rm->bm, rm->page, pageNum);
  //char *start = rm->page->data;
  int slotNum = id.slot;
  int r_size = getRecordSize(rel->schema);
  rm->page->data += slotNum * r_size;
  *rm->page->data = RECORD_DELETE;
  rm->numTup -= 1;
  //rm->page->data = start;
  markDirty(rm->bm, rm->page); 
  unpinPage(rm->bm, rm->page);
  return RC_OK;
}
RC updateRecord (RM_TableData *rel, Record *record){
  RM_Record_Mgr *rm = GET_RM_INFO(rel);
  int pageNum = record->id.page;
  int slotNum = record->id.slot;
  int r_size  = getRecordSize(rel->schema);
  pinPage(rm->bm, rm->page, pageNum);
  //char *start = rm->page->data;
  rm->page->data += (slotNum * r_size) + 1;
  memcpy(rm->page->data, record->data, r_size);
  //rm->page->data = start;
  markDirty(rm->bm, rm->page);
  unpinPage(rm->bm, rm->page);
  return RC_OK;
}
RC getRecord (RM_TableData *rel, RID id, Record *record){
  RM_Record_Mgr *rm = GET_RM_INFO(rel);
  int pageNum = id.page;
  int slotNum = id.slot;
  int r_size  = getRecordSize(rel->schema);
  record->id = id;
  pinPage(rm->bm, rm->page, pageNum);
  //char *start = rm->page->data;
  rm->page->data += (slotNum * r_size);
  if ('$' == *rm->page->data){
    memcpy(record->data, rm->page->data, r_size);
    //rm->page->data = start;
    unpinPage(rm->bm, rm->page);
  } 
  else{
    return RC_RECORD_NOT_FOUND; 
  }
  
  return RC_OK;
}

// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
  RM_Record_Mgr *rm = GET_RM_INFO(rel);
  scan->cond = cond;
  //Set the first position to be the first slot of page one.
  scan->currentPage = 1;
  scan->currentSlot = 0;
  scan->rel = rel;
  return RC_OK;
}

RC getNextRecord(RM_ScanHandle *scan, Record *rcd){
  RM_Record_Mgr *rm = GET_RM_INFO(scan->rel);
  RC status;
  RID *nextRec = (RID *)malloc(sizeof(RID));
  if(scan->scanned == rm->numTup) return RC_RM_NO_MORE_TUPLES;
  while(true){ // This could be an infinite loop
    if(scan->currentSlot > (rm->recordsPerPage - 1)){
      scan->currentPage += 1;
      scan->currentSlot = 0; 
    }
    nextRec->page = scan->currentPage;
    nextRec->slot = scan->currentSlot;
    status = getRecord(scan->rel, *nextRec, rcd);
    if(status == RC_OK){
      scan->scanned += 1;
      scan->currentSlot += 1;
      free(nextRec);
      return status;
    }
  }
}

RC next (RM_ScanHandle *scan, Record *record){
  RM_Record_Mgr *rm = GET_RM_INFO(scan->rel);
  RC status;
  if (rm->numTup == 0) return RC_RM_NO_MORE_TUPLES;
  else if (scan->cond == NULL){ //return all of the records.
    status = getNextRecord(scan, record);
    return status;
  }
  else{
    //int r_size = getRecordSize(scan->rel->schema);
    Value *val = (Value *)malloc(sizeof(Value));
    RID *rec   = (RID *)malloc(sizeof(RID));
    while(scan->scanned <= rm->numTup){
      if(scan->currentSlot > (rm->recordsPerPage - 1)){
        scan->currentPage += 1;
        scan->currentSlot = 0;
      }
      rec->page  = scan->currentPage;
      rec->slot  = scan->currentSlot;
      status     = getRecord(scan->rel, *rec, record);
      if(status == RC_RECORD_NOT_FOUND){
        scan->currentSlot += 1;
        continue;
      }
      else{
        scan->scanned += 1;
        scan->currentSlot += 1;
        evalExpr(record, scan->rel->schema, scan->cond, &val);
        if(val->v.boolV == true){
          free(val);
          free(rec); 
          return RC_OK;
        }
      }
    }
    free(val); 
    free(rec);  
    return RC_RM_NO_MORE_TUPLES;
  }
}
RC closeScan (RM_ScanHandle *scan){
  RM_Record_Mgr *rm = GET_RM_INFO(scan->rel);
  scan->currentPage = -1;
  scan->currentSlot = -1;
  scan->cond = NULL;
  return RC_OK;
}

// dealing with schemas
int getRecordSize (Schema *schema){
  int r_size = 1; //at the beginning of every inserted record there will be a $ to indicate that the slot is taken. Random otherwise
  for(int i = 0; i < schema->numAttr; i++){
    if(schema->dataTypes[i] == DT_STRING) r_size += schema->typeLength[i];
    else if(schema->dataTypes[i] == DT_INT) r_size += sizeof(int);
    else if(schema->dataTypes[i] == DT_FLOAT) r_size += sizeof(float);
    else r_size += sizeof(bool); 
  }
  return r_size;
}
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
  Schema *schema = (Schema *) malloc(sizeof(Schema));
  schema->numAttr = numAttr;
  schema->attrNames = attrNames;
  schema->dataTypes = dataTypes;
  schema->typeLength = typeLength;
  schema->keySize = keySize;
  schema->keyAttrs = keys;
  return schema;
}
RC freeSchema (Schema *schema){
  free(schema);
  return RC_OK;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema){
  Record *rcd = (Record *) malloc(sizeof(Record));
  int r_size = getRecordSize(schema);
  rcd->data = (char *) malloc(r_size);
  rcd->id.page = UNSLOTTED;
  rcd->id.slot = UNSLOTTED;
  *record = rcd;
  return RC_OK;
}
RC freeRecord (Record *record){
  //free(record->id);
  free(record->data);
  free(record);
  return RC_OK;
}
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
  char *start = record->data;
  int pos = 1;
  for(int i = 0; i < attrNum; i++){
    if(schema->dataTypes[i] == DT_STRING)     pos += schema->typeLength[i];
    else if(schema->dataTypes[i] == DT_INT)   pos += sizeof(int);
    else if(schema->dataTypes[i] == DT_FLOAT) pos += sizeof(float);
    else pos += sizeof(bool);
  }
  Value *val = (Value *)malloc(sizeof(Value));
  record->data += pos;
  if(schema->dataTypes[attrNum] == DT_STRING){
    val->dt = DT_STRING;
    int size = schema->typeLength[attrNum] + 1;
    val->v.stringV = (char *)malloc(size);
    strncpy(val->v.stringV, record->data, size);
    val->v.stringV[size] = '\0'; 
  }else if(schema->dataTypes[attrNum] == DT_INT){
    val->dt = DT_INT;
    memcpy(&val->v.intV, record->data, sizeof(int));
  }else if(schema->dataTypes[attrNum] == DT_FLOAT){
    val->dt = DT_FLOAT;
    memcpy(&val->v.floatV, record->data, sizeof(float));
  }else{
    val->dt = DT_BOOL;
    memcpy(&val->v.boolV, record->data, sizeof(bool));  
  }
  *value = val;
  return RC_OK;  
}
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
  char *start = record->data;
  int pos = 1;
  // move the data pointer to the right location
  for(int i = 0; i < attrNum; i++){
    if(schema->dataTypes[i] == DT_STRING)     pos += schema->typeLength[i];
    else if(schema->dataTypes[i] == DT_INT)   pos += sizeof(int);
    else if(schema->dataTypes[i] == DT_FLOAT) pos += sizeof(float);
    else pos += sizeof(bool);
  }
  // set the attribute 
  record->data += pos;
  if(schema->dataTypes[attrNum] == DT_STRING){
    int size = schema->typeLength[attrNum];
    strncpy(record->data, value->v.stringV, size);
  }
  else if(schema->dataTypes[attrNum] == DT_INT) *(int*)record->data   = value->v.intV;
  else if(schema->dataTypes[attrNum] == DT_FLOAT)     *(float*)record->data = value->v.floatV;
  else *(bool*)record->data = value->v.boolV;
  record->data = start;
  return RC_OK;  
}
