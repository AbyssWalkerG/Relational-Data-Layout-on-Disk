#include "library.h"


int fixed_len_sizeof(Record *record) {
    int size = 0;
    for (auto r: *record) {
        size += strlen(r);
    }
    return size;
}

void fixed_len_write(Record *record, void *buf) {
    int i = 0;
    for(auto r: *record){
        memcpy((char *)buf+i*ATTR_LEN, r, ATTR_LEN);
        i++;
    }
}

void fixed_len_read(void *buf, int size, Record *record) {
    for(int i=0; i*ATTR_LEN<size; i++) {
        char *attr;
        attr = (char*) malloc(ATTR_LEN + 1);
        memset(attr, 0, ATTR_LEN + 1);
        memcpy(attr, (char *)buf+i*ATTR_LEN, ATTR_LEN);
        record->push_back(attr);
    }
}

int var_len_sizeof(Record *record) {
    unsigned short attrs_count = record->size();
    int size = sizeof(attrs_count) * (attrs_count + 1);
    for(auto r: *record) {
        size += strlen(r);
    }
    return size;
}

void var_len_write(Record *record, void *buf) {
    int size = fixed_len_sizeof(record);
    unsigned short attrs_count = record->size();
    unsigned short offset = 0;
    memcpy((char *)buf+offset, &attrs_count, sizeof(attrs_count));
    offset = sizeof(offset) * (record->size()+1);
    unsigned short i = 1;
    for(auto r: *record) {
        unsigned short attr_len = strlen(r);
        memcpy((char *)buf+offset, r, attr_len);
        offset += attr_len;
        memcpy((char *)buf+sizeof(offset)*i, &offset, sizeof(offset));
        i++;
    }
}

void var_len_read(void *buf, int size, Record *record) {
    unsigned short attrs_count = 0;
    memcpy(&attrs_count, (char *)buf, sizeof(attrs_count));
    unsigned short offset_l = sizeof(offset_l) * (attrs_count + 1);
    unsigned short offset_h = 0;
    for(unsigned short i=1; i<=attrs_count; i++) {
        memcpy(&offset_h, (char *)buf+sizeof(offset_l)*i, sizeof(offset_h));
        unsigned short attr_len = offset_h - offset_l;
        char *attr;
        attr = (char*) malloc(attr_len + 1);
        memset(attr, 0, attr_len + 1);
        memcpy(attr, (char *)buf+offset_l, attr_len);
        offset_l = offset_h;
        record->push_back(attr);
    }
}

void init_fixed_len_page(Page *page, int page_size, int slot_size) {
    page->page_size = page_size;
    page->slot_size = slot_size;
    page->data = malloc(page_size);
    //initialize trailer
    int capacity = fixed_len_page_capacity(page);
    memcpy((char *)page->data + page->page_size - 4 -4, &capacity, sizeof(capacity));
    memcpy((char *)page->data + page->page_size - 4, &slot_size, sizeof(slot_size));
    int avail_bytes = (int)ceil(capacity/8.0);
    unsigned char a = '\0';
    for(int i=0; i<avail_bytes; i++) {
        memcpy((char *)page->data + page->page_size - 5 - i -4, &a, sizeof(a));
    }
}

int fixed_len_page_capacity(Page *page) {
    return (int)((page->page_size - 5 -4)/(page->slot_size + 0.125));
}

int fixed_len_page_freeslots(Page *page) {
    int capacity = 0;
    memcpy(&capacity, (char *)page->data + page->page_size - 4 -4, sizeof(capacity));
    int slot_counts = 0;
    int free_slots = 0;
    for(int i=0; i < ceil(capacity/8.0);i++) {
        unsigned char c = '\0';
        memcpy(&c, (char *)page->data + page->page_size - 5 - i -4, 1);
        for(int j=0; j<8; j++) {
            if((c>>j & 1) == 0) {
                free_slots += 1;
            }
            slot_counts += 1;
            if(slot_counts >= capacity) {
                break;
            }
        }
        if(slot_counts >= capacity) {
            break;
        }
    }
    return free_slots;
}

int find_first_free_slot(Page *page) {
    int capacity = 0;
    memcpy(&capacity, (char *)page->data + page->page_size - 4 -4, sizeof(capacity));
    int slot_counts = 0;
    int free_slot_index = 0;
    bool found = false;
    for(int i=0; i < ceil(capacity/8.0);i++) {
        if(found || (slot_counts >= capacity)) {
            break;
        }
        unsigned char c = '\0';
        memcpy(&c, (char *)page->data + page->page_size - 5 - i -4, 1);
        for(int j=0; j<8; j++) {
            if((c>>j & 1) == 0) {
                free_slot_index = 8 * i + j;
                found = true;
                break;
            }
            slot_counts += 1;
            if(slot_counts >= capacity) {
                break;
            }
        }
    }
    if(found){
        return free_slot_index;
    }else{
        return -1;
    }
}

int add_fixed_len_page(Page *page, Record *r) {
    int free_slot_index = find_first_free_slot(page);
    if(free_slot_index < 0) {
        return -1;
    }else{
        fixed_len_write(r, (char *)page->data + free_slot_index * page->slot_size);
        int i = free_slot_index / 8;
        int rem = free_slot_index % 8;
        unsigned char c = '\0';
        memcpy(&c, (char *)page->data + page->page_size - 5 - i -4, 1);
        c = c | 1<<rem;
        memcpy((char *)page->data + page->page_size - 5 - i -4, &c, 1);
        return free_slot_index;
    }
}

void write_fixed_len_page(Page *page, int slot, Record *r) {
    fixed_len_write(r, (char *)page->data + slot * page->slot_size);
    int i = slot / 8;
    int rem = slot % 8;
    unsigned char c = '\0';
    memcpy(&c, (char *)page->data + page->page_size - 5 - i -4, 1);
    c = c | 1<<rem;
    memcpy((char *)page->data + page->page_size - 5 - i -4, &c, 1);
}

void read_fixed_len_page(Page *page, int slot, Record *r) {
    if(check_slot_occupied(page, slot)) {
        fixed_len_read((char *) page->data + page->slot_size * slot, page->slot_size, r);
    }
}

int check_slot_occupied(Page *page, int slot_index) {
    int i = slot_index / 8;
    int rem = slot_index % 8;
    unsigned char c = '\0';
    memcpy(&c, (char *)page->data + page->page_size - 5 - i -4, 1);
    c = c>>rem & 1;
    return c;
}

int number_of_pages_per_directory_page(int page_size) {
    int n = (page_size - 4) / 8;
    return n;
}

void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){
    heapfile->file_ptr = file;
    heapfile->page_size = page_size;

    //the relative offset of the next directory page from current directory page
    int next_directory_page_relative_offset = 0;
    fseek(heapfile->file_ptr, 0, SEEK_SET);
    fwrite(&next_directory_page_relative_offset, sizeof(next_directory_page_relative_offset), 1, file);
    //write empty entries to the directory page
    int n = number_of_pages_per_directory_page(page_size);
    for (int i = 1; i <= n; i++) {
        // offset for pages
        int data_page_offset = heapfile->page_size * i;
        fwrite(&data_page_offset, sizeof(data_page_offset), 1, file);
        // freespace (initialized as -1)
        int unoccupied_page_free_space = -1;
        fwrite(&unoccupied_page_free_space, sizeof(unoccupied_page_free_space), 1, file);
    }
    int rem = (heapfile->page_size-4) % (2*sizeof(int));
    char t = '\0';
    for(int j=0; j < rem; j++){
        fwrite(&t, sizeof(char), 1, file);
    }
    fflush(file);
}

int find_the_offset_of_the_last_directory_page(FILE *file) {
    fseek(file, 0, SEEK_SET);
    int next_directory_page_relative_offset = 0;
    int page_offset_in_heapfile = 0;
    fread(&next_directory_page_relative_offset, sizeof(next_directory_page_relative_offset), 1, file);
    page_offset_in_heapfile += next_directory_page_relative_offset;
    while(next_directory_page_relative_offset != 0) {
        fseek(file, page_offset_in_heapfile, SEEK_SET);
        fread(&next_directory_page_relative_offset, sizeof(next_directory_page_relative_offset), 1, file);
        page_offset_in_heapfile += next_directory_page_relative_offset;
    }
    return page_offset_in_heapfile;
}

PageID alloc_page(Heapfile *heapfile) {
    int last_directory_page_offset = find_the_offset_of_the_last_directory_page(heapfile->file_ptr);
    fseek(heapfile->file_ptr, last_directory_page_offset + 4, SEEK_SET);

    int data_page_offset = 0;
    int freespace = 0;
    int n = number_of_pages_per_directory_page(heapfile->page_size);
    for (int i = 0; i < n; i++){
        fread(&data_page_offset, sizeof(int), 1, heapfile->file_ptr);
        fread(&freespace, sizeof(int), 1, heapfile->file_ptr);

        // if there is an unoccupied data page (as the initial freespace to be -1)
        if (freespace == -1) {
            // change the freespace to page_size
            fseek(heapfile->file_ptr, -4, SEEK_CUR);
            int cur = ftell(heapfile->file_ptr);
            fwrite(&heapfile->page_size, sizeof(heapfile->page_size), 1, heapfile->file_ptr);

            // append an empty data page to the file
            char *new_data_page = new char[heapfile->page_size];
            memset(new_data_page, 0, heapfile->page_size);
            fseek(heapfile->file_ptr, 0, SEEK_END);
            cur = ftell(heapfile->file_ptr);
            fwrite(new_data_page, heapfile->page_size, 1, heapfile->file_ptr);
            fflush(heapfile->file_ptr);

            // calculate page_id
            PageID page_id = last_directory_page_offset / heapfile->page_size *
                             number_of_pages_per_directory_page(heapfile->page_size) / (number_of_pages_per_directory_page(heapfile->page_size) + 1)
                             + i;
            return page_id;
        }
    }

    // if this directory is full
    fseek(heapfile->file_ptr, 0, SEEK_END);
    // write offset of new directory into the previous
    int current_position = ftell(heapfile->file_ptr);
    int next_directory_page_relative_offset = current_position - last_directory_page_offset;
    fseek(heapfile->file_ptr, last_directory_page_offset, SEEK_SET);
    fwrite(&next_directory_page_relative_offset, sizeof(next_directory_page_relative_offset), 1, heapfile->file_ptr);

    // initialize a new directory page
    fseek(heapfile->file_ptr, 0, SEEK_END);
    next_directory_page_relative_offset = 0;
    fwrite(&next_directory_page_relative_offset, sizeof(next_directory_page_relative_offset), 1, heapfile->file_ptr);
    for(int i = 1; i <= number_of_pages_per_directory_page(heapfile->page_size); i++) {
        int unoccupied_page_space = -1;
        data_page_offset = current_position + i * heapfile->page_size;
        fwrite(&data_page_offset, sizeof(data_page_offset), 1,heapfile->file_ptr);
        if(i == 1){
            // change the freespace to page_size
            fwrite(&heapfile->page_size, sizeof(heapfile->page_size), 1, heapfile->file_ptr);
        }else {
            fwrite(&unoccupied_page_space, sizeof(unoccupied_page_space), 1, heapfile->file_ptr);
        }
    }
    int rem = (heapfile->page_size-4) % (2*sizeof(int));
    char t = '\0';
    for(int j=0; j < rem; j++){
        fwrite(&t, sizeof(char), 1, heapfile->file_ptr);
    }



    // append an empty data page to the file
    fseek(heapfile->file_ptr, 0, SEEK_END);
    char *new_data_page = new char[heapfile->page_size];
    memset(new_data_page, 0, heapfile->page_size);
    fwrite(new_data_page, heapfile->page_size, 1, heapfile->file_ptr);
    fflush(heapfile->file_ptr);

    // calculate page_id
    last_directory_page_offset = find_the_offset_of_the_last_directory_page(heapfile->file_ptr);
    PageID page_id = last_directory_page_offset / heapfile->page_size *
                     number_of_pages_per_directory_page(heapfile->page_size) / (number_of_pages_per_directory_page(heapfile->page_size) + 1)
                     + 0;
    return page_id;
}

void write_page(Page *page, Heapfile *heapfile, PageID pid) {
    // write the page to data page
    int data_page_offset = (pid / number_of_pages_per_directory_page(heapfile->page_size)
            * (number_of_pages_per_directory_page(heapfile->page_size) + 1) + 1 + pid %
            number_of_pages_per_directory_page(heapfile->page_size))* heapfile->page_size;
    fseek(heapfile->file_ptr, data_page_offset, SEEK_SET);
    fwrite(page->data, page->page_size, 1,heapfile->file_ptr);

    // find the directory containing this page's entry
    int directory_offset = pid / number_of_pages_per_directory_page(heapfile->page_size)
            * (number_of_pages_per_directory_page(heapfile->page_size)+1)
            * heapfile->page_size; // start from 0
    // find the relative offset of the data page in the directory page
    int data_page_relative_offset_in_directory = pid % number_of_pages_per_directory_page(heapfile->page_size) * 8; // start from 0
    // change the freespace
    fseek(heapfile->file_ptr, directory_offset + 4 + data_page_relative_offset_in_directory + 4, SEEK_SET);
    int freespace = fixed_len_page_freeslots(page)*page->slot_size;
    fwrite(&freespace, sizeof(freespace), 1, heapfile->file_ptr);
    fflush(heapfile->file_ptr);
}

void read_page(Heapfile *heapfile, PageID pid, Page *page) {
    // calculate the page offset
    int data_page_offset = pid / number_of_pages_per_directory_page(heapfile->page_size) * (number_of_pages_per_directory_page(heapfile->page_size)+1)
            * heapfile->page_size + pid % number_of_pages_per_directory_page(heapfile->page_size) * 8 + 4;
    int real_offset = 0;
    fseek(heapfile->file_ptr, data_page_offset, SEEK_SET);
    fread(&real_offset, sizeof(int), 1, heapfile->file_ptr);
    init_fixed_len_page(page, heapfile->page_size, ATTR_COUNT * ATTR_LEN);
    fseek(heapfile->file_ptr, real_offset, SEEK_SET);
    fread(page->data, heapfile->page_size, 1, heapfile->file_ptr);
}

RecordIterator::RecordIterator(Heapfile *heapfile) {
    Page *page = new Page();
    page->page_size = heapfile->page_size;
    char *buf = new char[page->page_size];
    memset(buf,0, page->page_size);
    page->data = buf;
    fseek(heapfile->file_ptr, heapfile->page_size, SEEK_SET);
    fread(page->data, page->page_size, 1,heapfile->file_ptr);
    int slot_size = 0;
    memcpy(&slot_size, (char *)page->data + page->page_size - 4, sizeof(slot_size));
    page->slot_size = slot_size;
    this->heapfile = heapfile;
    this->curPage = page;
    Page *directory = new Page();
    page->page_size = heapfile->page_size;
    char *buf_dir = new char[page->page_size];
    directory->data = buf_dir;
    this->curDirectory = directory;
    fseek(heapfile->file_ptr, 0, SEEK_SET);
    fread(this->curDirectory->data, page->page_size, 1,heapfile->file_ptr);
    this->curDirectoryOffset = 0;
    this->curPid = 0;
    this->curSlot = -1;
}

bool RecordIterator::hasNext() {
    bool find_next = false;
    while(!find_next){
        // if this is the last page of this directory, and the last slot of this page, move to the next directory
        if((this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size) == number_of_pages_per_directory_page(this->heapfile->page_size)-1)
        && (curSlot == fixed_len_page_capacity(this->curPage)-1)) {
            // Does the next directory exist?
            int next_directory_page_relative_offset = 0;
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset, SEEK_SET);
            fread(&next_directory_page_relative_offset, sizeof(int), 1, this->heapfile->file_ptr);
            // if the next directory does not exist, return
            if (next_directory_page_relative_offset == 0){
                return find_next;
            }
            // if the next directory exists
            this->curDirectoryOffset += next_directory_page_relative_offset;
            this->curPid += 1;
            this->curSlot = -1;
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset, SEEK_SET);
            fread(this->curDirectory->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);
            // check if the next page exists
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + 4, SEEK_SET);
            int freespace = 0;
            fread(&freespace, sizeof(int), 1, this->heapfile->file_ptr);
            if(freespace == -1){
                return find_next;
            }
            // if the next page exists, update curPage
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4, SEEK_SET);
            int data_page_offset = 0;
            fread(&data_page_offset, sizeof(int), 1, this->heapfile->file_ptr);
            fseek(this->heapfile->file_ptr, data_page_offset, SEEK_SET);
            fread(this->curPage->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);

            // if this is the last slot of this page and this page is not the max in the same directory, move to the next page in the same directory
        }else if(this->curSlot == fixed_len_page_capacity(this->curPage)-1 && this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size) != number_of_pages_per_directory_page(this->heapfile->page_size)-1){
            this->curSlot = -1;
            this->curPid += 1;
            // check if next page exists
            int entry_index_in_this_directory = this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size);
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8 + 4, SEEK_SET);
            int freespace = 0;
            fread(&freespace, sizeof(int), 1, this->heapfile->file_ptr);
            if(freespace == -1){
                return find_next;
            }
            // if the next page exists, update curPage
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8, SEEK_SET);
            int data_page_offset = 0;
            fread(&data_page_offset, sizeof(int), 1, this->heapfile->file_ptr);
            fseek(this->heapfile->file_ptr, data_page_offset, SEEK_SET);
            fread(this->curPage->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);
        }else{
            while(this->curSlot < fixed_len_page_capacity(this->curPage) - 1){
                this->curSlot += 1;
                if(check_slot_occupied(this->curPage,this->curSlot)){
                    find_next = true;
                    return find_next;
                }
            }
        }

//        // if this is the last slot of this page and this page is not the max in the same directory
//        if(this->curSlot == fixed_len_page_capacity(this->curPage)-1 && this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size) != number_of_pages_per_directory_page(this->heapfile->page_size)-1) {
//            this->curSlot = -1;
//            this->curPid += 1;
//            // check if next page exists
//            int entry_index_in_this_directory = this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size);
//            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8 + 4, SEEK_SET);
//            int freespace = 0;
//            fread(&freespace, sizeof(int), 1, this->heapfile->file_ptr);
//            if(freespace == -1){
//                return find_next;
//            }
//            // if the next page exists, update curPage
//            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8, SEEK_SET);
//            int data_page_offset = 0;
//            fread(&data_page_offset, sizeof(int), 1, this->heapfile->file_ptr);
//            fseek(this->heapfile->file_ptr, data_page_offset, SEEK_SET);
//            fread(this->curPage->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);
//        }

//        while(this->curSlot < fixed_len_page_capacity(this->curPage) - 1){
//            this->curSlot += 1;
//            if(check_slot_occupied(this->curPage,this->curSlot)){
//                find_next = true;
//                return find_next;
//            }
//        }
    }
    return false;
}

Record RecordIterator::next() {
    Record *record = new Record();
    read_fixed_len_page(this->curPage, this->curSlot, record);
    return *record;
}

FreeSlotIterator::FreeSlotIterator(Heapfile *heapfile) {
    Page *page = new Page();
    page->page_size = heapfile->page_size;
    char *buf = new char[page->page_size];
    memset(buf,0, page->page_size);
    page->data = buf;
    fseek(heapfile->file_ptr, heapfile->page_size, SEEK_SET);
    fread(page->data, page->page_size, 1,heapfile->file_ptr);
    int slot_size = 0;
    memcpy(&slot_size, (char *)page->data + page->page_size - 4, sizeof(slot_size));
    page->slot_size = slot_size;
    this->heapfile = heapfile;
    this->curPage = page;
    Page *directory = new Page();
    page->page_size = heapfile->page_size;
    char *buf_dir = new char[page->page_size];
    directory->data = buf_dir;
    this->curDirectory = directory;
    fseek(heapfile->file_ptr, 0, SEEK_SET);
    fread(this->curDirectory->data, page->page_size, 1,heapfile->file_ptr);
    this->curDirectoryOffset = 0;
    this->curPid = 0;
    this->curSlot = -1;
}

int FreeSlotIterator::hasNext() {
    int find_next = 0;
    while(!find_next){
        // if this is the last page of this directory, move to the next directory
        if((this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size) == number_of_pages_per_directory_page(this->heapfile->page_size)-1)
           && (curSlot == fixed_len_page_capacity(this->curPage)-1)) {
            // Does the next directory exist?
            int next_directory_page_relative_offset = 0;
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset, SEEK_SET);
            fread(&next_directory_page_relative_offset, sizeof(int), 1, this->heapfile->file_ptr);
            // if the next directory does not exist, return
            if (next_directory_page_relative_offset == 0){
                find_next = -2;
                return find_next;
            }
            // if the next directory exists
            this->curDirectoryOffset += next_directory_page_relative_offset;
            this->curPid += 1;
            this->curSlot = -1;
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset, SEEK_SET);
            fread(this->curDirectory->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);
            // check if the next page exists
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + 4, SEEK_SET);
            int freespace = 0;
            fread(&freespace, sizeof(int), 1, this->heapfile->file_ptr);
            if(freespace == -1){
                find_next = -1;
                return find_next;
            }
            // if the next page exists, update curPage
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4, SEEK_SET);
            int data_page_offset = 0;
            fread(&data_page_offset, sizeof(int), 1, this->heapfile->file_ptr);
            fseek(this->heapfile->file_ptr, data_page_offset, SEEK_SET);
            fread(this->curPage->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);

            // if this is the last slot of this page and this page is not the max in the same directory, move to the next page in the same directory
        }else if(this->curSlot == fixed_len_page_capacity(this->curPage)-1 && this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size) != number_of_pages_per_directory_page(this->heapfile->page_size)-1){
            this->curSlot = -1;
            this->curPid += 1;
            // check if next page exists
            int entry_index_in_this_directory = this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size);
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8 + 4, SEEK_SET);
            int freespace = 0;
            fread(&freespace, sizeof(int), 1, this->heapfile->file_ptr);
            if(freespace == -1){
                find_next = -1;
                return find_next;
            }
            // if the next page exists, update curPage
            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8, SEEK_SET);
            int data_page_offset = 0;
            fread(&data_page_offset, sizeof(int), 1, this->heapfile->file_ptr);
            fseek(this->heapfile->file_ptr, data_page_offset, SEEK_SET);
            fread(this->curPage->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);

        }else{
            while(this->curSlot < fixed_len_page_capacity(this->curPage) - 1){
                this->curSlot += 1;
                if(check_slot_occupied(this->curPage,this->curSlot) == 0){
                    find_next = 1;
                    return find_next;
                }
            }
        }

//        // if this is the last slot of this page and this page is not the max in the same directory
//        if(this->curSlot == fixed_len_page_capacity(this->curPage)-1) {
//            this->curSlot = -1;
//            this->curPid += 1;
//            // check if next page exists
//            int entry_index_in_this_directory = this->curPid % number_of_pages_per_directory_page(this->heapfile->page_size);
//            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8 + 4, SEEK_SET);
//            int freespace = 0;
//            fread(&freespace, sizeof(int), 1, this->heapfile->file_ptr);
//            if(freespace == -1){
//                find_next = -1;
//                return find_next;
//            }
//            // if the next page exists, update curPage
//            fseek(this->heapfile->file_ptr, this->curDirectoryOffset + 4 + entry_index_in_this_directory*8, SEEK_SET);
//            int data_page_offset = 0;
//            fread(&data_page_offset, sizeof(int), 1, this->heapfile->file_ptr);
//            fseek(this->heapfile->file_ptr, data_page_offset, SEEK_SET);
//            fread(this->curPage->data, this->heapfile->page_size, 1, this->heapfile->file_ptr);
//        }

//        while(this->curSlot < fixed_len_page_capacity(this->curPage) - 1){
//            this->curSlot += 1;
//            if(check_slot_occupied(this->curPage,this->curSlot) == 0){
//                find_next = 1;
//                return find_next;
//            }
//        }
    }
    return 0;
}

vector<int> FreeSlotIterator::next() {
    vector<int> ret;
    int data_page_offset_in_heapfile = (this->curPid / number_of_pages_per_directory_page(this->heapfile->page_size)
            * (number_of_pages_per_directory_page(this->heapfile->page_size + 1)) + 1 + this->curPid %
            number_of_pages_per_directory_page(this->heapfile->page_size))* this->heapfile->page_size;
    int free_slot_offset_in_heapfile = data_page_offset_in_heapfile + this->curSlot * ATTR_COUNT * ATTR_LEN;
    // find the directory containing this page's entry
    int directory_offset = this->curPid / number_of_pages_per_directory_page(heapfile->page_size)
                           * (number_of_pages_per_directory_page(heapfile->page_size)+1)
                           * heapfile->page_size; // start from 0
    // find the relative offset of the data page in the directory page
    int data_page_relative_offset_in_directory = this->curPid % number_of_pages_per_directory_page(heapfile->page_size) * 8; // start from 0
    int free_space_offset_in_heapfile = directory_offset + 4 + data_page_relative_offset_in_directory + 4;
    ret.push_back(data_page_offset_in_heapfile);
    ret.push_back(free_space_offset_in_heapfile);

    return ret;
}


