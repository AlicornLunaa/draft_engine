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
