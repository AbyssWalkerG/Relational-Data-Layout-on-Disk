#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        cout << "Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>";
        return 1;
    }
    string csv_file(argv[1]);
    string page_file(argv[2]);
    int page_size = stoi(argv[3]);

    // set the initial variables
    int page_status = -1;
    int page_count = 0;
    int record_count = 0;
    auto page = new Page();

    ofstream outFile(page_file, ios::out | ios::binary);
    if(outFile.fail()) {
        cout<<"File not created"<<endl;
        return 0;
    }
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
        //if page is full, write the page to file, and init a new page
        if(page_status < 0) {
            outFile.write((const char *)page->data, page->page_size);
            init_fixed_len_page(page, page_size, fixed_len_sizeof(record));
            page_status = 0;
            page_count += 1;
            // write the record to page
            add_fixed_len_page(page, record);
        }
    }
    // if the last page is initialized, write it to the file
    if(page_status >= 0) {
        outFile.write((const char *)page->data, page->page_size);
    }
    inFile.close();
    outFile.close();

    // timestamp
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);

    // output statistics
    cout << "NUMBER OF RECORDS: " << record_count << endl;
    cout << "NUMBER OF PAGES: " << page_count << endl;
    cout << "TIME: " << duration.count() << " milliseconds" << endl;

    return 0;
}
