//
// Created by 王瑾琛 on 2023/2/18.
//
#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 6) {
        cout << "Usage: select <heapfile> <attribute_id> <start> <end> <page_size>" << endl;
        return 1;
    }

    string heapfile(argv[1]);
    int attribute_id = stoi(argv[2]);
    string start(argv[3]);
    string end(argv[4]);
    int page_size = stoi(argv[5]);

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    FILE *heapFile = fopen(argv[1], "r+");
    if (!heapFile) {
        cout << "File not found:  " << heapfile << endl;
        return 1;
    }

    if (attribute_id < 0 || attribute_id >= ATTR_COUNT) {
        cout << "attribute_id is " << attribute_id << ", must be between 0 and 99." << endl;
        return 1;
    }

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapFile;

    int pages;
    int count = 0;
    fseek(heapFile , 0, SEEK_END);
    int file_size = ftell(heapFile);
    pages = file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
            + file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;

    RecordIterator rr = RecordIterator(heap);
    while(rr.hasNext()){
        Record *record = new Record();
        *record = rr.next();
        string temp = record->at(attribute_id);
        if(temp >= start && temp <= end){
            temp = temp.substr(0, 5);
            cout << temp << endl;
            count += 1;
        }

    }

    fclose(heap->file_ptr);

    auto time_end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(time_end - begin);
    cout << "TIME: " << duration.count() << " milliseconds" << endl;
    cout << "Number of records: " << count << endl;
    cout << "Number of pages: " << pages << endl;

    return 0;
}