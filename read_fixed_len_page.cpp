#include <iostream>
#include "library.h"
#include <fstream>
#include <string>
#include <chrono>

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        cout << "Usage: read_fixed_len_page <page_file> <page_size>";
        return 1;
    }
    string page_file(argv[1]);
    int page_size = stoi(argv[2]);

    // timestamp
    auto begin = chrono::high_resolution_clock::now();

    ifstream inFile(page_file, ios::in | ios::binary);
    if(inFile.fail()){
        cout<<"File not found"<<endl;
        return 0;
    }

    int count = 0;
    auto page = new Page();
    page->page_size = page_size;
    char buf[page_size];
    page->data = &buf;
    while(inFile.read((char *)page->data, page_size)){
        int capacity = 0;
        memcpy(&capacity, (char *)page->data + page_size - 4 -4, sizeof(capacity));
        int slot_size = 0;
        memcpy(&slot_size, (char *)page->data + page_size - 4, sizeof(slot_size));
        page->slot_size = slot_size;
        // print records
        for (int i = 0; i < capacity; i++) {
            auto record = new Record();
            // check if the slot is occupied or not
            if(check_slot_occupied(page, i)) {
                read_fixed_len_page(page, i, record);
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
        }
    }

    inFile.close();

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - begin);
    cout << "TIME: " << duration.count() << " milliseconds" << endl;
    cout << "Number of records: " << count << endl;

    return 0;
}
