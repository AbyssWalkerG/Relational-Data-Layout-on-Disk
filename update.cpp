#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 6) {
        cout << "Usage: update <heapfile> <record_id> <attribute_id> <new_value> <page_size>" << endl << "<record_id>: <page_id>@<slot>";
        return 1;
    }

    string heapfile(argv[1]);
    string serialized_record_id = argv[2];
    int attribute_id = stoi(argv[3]);
    const char *new_value = argv[4];
    int page_size = stoi(argv[5]);

    FILE *heapFile = fopen(argv[1], "r+");
    if (!heapFile) {
        cout << "File not found:  " << heapfile << endl;
        return 1;
    }

    int delimiter_position = serialized_record_id.find('@');
    if (delimiter_position == 0 || delimiter_position == serialized_record_id.length() - 1) {
        std::cout << "record_id must be: \"<page_id>@<slot>\" " << endl;
        return 1;
    }

    RecordID *recordId = new RecordID();
    recordId->page_id = stoi(serialized_record_id.substr(0, delimiter_position));
    recordId->slot = stoi(serialized_record_id.substr(delimiter_position + 1));

    if (attribute_id < 0 || attribute_id >= ATTR_COUNT) {
        std::cout << "attribute_id is " << attribute_id << ", must be between 0 and 99." << endl;
        return 1;
    }

    if (strlen(new_value) != 10) {
        std::cout << "new_value is " << strlen(new_value) << " bytes, but must be 10 bytes long." << endl;
        return 1;
    }

    Heapfile *heap = new Heapfile();
    heap->page_size = page_size;
    heap->file_ptr = heapFile;

    fseek(heap->file_ptr, 0, SEEK_END);
    int file_size = ftell(heap->file_ptr);
    int page_count = file_size / page_size / (number_of_pages_per_directory_page(page_size) + 1) * number_of_pages_per_directory_page(page_size)
                     + file_size / page_size % (number_of_pages_per_directory_page(page_size) + 1) - 1;
    if(recordId->page_id >= page_count){
        cout << "page_id " << recordId->slot << " is invalid. Valid range is 0 to "<< page_count-1 << "." << endl;
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
        cout << "no record found at slot " << recordId->slot << endl;
        return 1;
    }

    memcpy((char*)page->data + ATTR_LEN*ATTR_COUNT*recordId->slot + ATTR_LEN*attribute_id, new_value, ATTR_LEN);
    write_page(page, heap, recordId->page_id);

    fclose(heap->file_ptr);

    return 0;
}
