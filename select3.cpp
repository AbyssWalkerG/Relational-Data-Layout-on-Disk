#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 7) {
        cout << "Usage: select <colstore_name> <attribute_id> <return_attribute_id> <start> <end> <page_size>" << endl;
        return 1;
    }

    const char* colstore_name(argv[1]);
    int attribute_id = stoi(argv[2]);
    if (attribute_id < 0 || attribute_id >= ATTR_COUNT) {
        std::cout << "Error, attribute_id is " << attribute_id << ", out of bound\n";
        return 1;
    }
    int return_attribute_id = stoi(argv[3]);
    if (return_attribute_id < 0 || return_attribute_id >= ATTR_COUNT) {
        std::cout << "Error, attribute_id is " << attribute_id << ", out of bound\n";
        return 1;
    }
    string start(argv[4]);
    string end(argv[5]);
    int page_size = stoi(argv[6]);

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    std::ostringstream path_stream;
    path_stream << colstore_name << "/" << attribute_id;
    std::string tmp = path_stream.str();
    const char* path = tmp.c_str();

    std::ostringstream return_path_stream;
    return_path_stream << colstore_name << "/" << return_attribute_id;
    std::string return_tmp = return_path_stream.str();
    const char* return_path = return_tmp.c_str();

    FILE *col_heapfile = fopen(path, "r+");
    if (!col_heapfile) {
        std::cout << "Could not open file " << path << "\n";
        return 1;
    }

    FILE *return_col_heapfile = fopen(return_path, "r");
    if (!return_col_heapfile) {
        std::cout << "Error, could not open file " << return_path << "\n";
        return 1;
    }

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = col_heapfile;

    Heapfile *return_heap = new Heapfile();
    return_heap->page_size = page_size;
    return_heap->file_ptr = return_col_heapfile;

    int pages;
    int count = 0;
    fseek(col_heapfile , 0, SEEK_END);
    int file_size = ftell(col_heapfile);
    pages = file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
            + file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;

    int return_pages;
    fseek(return_col_heapfile , 0, SEEK_END);
    int return_file_size = ftell(return_col_heapfile);
    return_pages = return_file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
            + return_file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;

    RecordIterator rr = RecordIterator(heap);
    RecordIterator return_rr = RecordIterator(return_heap);
    while(rr.hasNext() && return_rr.hasNext()){
        Record *record = new Record();
        Record *rrecord = new Record();
        *record = rr.next();
        *rrecord = return_rr.next();
        string temp = record->at(0);
        string return_temp = rrecord->at(0);
        if(temp >= start && temp <= end){
            return_temp = return_temp.substr(0, 5);
            cout << return_temp << endl;
            count += 1;
        }

    }

    fclose(heap->file_ptr);
    fclose(return_heap->file_ptr);
    delete heap;
    delete return_heap;

    auto time_end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(time_end - begin);
    cout << "TIME: " << duration.count() << " milliseconds" << endl;
    cout << "Number of records: " << count << endl;
    cout << "Number of pages: " << pages << endl;

    return 0;
}
