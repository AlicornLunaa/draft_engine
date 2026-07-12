#pragma once

#include <algorithm>

namespace Draft {
    /**
     * @brief A fixed-capacity ring buffer, pushing past capacity silently overwrites the
     * oldest element rather than growing.
     */
    template<typename T>
    class CircularBuffer {
    private:
        // Variables
        unsigned long start = 0;
        unsigned long end = 0;
        unsigned long len = 0;
        unsigned long size = 0;
        T* ptr = nullptr;

    public:
        // Constructor
        CircularBuffer(unsigned long size) : size(size) {
            ptr = new T[size];
        }

        CircularBuffer(const CircularBuffer& other) :
            start(other.start), end(other.end), len(other.len), size(other.size),
            ptr(new T[other.size])
        {
            std::copy(other.ptr, other.ptr + size, ptr);
        }

        CircularBuffer(CircularBuffer&& other) noexcept :
            start(other.start), end(other.end), len(other.len), size(other.size),
            ptr(other.ptr)
        {
            other.ptr = nullptr;
            other.start = other.end = other.len = other.size = 0;
        }

        CircularBuffer& operator=(const CircularBuffer& other){
            if(this == &other)
                return *this;

            T* newPtr = new T[other.size];
            std::copy(other.ptr, other.ptr + other.size, newPtr);

            delete[] ptr;
            ptr = newPtr;
            start = other.start;
            end = other.end;
            len = other.len;
            size = other.size;

            return *this;
        }

        CircularBuffer& operator=(CircularBuffer&& other) noexcept {
            if(this == &other)
                return *this;

            delete[] ptr;
            ptr = other.ptr;
            start = other.start;
            end = other.end;
            len = other.len;
            size = other.size;

            other.ptr = nullptr;
            other.start = other.end = other.len = other.size = 0;

            return *this;
        }

        ~CircularBuffer(){
            delete [] ptr;
        }

        // Functions
        /**
         * @brief Pushes @p value onto the buffer, overwriting the oldest element once length()
         * has reached the buffer's capacity.
         */
        void push(T value){
            if(len >= size){
                start++;
            }

            ptr[end] = value;
            end++;
            len = std::min(len + 1, size);

            if(end >= size){
                // If the new ending is past the size of the buffer, do the circle
                end = 0;
            }

            if(start >= size){
                start = 0;
            }
        }

        /**
         * @brief Number of elements currently held, up to the buffer's capacity.
         */
        inline unsigned long length() const { return len; }

        // Operators
        /**
         * @brief Accesses the @p index'th oldest element still held (0 is the oldest).
         */
        inline T& operator[](int index){ return ptr[(start + index) % size]; }
    };
};
