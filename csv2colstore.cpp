//
// Created by Aiden Liu on 2/22/23.
//
#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <sys/stat.h>
#include <filesystem>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        cout << "Usage: select <csv_file> <colstore_name> <page_size>" << endl;
        return 1;
    }

    std::ifstream file_csv;
    file_csv.open(argv[1]);
    if(!file_csv) {
        cout<<"File not found "<<argv[1]<<endl;
        return 0;
    }

    const char *colstore_name(argv[2]);
    int page_size = stoi(argv[3]);

    //make a directory for the heapfiles
    if(!filesystem::exists(colstore_name)){
        if(mkdir(colstore_name, 0777) != 0){
            std::cout << "Could not create directory " << colstore_name << endl;
            return 1;
        }
    }

    // set the initial variables
    //int page_status = -1;
    int page_count = 0;
    int record_count = 0;
    int record_size = ATTR_LEN;
    //auto page = new Page();


    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    std::vector<Record> *records = new std::vector<Record>();

    string lineStr;
    while(file_csv) {
        std::string line;
        file_csv>>line;

        if(line.size()==0){
            //ignore empty lines
            continue;
        }

        line.erase(std::remove(line.begin(), line.end(),','), line.end());

        // read csv file to a Record
        Record *record = new Record;
        fixed_len_read((char*)line.c_str(), ATTR_LEN * ATTR_COUNT, record);
        records->push_back(*record);
        delete record;
    }

    file_csv.close();
    for(int i=0; i<ATTR_COUNT; i++){

        int page_status = -1;
        auto page = new Page();

        std::ostringstream path_stream;
        path_stream << colstore_name << "/" << i;
        std::string tmp = path_stream.str();
        const char* path = tmp.c_str();

        FILE *col_heapfile = fopen(path, "w+");
        if (!col_heapfile) {
            std::cout << "Could not create file " << path << " for column " << i << "\n";
            return 1;
        }

        auto *heapFile = new Heapfile();
        init_heapfile(heapFile, page_size, col_heapfile);

        Record *col_record = new Record;

        for(size_t j = 0; j < records->size(); ++j) {
            col_record->clear();
            col_record->push_back(records->at(j).at(i));
            if (page_status < 0) {
                init_fixed_len_page(page, page_size, record_size);
                page_status = 0;
                page_count += 1;
            }

            // write the record to page
            page_status = add_fixed_len_page(page, col_record);
            record_count += 1;

            //if page is full, write the page to heapFile, and init a new page
            if (page_status < 0) {
                // alloc a new page in heapFile
                PageID pid = alloc_page(heapFile);
                // write the full page to `pid` data page in heapFile
                write_page(page, heapFile, pid);

                // init a new page
                init_fixed_len_page(page, page_size, record_size);
                page_status = 0;
                page_count += 1;
                // write the record to page
                add_fixed_len_page(page, col_record);
            }
        }

        // if the last page is initialized, write it to the file
        if (page_status >= 0) {
            // alloc a new page in heapFile
            PageID pid = alloc_page(heapFile);
            // write the full page to `pid` data page in heapFile
            write_page(page, heapFile, pid);
        }
        fclose(col_heapfile);
    }



    // timestamp
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);

    // output statistics
    cout << "NUMBER OF RECORDS: " << record_count/ATTR_COUNT << endl;
    cout << "NUMBER OF PAGES: " << page_count << endl;
    cout << "TIME: " << duration.count() << " milliseconds" << endl;

    return 0;
}
