Outline
=========================

###Compression

Compression information is stored as an array in the file header. Each element of the array is a tuple containing the length of the file segment and the method by which it's compressed. Roughly 20 or so compression algorithms will be used in the final version, simple enough that each chunk decompression would be virtually instantaneous. (This will be important later.)

The file would be compressed by creating a file called "new", looking up each tuple of the compression array, sectioning off that number of bytes into its own file, applying the necessary compression code to that file, and appending it to the "new" file. Again, we're talking stupidly simple, crappy compression.

Of course, the length of the compressed segment won't be the same as the original length. This means that the length of the segment in the tuple would be edited before the new file is appended to the new file and deleted for good.

The finished file is then included with the compression array as a header.
    
###Decompression
  
The header (probably suffixed with a string for program recognition) is removed from the compressed file and copied over to its own file. It's then used as a reference guide as the compression process happens in reverse. First, we create a "decompressed" file. For an array element of [len, algo], say, we'd copy the first len bytes into a new file, decompress that file according the algorithm corresponding to algo, append the new file to "decompressed" and then delete the first len bytes from the compressed file. Rinse and repeat.
    
###Evolution

Here's where things get fun. We begin our process with a population of a buch of a certain type of test file to be compressed. Then, we use a standard genetic algorithm. We create an initial population of 10,000 random arrays (Hey, we have time), and use these arrays as the basis for compression. A test of an array would consist of using it to compress each file, and averaging the compression scores. (Compressed/Uncompressed). The final compression score, factoring in the time (somehow) would be the fitness.

We then "mate" the arrays. In order to mate two arrays, we swap randomly chosen, randomly sized collections of each array, generating two offspring. It's important that the original arrays aren't changed. There's also a small (1%, probably) chance that a tuple's compression method will randomly change in a child. For the 5,000 mates required for 10,000 offspring, we'd choose 2 arrays at random for each mate. Arrays with higher fitness have a better shot at getting picked.

###How will the final application work?

After running this evolution, we take the 5 best arrays and store them in a file for this type. The application compresses the sample file with all 5 arrays, picks the smallest one, and spits it out. Yay!
