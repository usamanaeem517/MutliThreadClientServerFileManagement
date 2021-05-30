# MutliThreadClientServerFileManagement

## Solution Explanation
<p>A struct is used to store the information about the disk such as disk size, number of cylinders, number of sectors, pointer to the disk. A struct for file system is maintained in which Bitmap is used to keep track of the free blocks. And a struct to maintain information about each directory in the FAT system. A directory struct has info such as files in the directory, number of files in it, and name of directory. A struct for files is also present in directory struct to maintain the file name and the blocks allocated to the file and size of file. Whenever a block is allocated to the file the bit of that block in the Bitmap is set and when it is freed by the file, the block number is freed in the Bitmap and made available for writing. A bitmap is a char * array so that each char can accommodate 8 blocks. </p>


## To Compile:
<pre><code>make </code></pre>

## To execute server:
<pre><code>./server 4 2 disk.txt </code></pre>

## To execute client:
<pre><code>./client</code></pre>
