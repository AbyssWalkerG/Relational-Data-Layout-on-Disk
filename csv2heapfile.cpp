#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        cout << "Usage: csv2heapfile <csv_file> <heapfile> <page_size>";
        return 1;
    }

    string csv_file(argv[1]);
    string heapfile(argv[2]);
    int page_size = stoi(argv[3]);

    // set the initial variables
    int page_status = -1;
    int page_count = 0;
    int record_count = 0;
    auto page = new Page();

    // initialize the heapfile output
    FILE* fileHeap = fopen(argv[2], "w+b");
    if (!fileHeap) {
        std::cout << "File not created: " << argv[2] << endl;
        return 1;
    }
    Heapfile *heapFile = new Heapfile();
    init_heapfile(heapFile, page_size, fileHeap);

    // open csv file
    ifstream inFile(csv_file, ios::in);
    if(inFile.fail()) {
        cout<<"File not found"<<endl;
        return 0;
    }

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

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
            page_count += 1;
        }

        // write the record to page
        page_status = add_fixed_len_page(page, record);
        record_count += 1;

        //if page is full, write the page to heapFile, and init a new page
        if(page_status < 0) {
            // alloc a new page in heapFile
            PageID pid = alloc_page(heapFile);
            // write the full page to `pid` data page in heapFile
            write_page(page, heapFile, pid);
            // init a new page
            init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
            page_status = 0;
            page_count += 1;
            // write the record to page
            add_fixed_len_page(page, record);
        }
    }
    // if the last page is initialized, write it to the file
    if(page_status >= 0) {
        // alloc a new page in heapFile
        PageID pid = alloc_page(heapFile);
        // write the full page to `pid` data page in heapFile
        write_page(page, heapFile, pid);
    }

    fclose(fileHeap);
    inFile.close();

    // timestamp
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);

    // output statistics
    cout << "NUMBER OF RECORDS: " << record_count << endl;
    cout << "NUMBER OF PAGES: " << page_count << endl;
    cout << "TIME: " << duration.count() << " milliseconds" << endl;

    return 0;
}
