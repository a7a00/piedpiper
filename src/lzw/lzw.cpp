//An implementation of the LZM algorithm in C++.
//I got so much help for this, it's not even funny.
//Seriously, if you want a good example of my code, don't look here. It's good code, but what I write doesn't look like this.

#include <string>
#include <exception>
#include <ostream>
#include <memory>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <fstream>
#include <ios>
#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <iostream>
#include <istream>
#include <limits>

using CharValue = std::uint32_t; //Suggested by the helpful people on the C++ forums.

namespace constants //To be used throughout the whole program.
{
  const CharValue maxSize {512 * 1024};
}

enum class MetaCode: CharValue
{
  Eof = 1u << CHAR_BIT,   
};

class EncoderDictionary
{
    struct Node
    {
      explicit Node(char c): first(constants::maxSize), c(c), left(constants::maxSize), right(constants::maxSize) {}

        CharValue    first;
        char        c;  
        CharValue    left;
        CharValue    right;
    };

  public:
      EncoderDictionary()
      {
          const long int minc = std::numeric_limits<char>::min();
          const long int maxc = std::numeric_limits<char>::max();
          CharValue k {0};

          for (long int c = minc; c <= maxc; ++c)
              initials[static_cast<unsigned char> (c)] = k++;

          vn.reserve(constants::maxSize);
          reset();
      }

      void reset()
      {
          vn.clear();

          const long int minc = std::numeric_limits<char>::min();
          const long int maxc = std::numeric_limits<char>::max();

          for (long int c = minc; c <= maxc; ++c)
              vn.push_back(Node(c));
          vn.push_back(Node('\x00'); //Nodes for the end of the file.
      }

      CharValue search_and_insert(CharValue i, char c)
      {
          if (i == constants::maxSize)
              return search_initials(c);

          const CharValue vn_size = vn.size();
          CharValue ci {vn[i].first}; // Current Index

          if (ci != constants::maxSize)
          {
              while (true)
                  if (c < vn[ci].c)
                  {
                      if (vn[ci].left == constants::maxSize)
                      {
                          vn[ci].left = vn_size;
                          break;
                      }
                      else
                          ci = vn[ci].left;
                  }
                  else
                  if (c > vn[ci].c)
                  {
                      if (vn[ci].right == constants::maxSize)
                      {
                          vn[ci].right = vn_size;
                          break;
                      }
                      else
                          ci = vn[ci].right;
                  }
                  else
                      return ci;
          }
          else
              vn[i].first = vn_size;

        vn.push_back(Node(c));
        return constants::maxSize;
    }

    CharValue search_initials(char c) const
    {
        return initials[static_cast<unsigned char> (c)];
    }

    std::vector<Node>::size_type size() const
    {
        return vn.size();
    }

private:

    std::vector<Node> vn;
    std::array<CharValue, 1u << CHAR_BIT> initials;
};

struct ByteCache {

    ByteCache(): used(0), data(0x00)
    {
    }

    std::size_t     used;
    unsigned char   data;
};

class CodeWriter {
public:

    explicit CodeWriter(std::ostream &os): os(os), bits(CHAR_BIT + 1)
    {
    }
    ~CodeWriter()
    {
        write(static_cast<CharValue> (MetaCode::Eof));
        if (lo.used != 0)
            os.put(static_cast<char> (lo.data));
    }

    std::size_t get_bits() const
    {
        return bits;
    }

    void reset_bits()
    {
        bits = CHAR_BIT + 1;
    }

    void increase_bits()
    {
        if (bits == SIZE_MAX)
            throw std::overflow_error("CodeWriter::increase_bits()");
        ++bits;
    }

    bool write(CharValue k) //Returns whether we can keep using the stream to output files.
    {
        std::size_t remaining_bits {bits};

        if (lo.used != 0)
        {
            lo.data |= k << lo.used;
            os.put(static_cast<char> (lo.data));
            k >>= CHAR_BIT - lo.used;
            remaining_bits -= CHAR_BIT - lo.used;
            lo.used = 0;
            lo.data = 0x00;
        }

        while (remaining_bits != 0)
            if (remaining_bits >= CHAR_BIT)
            {
                os.put(static_cast<char> (k));
                k >>= CHAR_BIT;
                remaining_bits -= CHAR_BIT;
            }
            else
            {
                lo.used = remaining_bits;
                lo.data = k;
                break;
            }

        return os;
    }

private:

    std::ostream    &os;
    std::size_t     bits;
    ByteCache       lo;
};

class CodeReader {
public:

    explicit CodeReader(std::istream &is): is(is), bits(CHAR_BIT + 1), feofmc(false)
    {
    }

    std::size_t get_bits() const
    {
        return bits;
    }

    void reset_bits()
    {
        bits = CHAR_BIT + 1;
    }

    void increase_bits()
    {
        if (bits == SIZE_MAX)
            throw std::overflow_error("CodeReader::increase_bits()");
        ++bits;
    }

    bool read(CharValue &k) //This method returns whether we can still use this as an input stream.
    {
        static const std::array<unsigned long int, 9> masks {
            {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF}
        };

        std::size_t remaining_bits {bits};
        std::size_t offset {lo.used};
        unsigned char temp;

        k = lo.data;
        remaining_bits -= lo.used;
        lo.used = 0;
        lo.data = 0x00;

        while (remaining_bits != 0 && is.get(reinterpret_cast<char &> (temp)))
            if (remaining_bits >= CHAR_BIT)
            {
                k |= static_cast<CharValue> (temp) << offset;
                offset += CHAR_BIT;
                remaining_bits -= CHAR_BIT;
            }
            else
            {
                k |= static_cast<CharValue> (temp & masks[remaining_bits]) << offset;
                lo.used = CHAR_BIT - remaining_bits;
                lo.data = temp >> remaining_bits;
                break;
            }

        if (k == static_cast<CharValue> (MetaCode::Eof))
        {
            feofmc = true;
            return false;
        }

        return is;
    }

    bool corrupted() const //Did we fuck up?
    {
        return !feofmc;
    }

private:

    std::istream    &is;
    std::size_t     bits;
    bool            feofmc;
    ByteCache       lo;
};

std::size_t required_bits(unsigned long int n) //How many bits do we need?
{
    std::size_t r {1};

    while ((n >>= 1) != 0)
        ++r;

    return r;
}

void compress(std::istream &is, std::ostream &os)
{
    EncoderDictionary ed;
    CodeWriter cw(os);
    CharValue i {constants::maxSize};
    char c;
    bool rbwf {false};

    while (is.get(c))
    {
        if (ed.size() == constants::maxSize)
        {
            ed.reset();
            rbwf = true;
        }

        const CharValue temp {i};

        if ((i = ed.search_and_insert(temp, c)) == constants::maxSize)
        {
            cw.write(temp);
            i = ed.search_initials(c);

            if (required_bits(ed.size() - 1) > cw.get_bits())
                cw.increase_bits();
        }

        if (rbwf)
        {
            cw.reset_bits();
            rbwf = false;
        }
    }

    if (i != constants::maxSize)
        cw.write(i);
}

void decompress(std::istream &is, std::ostream &os)
{
    std::vector<std::pair<CharValue, char>> dictionary;

    // "named" lambda function, used to reset the dictionary to its initial contents
    const auto reset_dictionary = [&dictionary] {
        dictionary.clear();
        dictionary.reserve(constants::maxSize);

        const long int minc = std::numeric_limits<char>::min();
        const long int maxc = std::numeric_limits<char>::max();

        for (long int c = minc; c <= maxc; ++c)
            dictionary.push_back({constants::maxSize, static_cast<char> (c)});
        dictionary.push_back({0, '\x00'});
    };

    const auto rebuild_string = [&dictionary](CharValue k) -> const std::vector<char> * {
        static std::vector<char> s;

        s.clear();
        s.reserve(constants::maxSize);

        while (k != constants::maxSize)
        {
            s.push_back(dictionary[k].second);
            k = dictionary[k].first;
        }

        std::reverse(s.begin(), s.end());
        return &s;
    };

    reset_dictionary();

    CodeReader cr(is);
    CharValue i {constants::maxSize}; // Index
    CharValue k; // Key

    while (true)
    {
        if (dictionary.size() == constants::maxSize)
        {
            reset_dictionary();
            cr.reset_bits();
        }

        if (required_bits(dictionary.size()) > cr.get_bits())
            cr.increase_bits();

        if (!cr.read(k))
            break;

        if (k > dictionary.size())
            throw std::runtime_error("invalid compressed code");

        const std::vector<char> *s; // String

        if (k == dictionary.size())
        {
            dictionary.push_back({i, rebuild_string(i)->front()});
            s = rebuild_string(k);
        }
        else
        {
            s = rebuild_string(k);

            if (i != constants::maxSize)
                dictionary.push_back({i, s->front()});
        }

        os.write(&s->front(), s->size());
        i = k;
    }

    if (cr.corrupted())
        throw std::runtime_error("corrupted compressed file");
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        print_usage("Wrong number of arguments.");
        return EXIT_FAILURE;
    }

    enum class Mode {
        Compress,
        Decompress
    };

    Mode m;

    if (std::string(argv[1]) == "-c")
        m = Mode::Compress;
    else
    if (std::string(argv[1]) == "-d")
        m = Mode::Decompress;
    else
    {
        print_usage(std::string("flag `") + argv[1] + "' is not recognized.");
        return EXIT_FAILURE;
    }

    const std::size_t buffer_size {1024 * 1024};
    const std::unique_ptr<char[]> input_buffer(new char[buffer_size]);
    const std::unique_ptr<char[]> output_buffer(new char[buffer_size]);

    std::ifstream input_file;
    std::ofstream output_file;

    input_file.rdbuf()->pubsetbuf(input_buffer.get(), buffer_size);
    input_file.open(argv[2], std::ios_base::binary);

    if (!input_file.is_open())
    {
        print_usage(std::string("input_file `") + argv[2] + "' could not be opened.");
        return EXIT_FAILURE;
    }

    output_file.rdbuf()->pubsetbuf(output_buffer.get(), buffer_size);
    output_file.open(argv[3], std::ios_base::binary);

    if (!output_file.is_open())
    {
        print_usage(std::string("output_file `") + argv[3] + "' could not be opened.");
        return EXIT_FAILURE;
    }

    try
    {
        input_file.exceptions(std::ios_base::badbit);
        output_file.exceptions(std::ios_base::badbit | std::ios_base::failbit);

        if (m == Mode::Compress)
            compress(input_file, output_file);
        else
        if (m == Mode::Decompress)
            decompress(input_file, output_file);
    }
    catch (const std::ios_base::failure &f)
    {
        print_usage(std::string("File input/output failure: ") + f.what() + '.', false);
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        print_usage(std::string("Caught exception: ") + e.what() + '.', false);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
