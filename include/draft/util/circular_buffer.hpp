#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <algorithm>

namespace Draft {
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

        ~CircularBuffer(){
            delete [] ptr;
        }

        // Functions
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

        inline unsigned long length() const { return len; }

        // Operators
        inline T& operator[](int index){ return ptr[(start + index) % len]; }
    };
};

#endif