# MutliThreadClientServerFileManagement

## Solution Explanation
<p>The disk is basically a file which is mapped using mmap(2) system call. When the user gives the file name then it is created if not already present and mapped using mmap call. It returns the pointer to the mapped file which is used for writing and reading data on it. So that file acts as a disk. In this way we are actually using the real storage space using file. So, whatever is written using the pointer returned by mmap that is written in actual file/ actual storage.</p></br>
<p>A struct is used to store the information about the disk such as disk size, number of cylinders, number of sectors, pointer to the disk. A struct for file system is maintained in which Bitmap is used to keep track of the free blocks. And a struct to maintain information about each directory in the FAT system. A directory struct has info such as files in the directory, number of files in it, and name of directory. A struct for files is also present in directory struct to maintain the file name and the blocks allocated to the file and size of file. Whenever a block is allocated to the file the bit of that block in the Bitmap is set and when it is freed by the file, the block number is freed in the Bitmap and made available for writing. A bitmap is a char * array so that each char can accommodate 8 blocks. </p>


## To Compile:
<pre><code>make </code></pre>

## To execute server:
<pre><code>./server 4 2 disk.txt </code></pre>

## To execute client:
<pre><code>./client</code></pre>
