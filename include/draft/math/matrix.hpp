#pragma once

#include <algorithm>
#include <cstddef>

#include "draft/math/vector2.hpp"
#include "draft/math/vector3.hpp"
#include "draft/math/vector4.hpp"

namespace Draft {
    // Primary template
    template<typename T, size_t rows, size_t cols>
    class Matrix {
    private:
        // Variables
        const Vector2u size = { rows, cols };
        T array[rows][cols]{};

    public:
        // Constructors
        Matrix(){}

        template<typename U>
        Matrix(const Matrix<U, rows, cols>& other){ this = other; }

        // Functions
        inline const Vector2u& get_size(){ return size; }
        inline const T& get(size_t row, size_t col) const { return array[row][col]; }

        // Static functions
        static Matrix<T, rows, cols> identity(){
            Matrix<T, rows, cols> mat{};

            // Create identity matrix
            for(int i = 0; i < std::min(rows, cols); i++){
                mat[i][i] = 1;
            }

            return mat;
        }

        // Operators
        T* operator[] (size_t row){ return array[row]; }
        const T* operator[] (size_t row) const { return array[row]; }

        void operator= (const Matrix<T, rows, cols>& other){
            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    array[row][col] = other[row][col];
                }
            }
        }

        bool operator== (const Matrix<T, rows, cols> &other){
            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    if(array[row][col] != other[row][col]){
                        return false;
                    }
                }
            }

            return true;
        }

        bool operator!= (const Matrix<T, rows, cols> &other){ return !(this == other); }

        Matrix<T, rows, cols>& operator+= (const Matrix<T, rows, cols>& other){
            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    array[row][col] += other[row][col];
                }
            }

            return this;
        }
        
        Matrix<T, rows, cols>& operator-= (const Matrix<T, rows, cols>& other){
            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    array[row][col] -= other[row][col];
                }
            }

            return this;
        }

        Matrix<T, rows, cols>& operator*= (T v){
            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    array[row][col] *= v;
                }
            }

            return this;
        }

        template<size_t rhsCols>
        Matrix<T, rows, rhsCols>& operator*= (const Matrix<T, cols, rhsCols>& other){
            Matrix<T, rows, rhsCols> out{};

            for(size_t i = 0; i < rows; i++){
                for(size_t k = 0; k < rhsCols; k++){
                    out[i][k] = 0;

                    for(size_t j = 0; j < cols; j++){
                        out[i][k] += (array[i][j] * other[j][k]);
                    }
                }
            }

            this = out;
            return this;
        }

        Matrix<T, rows, cols> operator+ (const Matrix<T, rows, cols>& other){
            Matrix<T, rows, cols> out{};

            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    out[row][col] = (array[row][col] + other[row][col]);
                }
            }

            return out;
        }
        
        Matrix<T, rows, cols> operator- (const Matrix<T, rows, cols>& other){
            Matrix<T, rows, cols> out{};

            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    out[row][col] = (array[row][col] - other[row][col]);
                }
            }

            return out;
        }

        Matrix<T, rows, cols> operator* (T v){
            Matrix<T, rows, cols> out{};

            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    out[row][col] = (array[row][col] * v);
                }
            }

            return out;
        }

        template<size_t rhsCols>
        Matrix<T, rows, rhsCols> operator* (const Matrix<T, cols, rhsCols>& other){
            Matrix<T, rows, rhsCols> out{};

            for(size_t i = 0; i < rows; i++){
                for(size_t k = 0; k < rhsCols; k++){
                    out[i][k] = 0;

                    for(size_t j = 0; j < cols; j++){
                        out[i][k] += (array[i][j] * other[j][k]);
                    }
                }
            }

            return out;
        }

        friend std::ostream& operator<< (std::ostream& stream, const Matrix<T, rows, cols>& v){
            // Top bar
            stream << "┌";
            for(int col = 0; col < cols * 2 + 1; col++){
                stream << "─";
            }
            stream << "┐\n";

            // Each row
            for(int row = 0; row < rows; row++){
                stream << "│ ";

                for(int col = 0; col < cols; col++){
                    stream << v.get(row, col) << " ";
                }

                stream << "│\n";
            }

            // Bottom bar
            stream << "└";
            for(int col = 0; col < cols * 2 + 1; col++){
                stream << "─";
            }
            stream << "┘\n";

            return stream;
        }
    };

    // Specializations
    template<typename T, size_t cols>
    class Matrix2 : public Matrix<T, 2, cols> {
    public:
        // Functions
        Vector2<T> operator* (const Vector2<T>& other){
            Matrix<T, 2, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp = this * tmp;
            return { tmp[0][0], tmp[1][0] };
        }
    };

    template<typename T, size_t cols>
    class Matrix3 : public Matrix<T, 3, cols> {
    public:
        // Functions
        Vector3<T> operator* (const Vector3<T>& other){
            Matrix<T, 3, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp[2][0] = other.z;
            tmp = this * tmp;
            return { tmp[0][0], tmp[1][0], tmp[2][0] };
        }
    };

    template<typename T, size_t rows>
    class Matrix4 : public Matrix<T, rows, 4> {
    public:
        // Functions
        Vector4<T> operator* (const Vector4<T>& other){
            Matrix<T, 4, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp[2][0] = other.z;
            tmp[3][0] = other.w;
            tmp = (Matrix<T, rows, 4>(*this)) * tmp;
            return { tmp[0][0], tmp[1][0], tmp[2][0], tmp[3][0] };
        }
    };

    // Aliases

};