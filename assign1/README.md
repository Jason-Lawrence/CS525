# cs525
IIT CS525 Homework 1

##Testing the Storage Manager

In addition to the given Test cases we have written our own test case called testMultiPageContent.
This function resides in test_assign1_1.c file. Below is a brief description of what it does.
First it creates 2 different test files and opens them and then verifies that this was done correctly.
After both files are created and verified it begins by calling ensureCapacity() on the first file
and then begins writing the different blocks. After each block is written it reads that block and verifys what was just written. Once the first file has been written and verified it then writes the second file
to match the first. After it writes one block it calls appendEmptyBlock() to increase the total number of pages
in the file. It continues to do this until all blocks are written. After it finishes writing the second file 
it verifys that the contents of the second file. It utilizes the different read and write functions to 
accomplish this. 

## Documentation

### void initStorageManager (void);

initializes storage manager

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|void|||

#### Return

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|void|||

### RC createPageFile (char *fileName);

Creates a new page file called fileName of size PAGE_SIZE.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|char *   |fileName|The name of the new file to create|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### int findFileSize(void *fp);

Finds the size of the opened file.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|void *   |fp      |The file pointer or file descriptor to the open file|

#### Return

|Data Type|Description|
|:--------|:----------|
|int      |The size of the file in Bytes|

### RC openPageFile (char *fileName, SM_FileHandle *fHandle);

Opens an existing page file. Should return RC_FILE_NOT_FOUND if the file does not exist. The second parameter is an existing file handle. If opening the file is successful, then the fields of this file handle should be initialized with the information about the opened file. For instance, you would have to read the total number of pages that are stored in the file from disk.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|char *   |fileName|The name of the file to open|
|SM_FileHandle * |fHandle|A struct with file information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC closePageFile (SM_FileHandle *fHandle);

Closes the file. Should return RC_FILE_NOT_FOUND if the file is not found. Else returns RC_OK

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC destroyPageFile (char *fileName);

Deletes the file. Should return RC_FILE_NOT_FOUND if the file is not found. Else returns RC_OK.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|char *   |fileName|The name of the file to open|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

Reads a block from the provided page file.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|int      |pageNum |The page number that is trying to be read|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### int getBlockPos (SM_FileHandle *fHandle);

Gets the block number of the current block. It does not get the position in the file of the block

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|

#### Return

|Data Type|Description|
|:--------|:----------|
|int      |The block number where the reader for the given file is positioned|

### int moveBlockPos (int blockNum, SM_FileHandle *fHandle);

Sets the current position in the file to the specified block number.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|int      |blockNum|The block number the current position is expected to move to.|
|SM_FileHandle * |fHandle|A struct with file information|

#### Return

|Data Type|Description|
|:--------|:----------|
|int      |Returns 0  |

### RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

Reads the first block in the page file

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

Reads the block before the current block

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

Reads the current block

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

Reads the next block based off of current file position.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

Reads the last block in the page file

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

Writes data to the provided page file in the given file.

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|int      |pageNum |The page number that needs to be written to the file|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

Write the given data to the current block

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|
|SM_PageHandle   |memPage|A struct containing the page information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC appendEmptyBlock (SM_FileHandle *fHandle);

Appends an empty block to the given file

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|SM_FileHandle * |fHandle|A struct with file information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

### RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle);

Increases the file to the number of pages provided if not already given. It does not shrink the file

#### Parameters

|Data Type|Variable|Description|
|:--------|:-------|:----------|
|int      |numberOfPages|The number of pages the file should grow to|
|SM_FileHandle * |fHandle|A struct with file information|

#### Return

|Data Type|Description|
|:--------|:----------|
|RC       |Return code|

## Contributors

Alec Buchanan

- Documentation part of read me
- readCurrentBlock()
- readPreviousBlock()
- readFirstBlock()
- moveBlockPos()
- getBlockPos()
- readBlock()
- openPageFile()
- findFileSize()
- createPageFile()
- initStorageManager()

Jason Lawrence
- closePageFile()
- destroyPageFile()
- readNextBlock()
- writeBlock()
- writeCurrentBlock()
- appendEmptyBlock()
- ensureCapacity()
- Added lots of test cases
