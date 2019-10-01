#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"
#define TESTPF2 "test_pagefile2.bin"

/* prototypes for test functions */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);
static void testMultiPageContent(void);

/* main function running all tests */
int
main (void)
{
  testName = "";
  
  initStorageManager();
// Given Test Cases
  //testCreateOpenClose();
  //testSinglePageContent();

//Additional Test Cases
  testMultiPageContent();

  return 0;
}

void 
testMultiPageContent(void)
{
// Initial steps
  SM_FileHandle fh1;
  SM_FileHandle fh2;

  SM_PageHandle ph1;
  SM_PageHandle ph2;
  SM_PageHandle ph3;
  SM_PageHandle ph4;
  SM_PageHandle ph5;
  SM_PageHandle ph6;
  SM_PageHandle ph7;
  
  TEST_CHECK(createPageFile(TESTPF));
  TEST_CHECK(createPageFile(TESTPF2));

  TEST_CHECK(openPageFile(TESTPF, &fh1));
  TEST_CHECK(openPageFile(TESTPF2, &fh2));
  
  ASSERT_TRUE(strcmp(fh1.fileName, TESTPF) == 0, "First filename correct");
  ASSERT_TRUE((fh1.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh1.curPagePos == 0), "freshly opened file's page position should be 0");

  ASSERT_TRUE(strcmp(fh2.fileName, TESTPF2) == 0, "Second filename correct");
  ASSERT_TRUE((fh2.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh2.curPagePos == 0), "freshly opened file's page position should be 0");

  printf("INFO test_assign1_1.c - testMultiPageContent: Finished file creation\n");
// Begin interesting stuff
  ph1 = (SM_PageHandle) malloc(PAGE_SIZE);
  ph2 = (SM_PageHandle) malloc(PAGE_SIZE);
  ph3 = (SM_PageHandle) malloc(PAGE_SIZE);
  ph4 = (SM_PageHandle) malloc(PAGE_SIZE);
  ph5 = (SM_PageHandle) malloc(PAGE_SIZE);
  ph6 = (SM_PageHandle) malloc(PAGE_SIZE);
  ph7 = (SM_PageHandle) malloc(PAGE_SIZE);  

  for(int i = 0; i < PAGE_SIZE; i++){
    ph1[i] = (i % 5) +'0';
    ph2[i] = 'a';
    ph3[i] = 'b';
  }
  // Test Ensure Capacity
  TEST_CHECK(ensureCapacity(3, &fh1));
  ASSERT_TRUE((fh1.totalNumPages == 3), "Expected 3 pages in the file");
 
  // Begin write and verify the first file
  ASSERT_TRUE(getBlockPos(&fh1) == 0, "Block position is the start");
  TEST_CHECK(writeCurrentBlock(&fh1, ph1));
  TEST_CHECK(readFirstBlock(&fh1, ph4));
  for(int j = 0; j < PAGE_SIZE; j++){
    ASSERT_TRUE((ph4[j] == (j % 5) + '0'), "character in page read from disk is the one we expected.");
  }
  ASSERT_TRUE(getBlockPos(&fh1) == 1, "Block position is 1");
  TEST_CHECK(writeCurrentBlock(&fh1, ph2));
  moveBlockPos(1, &fh1);
  TEST_CHECK(readBlock(1, &fh1, ph5)); //TEST_CHECK(readCurrentBlock(&fh1, ph5));
  for(int k = 0; k < PAGE_SIZE; k++){
    ASSERT_TRUE((ph5[k] == 'a'), "character in page read from disk is the one we expected.");
  }
  moveBlockPos(2, &fh1);
  ASSERT_TRUE(getBlockPos(&fh1) == 2, "Block Position is 2");
  TEST_CHECK(writeCurrentBlock(&fh1, ph3));
  TEST_CHECK(readLastBlock(&fh1, ph6));
  for(int l = 0; l < PAGE_SIZE; l++){
    ASSERT_TRUE((ph6[l] == 'b'), "character in page read from disk is the one we expected.");
  }
  moveBlockPos(0, &fh1);
  ASSERT_TRUE(getBlockPos(&fh1) == 0, "Block position is the start");  
  printf("INFO test_assign1_1.c - testMultiPageContent: Test File 1 has been written and verified.\n");

//End of write and verify of first file begin copy into second file.
  
  printf("INFO test_assign1_1.c - testMultiPageContent: Beginning file copy.\n");
  TEST_CHECK(writeCurrentBlock(&fh2, ph4));
  moveBlockPos(0, &fh2);
  TEST_CHECK(readCurrentBlock(&fh2, ph3));
  TEST_CHECK(appendEmptyBlock(&fh2));
  ASSERT_TRUE((fh2.totalNumPages == 2), "Empty block successfully appended.");
  TEST_CHECK(writeBlock(1, &fh2, ph5));
  ASSERT_TRUE(getBlockPos(&fh2) == 2, "Block Position is 2");
  TEST_CHECK(readPreviousBlock(&fh2, ph1));
  TEST_CHECK(appendEmptyBlock(&fh2));
  TEST_CHECK(writeBlock(2, &fh2, ph6));
  moveBlockPos(1, &fh2);
  TEST_CHECK(readNextBlock(&fh2, ph2));
  for(int a = 0; a < PAGE_SIZE; a++){
    ASSERT_TRUE((ph3[a] == (a % 5) + '0'), "character in page read from disk is the one we expected.");
    ASSERT_TRUE((ph1[a] == 'a'), "character in page read from disk is the one we expected which was a.");
    ASSERT_TRUE((ph2[a] == 'b'), "character in page read from disk is the one we expected which was b.");
  }
  printf("INFO test_assign1_1.c - testMultiPageContent: File Copy successful.\n");
  TEST_DONE();
//End of File Copy
}

/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */
void
testCreateOpenClose(void)
{
  SM_FileHandle fh;

  testName = "test create open and close methods";

  TEST_CHECK(createPageFile (TESTPF));
  
  TEST_CHECK(openPageFile (TESTPF, &fh));
  ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
  ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
  ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");

  TEST_CHECK(closePageFile (&fh));
  TEST_CHECK(destroyPageFile (TESTPF));

  // after destruction trying to open the file should cause an error
  ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

  TEST_DONE();
}

/* Try to create, open, and close a page file */
void
testSinglePageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test single page content";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");
  
  // read first page into handle
  TEST_CHECK(readFirstBlock (&fh, ph));
  // the page should be empty (zero bytes)
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
  printf("first block was empty\n");
    
  // change ph to be a string and write that one to disk
  for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';
  TEST_CHECK(writeBlock (0, &fh, ph));
  printf("writing first block\n");

  // read back the page containing the string and check that it is correct
  TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
  printf("reading first block\n");

  // destroy new page file
  TEST_CHECK(destroyPageFile (TESTPF));  
  
  TEST_DONE();
}
