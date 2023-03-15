#ifndef TEST_LIBRARY_H
#define TEST_LIBRARY_H
#include <vector>
#include <iostream>
#include <cmath>
#include <cstring>
#include <stdio.h>

#define ATTR_COUNT 100
#define ATTR_LEN 10

using namespace std;

typedef const char* V;

typedef vector<V> Record;

typedef struct {
    void *data;
    int page_size;
    int slot_size;
} Page;

typedef struct {
    FILE *file_ptr;
    int page_size;
} Heapfile;

typedef int PageID; //start from 0, consecutive

typedef struct {
    int page_id;
    int slot;
} RecordID; // serialized record_id has the form as "<page_id>@<slot>"

class RecordIterator {
private:
    Heapfile *heapfile;
    Page *curPage;
    int curDirectoryOffset;
    Page *curDirectory;
    int curPid;
    int curSlot;
public:
    RecordIterator(Heapfile *heapfile);
    Record next();
    bool hasNext();
};

class FreeSlotIterator {
private:
    Heapfile *heapfile;
    Page *curPage;
    int curDirectoryOffset;
    Page *curDirectory;
    int curPid;
    int curSlot;
public:
    FreeSlotIterator(Heapfile *heapfile);
    vector<int> next();
    int hasNext(); //return: -2: new directory needed; -1: new page needed; 1 free slot found
};

/**
 * Self-defined
 */
typedef struct {
    int page_offset;
    int free_space;
} DirectoryEntry;

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record);

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf);

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record);

/**
 * Compute the number of bytes required to serialize record
 */
int var_len_sizeof(Record *record);

/**
 * Serialize the record using variable record encoding
 */
void var_len_write(Record *record, void *buf);

/**
 * Deserialize the `buf` which contains the variable record encoding.
 */
void var_len_read(void *buf, int size, Record *record);

/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size);

/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page);

/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page);

/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r);

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Self-defined
 * @param page
 * @return index (from 0 to m-1) of the first free slot in page, return -1 if not found
 */
int find_first_free_slot(Page *page);

/**
 * Self-defined
 * @param page
 * @param slot_index
 * @return 0 for free, 1 for occupied
 */
int check_slot_occupied(Page *page, int slot_index);

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid);

/**
 * Self-defined
 * @param page_size
 * @return The number of entries that a page can hold.
 */
int number_of_pages_per_directory_page(int page_size);

/**
 * Self-defined
 * @param file
 * @return  The real offset of the last directory page in the heapfile
 */
int find_the_offset_of_the_last_directory_page(FILE *file);

#endif //TEST_LIBRARY_H
