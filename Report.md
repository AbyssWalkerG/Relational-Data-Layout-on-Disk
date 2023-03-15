# Report

## Team Member

Jinchen

Xiaochen
<br/><br>
## Compile
Enter the directory where `Makefile` resides.

        $ make
The executable files are `write_fixed_len_pages` and `read_fixed_len_page`.

The program is tested on MacOS.
## Usage

        $ write_fixed_len_pages <csv_file> <page_file> <page_size> 
        $ read_fixed_len_page <page_file> <page_size>
        $ csv2heapfile <csv_file> <heapfile> <page_size>
        $ scan <heapfile> <page_size>
        $ insert <heapfile> <csv_file> <page_size>
        $ update <heapfile> <record_id> <attribute_id> <new_value> <page_size>
        $ delete <heapfile> <record_id> <page_size>
        $ select <heapfile> <attribute_id> <start> <end> <page_size>
        $ csv2colstore <csv_file> <colstore_name> <page_size>
        $ select2  <colstore_name> <attribute_id> <start> <end> <page_size>
        $ select3  <colstore_name> <attribute_id> <return_attribute_id> <start> <end> <page_size>
The attribute size and number are set as constants in `library.h`, and the default values are 10 and 100. Please change the values if tested by different schema. 
<br/><br>

## 2.1 Experiments

We assume that there is only one table schema. There are 100 attributes, and each attribute is 10 bytes each. So, records in the table are fixed length.

* Calculate the size of fixed length serialization of records in the table.

    Because each attribute is 10 bytes and there are 100 attributes in one record, the length of the record should be the concatenation of all the attributes, which is 1000 bytes. 
    <br/><br>

* Use `fixed_len_sizeof()` to check if it agrees with your calculation.
    
    The result of `fixed_len_sizeof()` is 1000, which matches the calculation. 
    <br/><br>

* Calculate the size of variable length serialization of the records.

    Since we have to delineate the boudaries of each attributes in variable length records, an `unsigned short` is used for representing the number of attributes and the offset for each attribute. So, there are 101 `unsigned short` apart from the space of attributes. The size of the record is going to be 1202 bytes. 
    <br/><br>

* Use `var_len_sizeof()` to check if it agrees with your calculation.

    The result of `var_len_sizeof()` is 1202, which matches the calculation. 
    <br/><br>

## 3.2 Experiments

* Plot the performance (records / second) versus page size for write and read. 

    ![](./performance_page.png 'performance_page')
    Running `write_fixed_len_pages` and `read_fixed_len_page` for each page size for 5 times to get the average performance. 
    <br/><br>

* Compare this to the blocked disk I/O characteristics you observed in the tutorial.
    
    In the tutorial, pages are stored as heap file, which means that they are separated over the disk and have different addresses. In that case, accessing each page needs an I/O on heap file and another I/O on the page file, which requires less I/O operations as the page size increases (i.e. the page number decreases). But in this case, all pages are in the same page file, which means that only one I/O is needed for locate the page. So for different page sizes (numbers), the performances seem to be similar. 
      <br/><br>

* Discuss why page-based format is superior to storing records using a CSV file.

    Page-based format allows for faster access to data because it reduces the number of I/O operations needed to retrieve or modify data. In contrast, CSV files require scanning the entire file to retrieve a record, which can be slow for large datasets. Besides, page-based format allows for efficient data partitioning and distribution across multiple media. CSV files are limited by the storage capacity of a single file. 
      <br/><br>

* Discuss the shortcomings of the way we organize pages.
    
    If we organize pages in a page file, we don't have knowledge of how much free space each page in the page file has, and the only way to gain this knowledge is to traverse all the pages till we find one proper free space for operations such as insertions. Besides, if the data is large enough, the single page file can be too large to fit in certain storage media. However, because it is a single complete file, it cannot be placed in different locations, which is a problem. 

## 4.3 Experiments

* Measure the performance of `csv2heapfile`, comment on how the page size affects the performance of load.

    Larger the `Page Size` is, shorter time the program takes to finish. Since lager the `Page Size` is,the number of directory pages and data pages will be less, then less time is needed for initializing these pages. Along with the `Page Size` increasing to `1MB`, the performance seems to reach a relatively stable status. This is because the expense of intializing a small number of pages is not significant compared to the expense of writing all the data to disk. The test csv file has 10000 records with 100 attributes each and the performance is shown in the figure below.
    ![](./performance_csv2heapfile.png 'performanc_csv2heapfile_')
    <br/><br> 

* Measure the performance of the query versus page size.

    We tested the query performance for query (i.e. `select`) with respect to `Page Size` using 10000 records. For each `Page Size`, we tested three different select ranges `A to H`, `A to P` and `A to Z`. The performance is shown as below in the figure.
    ![](./performance_select.png 'performance_select')
    <br/><br>

* Comment on the choice of page size and the effects of the range from start and end on the performance of the query.
    
    We can see from the results that if we choose larger page size, the runtime will be shorter, implying better performance. And if the range is smaller, the performance will be roughly better. 
    Because larger page size means that the times for reading a page are fewer and smaller range means fewer records to be processed for displaying the output. 



## 5.2 Experiments

* Measure the performance of `csv2colstore` against different page sizes.

    The performance of `csv2colstore` is shown as below in the figure. 
    ![](./performance_csv2colstore.png 'performance_csv2colstore')
    <br/><br>

* Compare the result with that of `csv2heapfile` in the previous section. Comment on the difference.

    The average runtime of `csv2clostore` increases expontentially at first as the `Page Size` increases, and increases slow as page number decreases to a relatively small number. When `Page Size` is small, because `csv2colstore` only store an attribute in a heapfile, this results in that each page can hold more records so that the number of pages is small too, which accounts for its better performance than `csv2heapfile`. But when `Page Size` is large, the expense of creating and writing these pages for all the attributes in a bunch of heapfiles is significantly higher than just creating and writing pages in one heap file. 
    <br/><br>

* Compare the performance of `select2` with that of `select` in the previous section. Comment on the difference.

    The performance of `select2` is shown in the figure below. `select2` uses significantly less time when querying a single attribute than `select`. That is because `select` reads the whole complete records from the disk but `select2` only reads the single attribute it needs. In consideration of the `Page Size`, it seems that a negative correlation exists between `Page Size` and the runtime in `select2`, but it is not significant. And same as what has been obeserved in `select`, larger range results in more runtime for `select2`. 
    ![](./performance_select23.png 'performance_select23')
    <br/><br>

* Compare the performance of `select3` with that of `select` and `select2`. Comment on the difference especially with respect to different selection ranges (different values of start and end).

    The performance of `select3` is shown in the previous figure. `select3` is much like `select2` in contrast to `select`, such as that slower queries for a larger range, and that negative correlation between `Page Size` and runtime. But in general, `select3` takes more time to finish than `select2` does since it has to read from two separate heapfiles. 
