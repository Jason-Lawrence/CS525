#include <stdio.h>
#include "storage_mgr.h"

#define DEBUG 1

int findFileSize(void *fp);

void initStorageManager (void){
  // TODO: figure out what to do here 
  printf("INFO storage_mgr.c - initStorageManager: TODO do something with this func\n");
}

/*  createPageFile
 *    Create a new page file fileName. The initial file size should be one page. This method should fill this single page with '\0' bytes.
 * 
*/
RC createPageFile (char *fileName){
  // Create File
  if (DEBUG) printf("INFO storage_mgr.c - createPageFile:\tcreating page file\n");
  FILE *fp = fopen (fileName, "wb"); // w: creates empty file or overwrites contents
  if (fp == NULL){
    // Error Handling
    printf("ERROR storage_mgr.c - createPageFile:\tCould not create file %s\n", fileName);
    return RC_WRITE_FAILED;
  }  

  // Create Buffer
  char buf[PAGE_SIZE];
  for (int i = 0; i <  PAGE_SIZE; i++){
    buf[i] = '\0';
  }

  // Write to file
  if (DEBUG) printf("INFO storage_mgr.c - createPageFile:\twriting page file\n");
  size_t written = fwrite(buf,sizeof(char),PAGE_SIZE,fp);
  if (written != PAGE_SIZE){
    // Error Handling
    printf("ERROR storage_mgr.c - createPageFile:\tCould not write to file %s\n", fileName);
    return RC_WRITE_FAILED;
  }
  
  fclose(fp); // Return 0 on success
  
  return RC_OK; 
}



int findFileSize(void *fp){
  int startPos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, startPos, SEEK_SET);
  return size;
}
/*  openPageFile
 *    Opens an existing page file. Should return RC_FILE_NOT_FOUND if the file does not exist. The second parameter is an 
 *    existing file handle. If opening the file is successful, then the fields of this file handle should be initialized with the 
 *    information about the opened file. For instance, you would have to read the total number of pages that are stored in the 
 *    file from disk.
*/    
RC openPageFile (const char *fileName, SM_FileHandle *fHandle){
  // open file
  if (DEBUG) printf("INFO storage_mgr.c - openPageFile:\topening page file\n");
  FILE *fp = fopen (fileName, "rb+"); // opens for reading and writing
  if (fp == NULL){
    // Error Handling
    printf("ERROR storage_mgr.c - openPageFile:\tCould not open file %s\n", fileName);
    return RC_FILE_NOT_FOUND;
  }

  // Populate SM_FileHandle
  int fileSize = findFileSize(fp);
  int numPages = fileSize/PAGE_SIZE; 

  fHandle->fileName      = fileName;
  fHandle->totalNumPages = numPages;
  fHandle->curPagePos    = ftell(fp);
  fHandle->mgmtInfo      = fp;

  if (DEBUG) printf("INFO storage_mgr.c - openPageFile:\tOpened file with parameters\n\tfileName: %s\n\ttotalNumPages: %d\n\tcurPagePos: %d\n", fHandle->fileName, fHandle->totalNumPages, fHandle->curPagePos);

  return RC_OK;
}

/* closePageFile
Closes the file. Should return RC_FILE_NOT_FOUND if the file is not found. Else returns RC_OK
*/
RC closePageFile (SM_FileHandle *fHandle){
        if (DEBUG) printf("INFO storage_mgr.c - closePageFile: Closing File\n");
        fclose(fHandle->mgmtInfo);
        return RC_OK;
}

/* destroyPageFile
Deletes the file. Should return RC_FILE_NOT_FOUND if the file is not found. Else returns RC_OK.
*/
RC destroyPageFile (char *fileName){
        if (DEBUG) printf("INFO storage_mgr.c - destroyPageFile: Deleting File\n");
        int status = remove(fileName);
        if (status == -1){
                printf("ERROR storage_mgr.c - destroyPageFile: Could not find file %s\n", fileName);
                return RC_FILE_NOT_FOUND;
        }

        return RC_OK;
}

/* reading blocks from disc */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
  // Check if block is in file
  FILE *fp     = fHandle->mgmtInfo;
  int offset   = pageNum * PAGE_SIZE;
  /*if (fHandle->totalNumPages < pageNum){
    // Check if > is correct or if we need >=
    printf("ERROR storage_mgr.c - readBlock:\tAttempt to read block out of range in  %s\n", fHandle->fileName);
    return RC_READ_NON_EXISTING_PAGE;
  }*/

  // Read block from file
  int status   = fseek(fp, offset, SEEK_SET);                   // Set read pos to correct pos
  size_t rSize = fread(memPage, sizeof(char), PAGE_SIZE, fp);
  fHandle->curPagePos = ftell(fp);
  if (rSize != PAGE_SIZE){
    printf("ERROR storage_mgr.c - readBlock:\tFailed to read block from %s\n", fHandle->fileName);
    return RC_READ_NON_EXISTING_PAGE;
  }

  return RC_OK;

}
int getBlockPos (SM_FileHandle *fHandle){
	if (DEBUG) printf("INFO storage_mgr.c - getBlockPos: Scanning Block\n");
	FILE *fp = fHandle->mgmtInfo;
	int currentPos = ftell(fp);
	int Pos = currentPos/PAGE_SIZE;
	if (DEBUG) printf("INFO storage_mgr.c - getBlockPos: The current block position is, %x\n", Pos);
	return Pos;
}

int moveBlockPos (int blockNum, SM_FileHandle *fHandle){
	FILE *fp = fHandle->mgmtInfo;
	if (blockNum > fHandle->totalNumPages){
		printf("ERROR storage_mgr.c - moveBlockPos: Block does not exist\n");
		return -1;
	}
	int currentPos = ftell(fp);
	if (blockNum == currentPos/PAGE_SIZE){
		return 0;
	}
	fseek(fp, blockNum*PAGE_SIZE, SEEK_SET);
	fHandle->curPagePos = ftell(fp);
	return 0;
}
	
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if (DEBUG) printf("INFO storage_mgr.c - readFirstBlock: Reading the first block\n");
	return readBlock(0, fHandle, memPage);
}
//TODO
// made simpler by 
// getBlockPos()
// moveBlockPos()
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  FILE *fp   = fHandle->mgmtInfo;
  int curPos = ftell(fp);
  int curBlock = curPos / PAGE_SIZE;
  int prevBlock = curBlock - 1;
  if (prevBlock < 0){
    printf("ERROR storage_mgr.c - readPreviousBlock:\tPrevious Block does not exist\n");
    return RC_READ_NON_EXISTING_PAGE;
  }
  return readBlock(prevBlock, fHandle, memPage);
}
//TODO
// use getBlockPos()
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  FILE *fp   = fHandle->mgmtInfo;
  int curPos = ftell(fp);
  int curBlock = curPos / PAGE_SIZE;
  return readBlock(curBlock, fHandle, memPage);
}
//TODO
// use getBlockPos()
// use movBlockPos()
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	FILE *fp = fHandle->mgmtInfo;
	int fsize = findFileSize(fp);
	int currentPos = ftell(fp);
	int block = (currentPos/PAGE_SIZE) + 1;
	if ((block + PAGE_SIZE) == fsize){
		printf("ERROR storage_mgr.c - readNextBlock:\tNext Block does not exist\n");
		return RC_READ_NON_EXISTING_PAGE;
	}
	return readBlock(block, fHandle, memPage);
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
  FILE *fp  = fHandle->mgmtInfo;
  int fsize = findFileSize(fp);
  int block = (fsize/PAGE_SIZE) - 1; //causes an out of bounds error without the minus 1.
  return readBlock(block, fHandle, memPage);
}

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
// Initial setup
        FILE *fp = fHandle->mgmtInfo;
        int currentPos = ftell(fp);
	int offset = pageNum*PAGE_SIZE;
	int fsize = findFileSize(fp);
	if (offset >= fsize){
		ensureCapacity(pageNum, fHandle);
		//printf("ERROR storage_mgr.c - writeBlock:\tInvalid Page Number.");
		//return RC_WRITE_FAILED;
	}
//  Begin Overwrite.
	if (DEBUG) printf("INFO storage_mgr.c - writeBlock: Writing Block, %d\n", pageNum);
        fseek(fp, offset, SEEK_SET);
        size_t written = fwrite(memPage, sizeof(char), PAGE_SIZE, fp);
	if (written != PAGE_SIZE){
                 printf("ERROR storage_mgr.c - appendEmptyBlock:\tCould not write to file %s\n", fHandle->fileName);
                 return RC_WRITE_FAILED;
	}
// Update File Handler
        fHandle->curPagePos = ftell(fp);

        return RC_OK;
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
// Initial setup
        FILE *fp = fHandle->mgmtInfo;
        int currentPos = ftell(fp);
	int offset = currentPos - (currentPos % PAGE_SIZE);
	int Pos = currentPos/PAGE_SIZE;
        if (DEBUG) printf("INFO storage_mgr.c - writeCurrentBlock: current block before write is, %d, offset is %d\n", Pos, offset);	
       	fseek(fp, offset, SEEK_SET);
// Overwrite.
	size_t written = fwrite(memPage, sizeof(char), PAGE_SIZE, fp);
        if (written != PAGE_SIZE){
                 printf("ERROR storage_mgr.c - writeCurrentBlock:\tCould not write to file %s\n", fHandle->fileName);
                 return RC_WRITE_FAILED;
	}
// Update File Handler
	fHandle->curPagePos = ftell(fp);
	Pos = fHandle->curPagePos / PAGE_SIZE;
	if (DEBUG) printf("INFO storage_mgr.c - writeCurrentBlock: current block after write is, %d, Current position in file is, %d\n", Pos, fHandle->curPagePos);
	return RC_OK;
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
        FILE *fp = fHandle->mgmtInfo;
        int currentPos = ftell(fp); //save the current position for later
        fseek(fp, 0, SEEK_END); // go to the End of the File
 // Create Buffer
        char buf[PAGE_SIZE];
        for (int i = 0; i <  PAGE_SIZE; i++){
                buf[i] = '\0';
        }

// Write to file
        if (DEBUG) printf("INFO storage_mgr.c - appendEmptyBlock:\twriting page file\n");
        size_t written = fwrite(buf,sizeof(char),PAGE_SIZE,fp);

// Error Handling
        if (written != PAGE_SIZE){
                 printf("ERROR storage_mgr.c - appendEmptyBlock:\tCould not write to file %s\n", fHandle->fileName);
                 return RC_WRITE_FAILED;
        }
        fseek(fp, currentPos, SEEK_SET);  //Reset the the page position to what it was initially.
        fHandle->totalNumPages++;
        return RC_OK;
}
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
        if (fHandle->totalNumPages >= numberOfPages){ // The file is either the same size or larger than the given number.
                return RC_OK; // Just Return With OK status
        }
        int currentPos = fHandle->curPagePos;
        int numberToAdd = numberOfPages - fHandle->totalNumPages;
	while (numberToAdd > 0){
		RC status = appendEmptyBlock(fHandle);
		if (status != RC_OK){
			return status;
		}
		numberToAdd--;
	}
        return RC_OK;
}

