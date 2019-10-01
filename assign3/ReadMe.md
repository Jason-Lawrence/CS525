## CS525: Assingment-3
Jason Lawrence 

Alec Buchanan

## Structs
We implemented a new structure of our own and modified some of the preexisitng structs to implement the record manager

### RM_Record_Mgr
This structure was used to keep track of the BufferPool and PageHandles for pages in the table. 
It also keeps track of the total amount of tuples in the table and the amount of records that can be stored on a page.
A pointer to the record manager is stored in the void pointer in RM_TableData.

#### Parameters

|Data Type      |Variable      |Description         |
|:--------------|:-------------|:-------------------|
|BM_BufferPool *|bm            |This variable stores a pointer to a bufferPool manager to read and write data|
|BM_PageHandle *|page          |This variable stores a pointer to a PageHandle used to read record pages from the table|
|BM_PageHandle *|metadata      |This variable stores a pointer to a PageHandle used to read in the metadata for a table|
|int            |numTup        |This variable stores the amount of tuples that are in the table|
|int            |recordsPerPage|This variable stores the amount of slots a page has|

### RM_ScanHandle
This structure is used to store information about the scan. We added the following parameters to implement scans.

#### Parameters

|Data Type      |Variable      |Description         |
|:--------------|:-------------|:-------------------|
|Expr *         |cond          |This variable stores the search condition for the scan|
|int            |scanned       |This variable stores how many records were scanned. This number should never exceed the number of tuples stored in the record manager|
|int            |currentPage   |This variable stores the current page of the page file the scan is at|
|int            |currentSlot   |This variable stores the current slot of the page the scan is at|

## Table and Record Manager Functions
When we create a new table we use a temporary BufferPool Manager to handle writing the necessary metadata
to the first page (page 0). the order of the metadata is as follows: The number of tuples stored in the table, 
the amount of records that can be stored on each page, the number of attributes in the schema, the size of the key, 
and then the data for each atrribute. Opening a table initializes the Record Manager and stores it in the RM_TableData
struct. It then reads the metadata from page 0 and populates the corresponding data fields. 
Closing a table writes back the updated metadata to page 0, and then shutdowns the buffer manager.

## Record Functions
To insert a record it starts at page 1 of the table and looks linearly for an open slot. Filled slots are denoted
in memory with a '$' character at the start of the record. Any slot that does not have that indicator is considered
a free slot. if a page does not have a free slot it pins the next page and checks that one linearly. 
Once it finds a slot it prepends a '$' to the beginning of the slot and then writes the record data. 
When we delete a record we go to the page and slot according to the RID and then change the '$' to a '!' 
to denote that the slot is now free. When updating the record we go to the corresponding page and slot write the new data
and then return RC_OK. If the record is not found return the proper error. when retrieving a record we again
navigate to the corresponding page and slot and copy the contents of the slot to the record data field and return RC_OK
if the slot is open we then return a record not found error. 

## Scan Functions
When a client wants to start a scan we populate the fields of the scan handle. The client then 
calls next repeatedly until there are no more tuples in the table that fulfill the condition. When NULL is 
passed as the condition it returns every record found. For any non NULL condition when a record is found we call 
evalExpr() to check to see if the condition is met. Every record that is successfully found increases the scanned counter.
Once the scanned counter reaches the number stored in the numTup field of the record manager it knows that there are no
no more tuples in the table to be scanned. To close a scan the fields of the scan handle are reset.

## Schema Functions
To create a schema the client passes in the necessary data, and then our function populates the fields of the schema 
with the data that was passed in and then returns the schema object. To free a schema struct we free all of the malloced data
fields first and then free the schema variable. To obtain the record size we loop through all of the attributes in the
schema and sum the size of each attribute we also add one for the indicator flag at the start of each record.

## Attribute Functions
To get and set the attribute of a given attribute number we first must get the position in memory of that attribute.
Once we have that we must then check to see what the datatype is of that attribute and then set it accordingly. To retrieve 
it we must copy it into the value pointer given. 

## Ways to Improve
At the beginning of each page some space should be reserved to store data about 
what is on the page i.e.(number of free slots, type of records stored on the page)
Instead of having our records placed linearly on a page in FIFO we could sort the records and have a page 
dedicated to each type. 

## Contributions

### Jason Lawrence
- Makefile
- ReadMe
- Table Functions
- Scan Functions
- Attribute Functions
- Design

### Alec Buchanan
- Design
- Record Functions
- Schema Functions
- Record Manager Functions
- ReadMe
