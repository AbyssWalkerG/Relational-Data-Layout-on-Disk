//
// Created by 王瑾琛 on 2023/2/17.
//

#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]){
    if (argc < 3) {
        cout << "Usage: scan <heapfile> <page_size>";
        return 1;
    }
    string heapfile(argv[1]);
    int page_size = stoi(argv[2]);

    int count = 0;
    int pages = 0;

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    FILE *fileHeap = fopen(argv[1], "rb");
    if (!fileHeap)
    {
        cout << "File not found" << endl;
        return 1;
    }

    fseek(fileHeap, 0, SEEK_END);
    int file_size = ftell(fileHeap);
    pages = file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
            + file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = fileHeap;

    RecordIterator rr = RecordIterator(heap);
    while(rr.hasNext()){
        Record *record = new Record();
        *record = rr.next();
        for(auto iter = record->begin(); iter != record->end(); iter++) {
            if(iter + 1 == record->end()) {
                cout << *iter;
            }else{
                cout << *iter << ",";
            }
        }
        cout << endl;
        count += 1;
    }

    fclose(heap->file_ptr);

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);
    cout << "TIME: " << duration.count() << " milliseconds" << endl;
    cout << "Number of records: " << count << endl;
    cout << "Number of pages: " << pages << endl;

    return 0;
}