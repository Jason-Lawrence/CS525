#include "buffer_mgr.h"

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy,void *stratData){
  if (DEBUG) printf("INFO  buffer_mgr.c - initBufferPool: Configuring buffer pool\n");
  bm->pageFile = pageFileName;
  bm->fh = malloc(sizeof(SM_FileHandle));
  bm->numPages = numPages;
  bm->strategy = strategy;
  bm->pageFrameContents = malloc(sizeof(PageNumber) * numPages);
  bm->dirtyFlags        = malloc(sizeof(bool) * numPages);
  bm->fixedPageCounts   = malloc(sizeof(int) * numPages);
  bm->counter           = 0;
  bm->counterReadIO     = 0;
  bm->counterWriteIO    = 0;

  if (DEBUG) printf("INFO  buffer_mgr.c - initBufferPool: Allocating buffer pool meta data\n");
  BM_BufferPool_Page *BM_Pages = malloc(sizeof(BM_BufferPool_Page) * numPages);
  for(int i = 0; i < numPages; i++){
    BM_Pages[i].data    = NULL; //malloc(sizeof(PAGE_SIZE));
    BM_Pages[i].dirty   = 0;
    BM_Pages[i].pins    = 0;
    BM_Pages[i].pageID  = -1;
    BM_Pages[i].pageNum = -1;
  }
  bm->mgmtData = BM_Pages;
  if (DEBUG) printf("INFO  buffer_mgr.c - initBufferPool: Opening page file\n");
  RC status = openPageFile(pageFileName, bm->fh);
  if (status != RC_OK) return status;
  if (DEBUG) printf("INFO  buffer_mgr.c - initBufferPool: Done initializing buffer pool data\n");
  return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm){
  if (DEBUG) printf("INFO  buffer_mgr.c - shutdownBufferPool: Shutting down buffer pool\n");
  RC status = RC_OK;
  BM_BufferPool_Page *BM_Pages = (BM_BufferPool_Page *)bm->mgmtData;
  if (DEBUG) printf("INFO  buffer_mgr.c - shutdownBufferPool: Checking for dirty pages\n");
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum != -1){
      if (BM_Pages[p].dirty){
        if (DEBUG) printf("INFO  buffer_mgr.c - shutdownBufferPool: Cleaning page %d\n", BM_Pages[p].pageNum);
        status = cleanPage(bm, BM_Pages[p].pageNum);
        if (status != RC_OK) PRINT_AND_RETURN("ERROR buffer_mgr.c - shutdownBufferPool: Failed to clean page\n", status);
      }
    } 
  }
  free(BM_Pages);
  if (DEBUG) printf("INFO  buffer_mgr.c - shutdownBufferPool: Closing page file\n");
  status = closePageFile(bm->fh);
  if (status != RC_OK) PRINT_AND_RETURN("ERROR buffer_mgr.c - shutdownBufferPool: Failed to close page file\n", status);

  free(bm->fh);
  free(bm->pageFrameContents);
  free(bm->dirtyFlags);
  free(bm->fixedPageCounts);
  return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm){
  if (DEBUG) printf("INFO  buffer_mgr.c - forceFlushPool: Forcing data to be flushed\n");
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum != -1){
      if (BM_Pages[p].dirty && BM_Pages[p].pins == 0){
        if (DEBUG) printf("INFO  buffer_mgr.c - forceFlushPool: Cleaning page %d\n", BM_Pages[p].pageNum);
        RC status = cleanPage(bm, BM_Pages[p].pageNum);
        if (status != RC_OK) return status;
      }
    }
  }
}
// Buffer Manager Interface Access Pages


RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
  if (DEBUG) printf("INFO  buffer_mgr.c - markDirty: Marked page %d as dirty\n", page->pageNum);
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum == page->pageNum){
      BM_Pages[p].dirty = 1;
      /*
      if (bm->strategy == RS_LRU){
        page->pageID = bm->counter;
	BM_Pages[p].pageID = bm->counter;
        bm->counter++;
      }
      */
      return RC_OK;
    }
  }
  return RC_IM_NO_MORE_ENTRIES;
}


int getPageIndex(BM_BufferPool *const bm, int pageNum){
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum != -1){
      if(BM_Pages[p].pageNum == pageNum) return p;
    }
  }
  return -1;
}


RC cleanPage (BM_BufferPool *const bm, int dirtyPageNum){
  if (DEBUG) printf("INFO  buffer_mgr.c - cleanPage: Writing data\n");
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  RC status = RC_OK;
  if (DEBUG) printf("INFO buffer_mgr.c - getting index of the dirty page\n");
  int index = getPageIndex(bm, dirtyPageNum);

  if (index < 0) PRINT_AND_RETURN("ERROR buffer_mgr.c - cleanPage: Could not find index\n", RC_IM_KEY_NOT_FOUND);
  if (BM_Pages[index].dirty == 0) printf("NOTE  buffer_mgr.c - cleanPage: Page is being cleaned but not dirty\n");
  status = writeBlock(dirtyPageNum, bm->fh, BM_Pages[index].data);
  if (status != RC_OK) PRINT_AND_RETURN("ERROR buffer_mgr.c - cleanPage: Could not write data\n", status);

  bm->counterWriteIO++;
  BM_Pages[index].dirty = 0;

  // TODO: update eviction policy

  return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
  if (DEBUG) printf("INFO  buffer_mgr.c - unpinPage: Unpinning page %d\n", page->pageNum);
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  //int pageNum = page->pageNum;
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum == page->pageNum) {
      BM_Pages[p].pins--;
      printf("Pin count is %d\n", BM_Pages[p].pins);
      //if (BM_Pages[p].pins < 0) BM_Pages[p].pins = 0;
      //printf("Pin count is %d\n", BM_Pages[p].pins);
      return RC_OK;
    }
  }
  return RC_IM_NO_MORE_ENTRIES;
}



RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
  if (DEBUG) printf("INFO  buffer_mgr.c - forcePage: Forcing page %d\n", page->pageNum);
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  int pageNum = page->pageNum;
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum == pageNum) return cleanPage(bm, page->pageNum);
  }
  return RC_IM_NO_MORE_ENTRIES;
}




RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
  if (DEBUG) printf("INFO  buffer_mgr.c - pinPage: Initializing page handle %d\n", pageNum);
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  //page->pageNum    = pageNum;
  int lastEmptyPage = -1;
  if (DEBUG) printf("INFO  buffer_mgr.c - pinPage: Checking if page is already pinned\n");
  for (int p = 0; p < bm->numPages; p++){
    if (BM_Pages[p].pageNum == -1){
      lastEmptyPage = p; // Empty Page Frame found
      break;
    }else if (BM_Pages[p].pageNum == pageNum){
      BM_Pages[p].pins++;
      page->pageNum = pageNum;
      page->data    = BM_Pages[p].data;
      //Update Replacement Policy for LRU
      if (bm->strategy == RS_LRU){
	printf("Updating LRU policy\n");
	bm->counter++;
        BM_Pages[p].pageID = bm->counter;
	printf("Counter is %d\n", bm->counter);
      }
      if (DEBUG) printf("INFO  buffer_mgr.c - pinPage: Found page\n");
      return RC_OK;
    }
  }

  if (lastEmptyPage == -1){ // No Empty Page Frames were found
    if (DEBUG) printf("INFO  buffer_mgr.c - pinPage: Page not found, eviction needed\n");
    // TODO: implement eviction
    // TODO: set lastEmptyPage to the page that was evicted
    int minID;
    for(int i = 0; i < bm->numPages; i++){
      if(BM_Pages[i].pins == 0)
        minID = BM_Pages[i].pageID;
        lastEmptyPage = i;
    }
    for(int j = 0; j < bm->numPages; j++){
      if(BM_Pages[j].pins == 0 && BM_Pages[j].pageID < minID){
        minID = BM_Pages[j].pageID;
        lastEmptyPage = j;
      }
    }
  }
  if (lastEmptyPage == -1){
    if (DEBUG) printf("ERROR buffer_mgr.c - pinPage: No free pages to Evict\n");
    return RC_IM_N_TO_LAGE;
  }
  if(DEBUG) printf("INFO buffer_mgr.c - pinPage: Checking if page stored in Page Frame %d is dirty.\n", lastEmptyPage);
  
  if(BM_Pages[lastEmptyPage].dirty == 1){
    if (DEBUG) printf("INFO buffer_mgr.c - pinPage: Cleaning dirty page before eviction.\n");
    cleanPage(bm, BM_Pages[lastEmptyPage].pageNum);
    if (DEBUG) printf("INFO buffer_mgr.c - pinPage: Page now ready to evict.\n");
  }
  BM_Pages[lastEmptyPage].data = (SM_PageHandle) malloc(PAGE_SIZE);
  if (DEBUG) printf("INFO  buffer_mgr.c - pinPage: Adding page %d to position %d\n", pageNum, lastEmptyPage);
  RC status = readBlock(pageNum, bm->fh, BM_Pages[lastEmptyPage].data);
  //printf("read block");
  if (status == RC_READ_NON_EXISTING_PAGE) printf("Error");//memset(bufferInfo->bmPages[lastEmptyPage]->data, 0, PAGE_SIZE);
  else if (status != RC_OK) return status;
  bm->counter++;
  page->pageNum = pageNum;
  page->data    = BM_Pages[lastEmptyPage].data;
  BM_Pages[lastEmptyPage].pageNum = pageNum;
  BM_Pages[lastEmptyPage].pins    = 1;
  BM_Pages[lastEmptyPage].pageID  = bm->counter;
  //update ReadIO and eviction counter.
  bm->counterReadIO++;
  printf("counter is %d\n", bm->counter); 

  if (DEBUG) printf("INFO  buffer_mgr.c - pinPage: Done pinning page\n");
  return RC_OK;
}
// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  for(int p = 0; p < bm->numPages; p++){
   bm->pageFrameContents[p] = BM_Pages[p].pageNum;
  }
  return bm->pageFrameContents;
}
bool *getDirtyFlags (BM_BufferPool *const bm){
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  for(int p = 0; p < bm->numPages; p++){
    bm->dirtyFlags[p] = BM_Pages[p].dirty;
  }
  return bm->dirtyFlags;
}
int *getFixCounts (BM_BufferPool *const bm){
  BM_BufferPool_Page *BM_Pages = GET_POOL_INFO(bm);
  for(int p = 0; p < bm->numPages; p++){
    bm->fixedPageCounts[p] = BM_Pages[p].pins;
  }
  return bm->fixedPageCounts;
}
int getNumReadIO (BM_BufferPool *const bm){
  //BM_BufferPool_Info *bufferInfo = GET_POOL_INFO(bm);
  return bm->counterReadIO;
}
int getNumWriteIO (BM_BufferPool *const bm){
  //BM_BufferPool_Info *bufferInfo = GET_POOL_INFO(bm);
  return bm->counterWriteIO;
}
