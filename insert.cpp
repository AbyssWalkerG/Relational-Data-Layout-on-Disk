#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        cout << "Usage: insert <heapfile> <csv_file> <page_size>";
        return 1;
    }

    string csv_file(argv[2]);
    string heapfile(argv[1]);
    int page_size = stoi(argv[3]);

    // set the initial variables
    int page_status = -1;
    int record_count = 0;
    auto page = new Page();

    // open csv file
    ifstream inFile(csv_file, ios::in);
    if(inFile.fail()) {
        cout<<"File not found"<<endl;
        return 0;
    }

    // heapfile must exist
    ifstream inFile_test(heapfile, ios::in | ios::binary);
    if(inFile_test.fail()){
        inFile_test.close();
        FILE* fileHeap = fopen(heapfile.c_str(), "w");
        fflush(fileHeap);
        fclose(fileHeap);
    }
    inFile_test.close();

    // initialize the heapfile output
    FILE* fileHeap = fopen(heapfile.c_str(), "r+b");
    if (!fileHeap) {
        std::cout << "File not found: " << heapfile << endl;
        return 1;
    }
    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = fileHeap;

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    // check if this heap file is empty or not
    fseek(heap->file_ptr, 0, SEEK_END);
    int file_size = ftell(heap->file_ptr);
    // if the heap file is empty (without directory), same as `csv2heapfile.cpp`
    if (file_size == 0 ) {
        string lineStr;
        while(getline(inFile, lineStr)) {
            stringstream ss(lineStr);
            string attr;
            // read csv file to a Record
            auto record = new Record;
            while (getline(ss, attr, ',')) {
                char *temp = (char *)malloc(strlen(attr.c_str()) + 1);
                strcpy(temp, attr.c_str());
                record->push_back(temp);
            }

            // check if a page is available
            if(page_status < 0) {
                init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
                page_status = 0;
            }

            // write the record to page
            page_status = add_fixed_len_page(page, record);
            record_count += 1;

            //if page is full, write the page to heapFile, and init a new page
            if(page_status < 0) {
                // alloc a new page in heapFile
                PageID pid = alloc_page(heap);
                // write the full page to `pid` data page in heapFile
                write_page(page, heap, pid);
                // init a new page
                init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
                page_status = 0;
                // write the record to page
                add_fixed_len_page(page, record);
            }
        }
        // if the last page is initialized, write it to the file
        if(page_status >= 0) {
            // alloc a new page in heapFile
            PageID pid = alloc_page(heap);
            // write the full page to `pid` data page in heapFile
            write_page(page, heap, pid);
        }

        fclose(fileHeap);
        inFile.close();

        // timestamp
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);

        // output statistics
        cout << "NUMBER OF INSERTED RECORDS: " << record_count << endl;
        cout << "TIME: " << duration.count() << " milliseconds" << endl;

        return 0;
    }

    // if the heap file is initialized, search next empty slot
    FreeSlotIterator freeSlotIterator = {heap};
    string lineStr;
    int hasNextEmptySlot = 1;
    while(getline(inFile, lineStr)) {
        stringstream ss(lineStr);
        string attr;
        // read csv file to a Record
        auto record = new Record;
        while (getline(ss, attr, ',')) {
            char *temp = (char *)malloc(strlen(attr.c_str()) + 1);
            strcpy(temp, attr.c_str());
            record->push_back(temp);
        }
        // search for next empty slot
        if(hasNextEmptySlot == 1){
            hasNextEmptySlot = freeSlotIterator.hasNext();
        }
        // if new page needed, the record should be written to a new page, till this new page is full, flush this page to
        if (hasNextEmptySlot != 1) {
            // check if a page is available
            if(page_status < 0) {
                init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
                page_status = 0;
            }
            // write the record to page
            page_status = add_fixed_len_page(page, record);
            record_count += 1;
            //if page is full, write the page to heapFile, and init a new page
            if(page_status < 0) {
                // alloc a new page in heapFile
                PageID pid = alloc_page(heap);
                // write the full page to `pid` data page in heapFile
                write_page(page, heap, pid);
                // init a new page
                init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
                page_status = 0;
                // write the record to page
                add_fixed_len_page(page, record);
            }
            continue;
        }
        // if there is an empty slot
        vector<int> ret = freeSlotIterator.next();
        int data_page_offset_in_heapfile = ret[0];
        int free_space_offset_in_heapfile = ret[1];
        init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
        // insert the record
        fseek(heap->file_ptr, data_page_offset_in_heapfile, SEEK_SET);
        fread(page->data, page_size, 1, heap->file_ptr);
        add_fixed_len_page(page, record);
        fseek(heap->file_ptr, data_page_offset_in_heapfile, SEEK_SET);
        fwrite(page->data, page_size, 1, heap->file_ptr);
        page_status = -1;
        // change the free space in directory
        fseek(heap->file_ptr, free_space_offset_in_heapfile, SEEK_SET);
        int freespace = 0;
        fread(&freespace, sizeof(int), 1, heap->file_ptr);
        freespace -= fixed_len_sizeof(record);
        fseek(heap->file_ptr, free_space_offset_in_heapfile, SEEK_SET);
        fwrite(&freespace, sizeof(int), 1, heap->file_ptr);
        record_count += 1;
        fflush(heap->file_ptr);
    }
    // if the last page is initialized, write it to the file
    if(page_status >= 0) {
        // alloc a new page in heapFile
        PageID pid = alloc_page(heap);
        // write the full page to `pid` data page in heapFile
        write_page(page, heap, pid);
    }

    fclose(fileHeap);
    inFile.close();

    // timestamp
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);

    // output statistics
    cout << "NUMBER OF INSERTED RECORDS: " << record_count << endl;
    cout << "TIME: " << duration.count() << " milliseconds" << endl;

    return 0;
}
