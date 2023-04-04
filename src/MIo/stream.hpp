#ifndef STREAM_HPP
#define STREAM_HPP
#include <bits/types/FILE.h>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <new>
#include <string>

namespace mgk {

    class endl_t {};

    namespace format {
        struct hex
        {
            uint64_t x;
        };

        struct oct
        {
            uint64_t x;
        };
    }

    static const endl_t endl{};

    const size_t MAX_NUMBER_WIDTH = 32;
    const size_t BUFFER_MIN_CAPACITY = MAX_NUMBER_WIDTH;

    class OTextStream
    {

    protected:
        virtual void write(const void* data, size_t n)  = 0;

    public:
        OTextStream(size_t capacity = 4096) : bufferCapacity_(capacity)
        {
            if(bufferCapacity_)
            {
                bufferCapacity_ = std::max(capacity, BUFFER_MIN_CAPACITY);
                buffer_ = new(std::nothrow) char[bufferCapacity_];
                if(!buffer_) bufferCapacity_ = 0;
            }
        }

        virtual ~OTextStream() 
        {
            flush();
            delete [] buffer_;
            bufferSize_ = 0;
            bufferCapacity_ = 0;
        }

        // Non-copyable
        OTextStream(const OTextStream& oth)            = delete;
        OTextStream& operator=(const OTextStream& oth) = delete;

        // Movable
        OTextStream(OTextStream&& oth)            = default;
        OTextStream& operator=(OTextStream&& oth) = default;

        OTextStream& operator<<(bool c)              ;
        OTextStream& operator<<(char c)              ;
        OTextStream& operator<<(uint32_t x)          ;
        OTextStream& operator<<(int32_t x)           ;
        OTextStream& operator<<(int64_t x)           ;
        OTextStream& operator<<(uint64_t x)          ;
        OTextStream& operator<<(format::hex x)       ;
        OTextStream& operator<<(format::oct x)       ;
        OTextStream& operator<<(std::nullptr_t x)    ;
        OTextStream& operator<<(const char* s)       ;
        OTextStream& operator<<(const void* s)       ;
        OTextStream& operator<<(const std::string& s);
        OTextStream& operator<<(endl_t)              ;

        void flush();
    private:
        enum NumFlags : uint64_t
        {
            None        = 0x000,
            Signed      = 0x001,
            ForcedSign  = 0x002,
            Hex         = 0x004,
            Prefix      = 0x008,
            ZeroFill    = 0x010,
            Ptr         = 0x020,
            AllignLeft  = 0x040,
            Oct         = 0x080,
            SignedSpace = 0x100,
        };

        void putChar(char c);
        void putString(const char* string, size_t len);
        void putNumber(uint64_t number, uint64_t numFlags);
        char* buffer_ = nullptr;
        size_t bufferSize_ = 0;
        size_t bufferCapacity_ = 0;
    };

    class ofstream : public OTextStream
    {
    public:
        ofstream(size_t capacity = 4096) : OTextStream(capacity) {}
        ofstream(FILE* file, size_t capacity = 4096) : OTextStream(capacity), file_(file), destruct_file(false) {}
        
        ofstream(const ofstream&) = delete;
        ofstream& operator=(const ofstream&) = delete;

        virtual ~ofstream() override 
        {
            flush();
            close();
        }

        bool open(const char* filename);
        void close();
    private:
        FILE* file_ = nullptr;
        bool destruct_file = true;
        virtual void write(const void *data, size_t n) override;
    };

    extern ofstream out;


    //====================< istream >========================
    class istream
    {

    protected:
        virtual ssize_t read(const void* data, size_t n)  = 0;

    public:
        istream(size_t capacity = 4096) : bufferCapacity_(capacity)
        {
            bufferCapacity_ = std::max(capacity, BUFFER_MIN_CAPACITY);
            buffer_ = new char[bufferCapacity_];
        }


        istream(const istream&) = delete;
        istream& operator=(const istream&) = delete;

        virtual ~istream() 
        {
            delete [] buffer_;
            bufferSize_ = 0;
            bufferCapacity_ = 0;
        }

        // Non-copyable
        istream(const OTextStream& oth)            = delete;
        istream& operator=(const OTextStream& oth) = delete;

        // Movable
        istream(istream&& oth)            = default;
        istream& operator=(istream&& oth) = default;

        istream& operator>>(bool& c)       ;
        istream& operator>>(char& c)       ;
        istream& operator>>(uint32_t& x)   ;
        istream& operator>>(int32_t& x)    ;
        istream& operator>>(int64_t& x)    ;
        istream& operator>>(uint64_t& x)   ;
        istream& operator>>(std::string& s);

        operator bool() {return !eof_;}
    private:
        bool eof_ = false;
        void prepareBlock();
        void putChar(char c);
        void putString(const char* string, size_t len);
        void putNumber(uint64_t number, uint64_t numFlags);
        char* buffer_ = nullptr;
        size_t bufferSize_ = 0;
        size_t bufferCur_ = 0;
        size_t bufferCapacity_ = 0;
    };
}

#endif /* STREAM_HPP */
