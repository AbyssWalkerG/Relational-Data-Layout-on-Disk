#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        cout << "Usage: delete <heapfile> <record_id> <page_size>" << endl;
        return 1;
    }

    string heapfile(argv[1]);
    string serialized_record_id = argv[2];
    int page_size = stoi(argv[3]);

    FILE *heapFile = fopen(argv[1], "r+");
    if (!heapFile) {
        cout << "File not found:  " << heapfile << endl;
        return 1;
    }

    int delimiter_position = serialized_record_id.find('@');
    if (delimiter_position == 0 || delimiter_position == serialized_record_id.length() - 1) {
        cout << "record_id must be: \"<page_id>@<slot>\" " << endl;
        return 1;
    }

    RecordID *recordId = new RecordID();
    recordId->page_id = stoi(serialized_record_id.substr(0, delimiter_position));
    recordId->slot = stoi(serialized_record_id.substr(delimiter_position + 1));

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapFile;

    fseek(heap->file_ptr, 0, SEEK_END);
    int file_size = ftell(heap->file_ptr);
    int page_count = file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
                     + file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;
    if(recordId->page_id >= page_count){
        cout << "page_id " << recordId->page_id << " is invalid. Valid range is 0 to "<< page_count-1 << "." << endl;
        return 1;
    }

    Page *page = new Page();
    char buf[page_size];
    page->data = buf;
    read_page(heap, recordId->page_id, page);

    if (recordId->slot >= fixed_len_page_capacity(page)) {
        cout << "slot_number " << recordId->slot << " is invalid. Valid range is 0 to "<< fixed_len_page_capacity(page)-1 << endl;
        return 1;
    }

    if (!check_slot_occupied(page, recordId->slot)) {
        std::cout << "no record found at slot " << recordId->slot << endl;
        return 1;
    }

    int i = recordId->slot/8;
    int res = recordId->slot % 8;
    unsigned char t = 0;
    memcpy(&t, (char*)page->data+page_size-5-i-4, 1);
    t = t ^ (1 << res);
    memcpy((char*)page->data+page_size-5-i-4, &t, 1);

    write_page(page, heap, recordId->page_id);

    fclose(heap->file_ptr);

    return 0;

}
