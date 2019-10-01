# cs525
IIT CS525 Homework 2

##Documentation

## Modifications/New Structs Implemented

### BM_BufferPool_Info Struct

Used to store info that the Buffer Pool needs about the BM_PageHandles.

#### Paramenters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|SM_FileHandle *     |fh             |Stores a pointer to the file handle being loaded into the Buffer Pool|
|PageNumber *        |pageFrameContents| A pointer to an array that stores all of the current page numbers in all of the Page Frames.|
|bool *              |dirtyFlags     |A pointer to an array that stores which pages are currently dirty in the Page Frames|
|int *               |fixedPageCounts|A pointer to an array that stores the pin count for each Page Frame|
|BM_PageHandles **   |bmPages        |A Pointer to an array of pointers where ech pointer points the location of a page handle currently being stored in the Buffer Pool|
|int                 |counterReadIO  |A counter used to keep track of how many readIO requests are given since the Buffer Pool was initialiized|
|int                 |counterWriteIO |A counter used to keep track of how many WriteIO requests are given since the Buffer Pool was initialiized|
|int                 |counter        |A counter used to assign pageID to the pageHandles stored in the Buffer Pool. This ID is uesd to implement the Replacement Strategies|

### BM_PageHandle

Used to store Information about the Page Handles.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|PageNumber          |pageNum      |Stores the Page Number that was read form the file|
|char *              |data         |Stores the data read from the Page File|
|bool                |dirty        |Flag for whether a page has been modified|
|short               |pins         |Stores how many processes have a page pinned|
|int                 |pageID       |Stores the pageID of the page handle. Used to determine which page to Evict|

## Replacement Strategy Implementation

### FIFO

To implement this strategy whenever a page was pinned it was given a pageID via the counter variable in BM_BufferPool_Info struct. After assignment we would increment the counter. 
To determine which Page to evict from the Buffer Pool The page with the lowest pageID and pin count of 0 was chosen. This is because the older Pages would have lower pageIDs and
newer Pages would have lareger pageIDs.

### LRU

Similar to FIFO pages were given PageID's. The difference is that whenever the page was used we would assign it a new pageID via the counter variable and then increment the counter.
This menas that the most recently used Page would have higher pageIDs and the least recently used Page would have a lower pageID. 

This implementation made it easy to find which page to evict regardless of which strategy was used because it looks for the page with the lowes pageID and a pin count of 0.

## Functions

### RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData);

initializes the buffer pool by allocating all of the memory and initializes the data fields.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |
|const char *const   | pageFileName|Name of the page file to be opened       |
|const int           |numPages     |Number of Page Frames the Buffer Pool has|
|ReplacementStrategy |strategy     |Replacement strategy to be used          |
|void *              |stratData    |                                         |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC shutdownBufferPool(BM_BufferPool *const bm);

Shuts down the Buffer Pool. It frees all memory and writes any dirty pages back to the disk. Returns an error if there are any pinned pages in the Buffer Pool

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC forceFlushPool(BM_BufferPool *const bm);

This function cleans all of the dirty pages in the Buffer Pool by writing them back to the disk.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page);

This function is what the user calls to notify the Buffer Pool that a page is dirty. 

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |
|BM_PageHandle *const|page         |BufferPool Page descriptor               |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC cleanPage(BM_BufferPool *const bm, int dirtyPage);

This function writes the dirty pages back to the disk.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |
|int                 |dirtyPage    |The page number of the dirty page        |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page);

This function unpins a page when the client is done with it.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |
|BM_PageHandle *const|page         |BufferPool Page descriptor               |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);

This function cleans a page marked dirty by writing it back to the disk.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |
|BM_PageHandle *const|page         |BufferPool Page descriptor               |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum);

This function pins the requested page to the Buffer Pool if it exists. 
If the page is already pinned it updates the fixed count and then returns. 
Else if there is a free Page Frame then it will pin it to that. 
Else it will evict a page from the Page Frame using the specified Replacement Strategy.

#### Parameters

|Data Type           |Variable     |Description                              |
|:-------------------|:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |
|BM_PageHandle *const|page         |BufferPool Page descriptor               |
|const PageNumber    |pageNum      |The page number of the page to pin       |

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return Code|

### PageNumber *getFrameContents (BM_BufferPool *const bm);

This function generates an array of PageNumbers where the ith element is the page number of the ith Page Frame. 

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type |Description|
|:---------|:----------|
|PageNumber|The page number of the page stored in the Page Frame|

### bool *getDirtyFlags (BM_BufferPool *const bm);

This function generates an array of booleans where the ith element is True if the page stored in the ith Page Frame is dirty.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type |Description|
|:---------|:----------|
|bool      |A boolean value based on whether the page is dirty or not.

### int getFixCounts(BM_BufferPool *const bm);

This function generates an array where the ith element is the pin count on the page stored on the ith Page Frame.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type |Description|
|:---------|:----------|
|int       |The amount a page is pinned|

### int getNumReadIO (BM_BufferPool *const bm)

Returns the amount of reads from the disk since the Buffer Pool was initialized.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type |Description|
|:---------|:----------|
|int       |The amount of Reads since initialization|

### int getNumWriteIO (BM_BufferPool *const bm)

Returns the amount of writes to the disk since the Buffer Pool was Initialized.

#### Parameters

|Data Type           |Variable     |Description                              |
|:------------------ |:------------|:----------------------------------------|
|BM_BufferPool *const|     bm      |BufferPool descriptor                    |

#### Return

|Data Type |Description|
|:---------|:----------|
|int       |The amount of Writes since initialization|

## Contributors

### Alec Buchanan

- initBufferPool()
- markDirty()
- forcePage()
- forceFlushPool()
- pinPage()
- unpinPage()
- shutdownBufferPool()
- Error handling and Debugging
- Makefile

### Jason Lawrence

- Documentation 
- getFixedCounts()
- getDirtyFlags()
- getFrameContents()
- getNumReadIO()
- getNumWriteIO()
- FIFO Replacement Strategy
- LRU Replacement Stratgy