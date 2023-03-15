#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 6) {
        cout << "Usage: select <colstore_name> <attribute_id> <start> <end> <page_size>" << endl;
        return 1;
    }

    const char* colstore_name(argv[1]);
    int attribute_id = stoi(argv[2]);
    if (attribute_id < 0 || attribute_id >= ATTR_COUNT) {
        std::cout << "Error, attribute_id is " << attribute_id << ", out of bound\n";
        return 1;
    }
    string start(argv[3]);
    string end(argv[4]);
    int page_size = stoi(argv[5]);

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    std::ostringstream path_stream;
    path_stream << colstore_name << "/" << attribute_id;
    std::string tmp = path_stream.str();
    const char* path = tmp.c_str();

    FILE *col_heapfile = fopen(path, "r+");
    if (!col_heapfile) {
        std::cout << "Could not open file " << path << "\n";
        return 1;
    }

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = col_heapfile;

    int pages;
    int count = 0;
    fseek(col_heapfile , 0, SEEK_END);
    int file_size = ftell(col_heapfile);
    pages = file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
            + file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;

    int tuple_count=0;
    RecordIterator rr = RecordIterator(heap);
    while(rr.hasNext()){
        tuple_count++;
        Record *record = new Record();
        *record = rr.next();
        string temp = record->at(0);
        if(temp >= start && temp <= end){
            temp = temp.substr(0, 5);
            cout << temp << endl;
            count += 1;
        }

    }

    fclose(heap->file_ptr);
    delete heap;

    auto time_end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(time_end - begin);
    cout << "TIME: " << duration.count() << " milliseconds" << endl;
    cout << "Number of records: " << count << endl;
    cout << "Number of pages: " << pages << endl;

    return 0;
}
