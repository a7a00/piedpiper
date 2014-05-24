Pseudocode

  Compression
    Compression information is stored as an array in the file header. Each element of the array is a tuple containing the length of the file segment and the method by which it's compressed. Roughly 20 or so compression algorithms will be used in the final version, simple enough that each chunk decompression would be virtually instantaneous. (This will be important later.)
    The file would be compressed by creating a file called "new", looking up each tuple of the compression array, sectioning off that number of bytes into its own file, applying the necessary compression code to that file, and appending it to the "new" file. Again, we're talking stupidly simple, crappy compression.
    Of course, the length of the compressed segment won't be the same as the original length. This means that the length of the segment in the tuple would be edited before the new file is appended to the new file and deleted for good.
    The finished file is then included with the compression array as a header.
    
  Decompression
    The header (probably suffixed with a string for program recognition) is removed from the compressed file and copied over to its own file. It's then used as a reference guide as the compression process happens in reverse. First, we create a "decompressed" file. For an array element of [len, algo], say, we'd copy the first len bytes into a new file, decompress that file according to algo, append the new file to "decompressed" and then delete the first len bytes from the compressed file. Rinse and repeat.
    
  Evolution
    
