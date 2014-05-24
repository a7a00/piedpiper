#include <fstream>
using namespace std;

  int main ()
  {

    char * buffer;
    long size;

    ifstream infile ("Hamlet.txt",ifstream::binary);
    ofstream outfile1 ("part1.txt",ofstream::binary);
    infile.seekg(0,ifstream::end);
    size=(infile.tellg())/2;
    infile.seekg(0);
    buffer = new char [size];
    infile.read (buffer,size);
    outfile1.write (buffer,size);
    ofstream outfile2 ("part2.txt",ofstream::binary);
    infile.seekg(size);
    buffer = new char [size];
    infile.read (buffer,size);
    outfile2.write (buffer,size);
    delete[] buffer;
    outfile1.close();
    outfile2.close();
    infile.close();
    return 0;
 }
