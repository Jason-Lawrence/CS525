#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h"
// Include bool DT
#include "dt.h"
#include "storage_mgr.h"
#include <stdlib.h>
#include <string.h>

// Replacement Strategies
typedef enum ReplacementStrategy {
        RS_FIFO = 0,
        RS_LRU = 1,
        RS_CLOCK = 2,
        RS_LFU = 3,
        RS_LRU_K = 4
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber;
#define NO_PAGE -1

typedef struct BM_BufferPool {
        const char *pageFile;
	SM_FileHandle * fh;
        int numPages;
        ReplacementStrategy strategy;
	int   counterReadIO;
        int   counterWriteIO;
        int   counter;
	PageNumber *pageFrameContents;
	bool *dirtyFlags;
	int *fixedPageCounts;
        void *mgmtData; // use this one to store the bookkeeping info your buffer
        // manager needs for a buffer pool
} BM_BufferPool;

typedef struct BM_PageHandle {
        PageNumber pageNum;
        char * data;
        //bool dirty;
        //short pins;
        int pageID; // Used to implement the replacement strategies.
} BM_PageHandle;

typedef struct BM_BufferPool_Page{
	int pageID;
	short pins;
	bool dirty;
	SM_PageHandle data;
	PageNumber pageNum;
	
}BM_BufferPool_Page;

// convenience macros
#define MAKE_POOL()                                     \
                ((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))

#define MAKE_PAGE_HANDLE()                              \
                ((BM_PageHandle *) malloc (sizeof(BM_PageHandle)))

// I was type casting this to (BM_BufferPool *) but it threw a bunch of warnings
#define GET_POOL_INFO(buffPool) ((BM_BufferPool_Page *)buffPool->mgmtData)

#define DUMP_POOL_INFO(bpi) printf("TEST  BUFFER POOL INFO DUMP\n\tfb\n\t\tfileName = %s\n\t\ttotalNumPages = %d\n\t\tcurPagePos = %d\n\t\tmgmtInfo = %p\n\tdirtyPages = %p\n\tpagePinCount = %p\n\tpageHandles = %p\n\tcounterReadIO = %d\n\tcounterWriteIO = %d\n\n",bpi->fh->fileName, bpi->fh->totalNumPages, bpi->fh->curPagePos, (void *)bpi->fh->mgmtInfo, (void *)bpi->dirtyPages, (void *)bpi->pagePinCount, (void *)bpi->pageHandles, bpi->counterReadIO, bpi->counterWriteIO)

#define PRINT_AND_RETURN(text, retCode) do{ \
        printf(text);\
        return retCode;\
        }while(0);

#define DEBUG 1

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                const int numPages, ReplacementStrategy strategy,
                void *stratData);
RC shutdownBufferPool(BM_BufferPool *const bm);
RC forceFlushPool(BM_BufferPool *const bm);

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page);
RC cleanPage (BM_BufferPool *const bm, int dirtyPage);
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
                const PageNumber pageNum);

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm);
bool *getDirtyFlags (BM_BufferPool *const bm);
int *getFixCounts (BM_BufferPool *const bm);
int getNumReadIO (BM_BufferPool *const bm);
int getNumWriteIO (BM_BufferPool *const bm);
int getPageIndex(BM_BufferPool *const bm, int pageNum);
#endif
