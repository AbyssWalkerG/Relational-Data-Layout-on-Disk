
CC=g++ 

all: write_fixed_len_pages read_fixed_len_page csv2heapfile insert delete scan select csv2colstore select2 select3 update

.PHONY: all

OBJS1 = library.o write_fixed_len_pages.o

OBJS2 = library.o read_fixed_len_page.o

OBJS3 = library.o csv2heapfile.o

OBJS4 = library.o insert.o

OBJS5 = library.o delete.o

OBJS6 = library.o scan.o

OBJS7 = library.o select.o

OBJS8 = library.o csv2colstore.o

OBJS9 = library.o select2.o

OBJS10 = library.o select3.o

OBJS11 = library.o update.o

CPP_FLAGS='-std=c++17'

library.o: library.cpp library.h
	$(CC) $(CPP_FLAGS) -c $< -o $@

write_fixed_len_pages.o: write_fixed_len_pages.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

read_fixed_len_page.o: read_fixed_len_page.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

csv2heapfile.o: csv2heapfile.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

insert.o: insert.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

delete.o: delete.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

scan.o: scan.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

select.o: select.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

csv2colstore.o: csv2colstore.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

select2.o: select2.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

select3.o: select3.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

update.o: update.cpp

write_fixed_len_pages: $(OBJS1)
	$(CC) $(CPP_FLAGS) $(OBJS1) -o $@

read_fixed_len_page: $(OBJS2)
	$(CC) $(CPP_FLAGS) $(OBJS2) -o $@

csv2heapfile: $(OBJS3)
	$(CC) $(CPP_FLAGS) $(OBJS3) -o $@

insert: $(OBJS4)
	$(CC) $(CPP_FLAGS) $(OBJS4) -o $@

delete: $(OBJS5)
	$(CC) $(CPP_FLAGS) $(OBJS5) -o $@

scan: $(OBJS6)
	$(CC) $(CPP_FLAGS) $(OBJS6) -o $@

select: $(OBJS7)
	$(CC) $(CPP_FLAGS) $(OBJS7) -o $@

csv2colstore: $(OBJS8)
	$(CC) $(CPP_FLAGS) $(OBJS8) -o $@

select2: $(OBJS9)
	$(CC) $(CPP_FLAGS) $(OBJS9) -o $@

select3: $(OBJS10)
	$(CC) $(CPP_FLAGS) $(OBJS10) -o $@

update: $(OBJS11)
	$(CC) $(CPP_FLAGS) $(OBJS11) -o $@

.PHONY: clean

clean: 
	rm $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(OBJS7) $(OBJS8) $(OBJS9) $(OBJS10) $(OBJS11)