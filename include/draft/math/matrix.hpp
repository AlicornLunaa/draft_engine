#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>

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
        inline const T* arr_ptr() const { return &array[0][0]; }

        Matrix<T, cols, rows> transpose(){
            Matrix<T, cols, rows> out{};

            for(size_t row = 0; row < rows; row++){
                for(size_t col = 0; col < cols; col++){
                    out[col][row] = array[row][col];
                }
            }

            return out;
        }

        // TODO: inverse

        // Static functions
        static Matrix<T, rows, cols> identity(){
            Matrix<T, rows, cols> mat{};

            // Create identity matrix
            for(int i = 0; i < std::min(rows, cols); i++){
                mat[i][i] = 1;
            }

            return mat;
        }
        
        static Matrix<T, 2, 2> scale(const Vector2<T>& vec){
            Matrix<T, 2, 2> mat = Matrix<T, 2, 2>::identity();
            mat[0][0] = vec.x;
            mat[1][1] = vec.y;
            return mat;
        }

        static Matrix<T, 2, 2> translation(const Vector2<T>& vec){
            Matrix<T, 2, 2> mat = Matrix<T, 2, 2>::identity();
            mat[0][1] = vec.x;
            mat[1][1] = vec.y;
            return mat;
        }

        static Matrix<T, 2, 2> rotation(T angle){
            Matrix<T, 2, 2> mat = Matrix<T, 2, 2>::identity();
            mat[0][0] = std::cos(angle);
            mat[0][1] = -std::sin(angle);
            mat[1][0] = std::sin(angle);
            mat[1][1] = std::cos(angle);
            return mat;
        }

        static Matrix<T, 4, 4> scale(const Vector3<T>& vec){
            Matrix<T, 4, 4> mat = Matrix<T, 4, 4>::identity();
            mat[0][0] = vec.x;
            mat[1][1] = vec.y;
            mat[2][2] = vec.z;
            return mat;
        }

        static Matrix<T, 4, 4> translation(const Vector3<T>& vec){
            Matrix<T, 4, 4> mat = Matrix<T, 4, 4>::identity();
            mat[0][3] = vec.x;
            mat[1][3] = vec.y;
            mat[2][3] = vec.z;
            return mat;
        }

        static Matrix<T, 4, 4> rotation(const Vector3<T>& angles){
            Matrix<T, 4, 4> xRotMat = Matrix<T, 4, 4>::identity();
            xRotMat[1][1] = std::cos(angles.x);
            xRotMat[1][2] = -std::sin(angles.x);
            xRotMat[2][1] = std::sin(angles.x);
            xRotMat[2][2] = std::cos(angles.x);

            Matrix<T, 4, 4> yRotMat = Matrix<T, 4, 4>::identity();
            yRotMat[0][0] = std::cos(angles.y);
            yRotMat[0][2] = std::sin(angles.y);
            yRotMat[2][0] = -std::sin(angles.y);
            yRotMat[2][2] = std::cos(angles.y);

            Matrix<T, 4, 4> zRotMat = Matrix<T, 4, 4>::identity();
            zRotMat[0][0] = std::cos(angles.z);
            zRotMat[0][1] = -std::sin(angles.z);
            zRotMat[1][0] = std::sin(angles.z);
            zRotMat[1][1] = std::cos(angles.z);

            return xRotMat * yRotMat * zRotMat;
        }

        static Matrix<float, 4, 4> orthographic(float left, float right, float bottom, float top, float near, float far){
            Matrix<float, 4, 4> mat = Matrix<float, 4, 4>::identity();
            mat[0][0] = 2.f / (right - left);
            mat[1][1] = 2.f / (top - bottom);
            mat[2][2] = -2.f / (far - near);
            mat[0][3] = -((right + left) / (right - left));
            mat[1][3] = -((top + bottom) / (top - bottom));
            mat[2][3] = -((far + near) / (far - near));
            return mat;
        }

        static Matrix<float, 4, 4> perspective(float fov, float aspect, float near, float far){
            Matrix<float, 4, 4> mat{};
            mat[0][0] = 1.f / (aspect * std::tan(fov / 2.f));
            mat[1][1] = 1.f / std::tan(fov / 2.f);
            mat[2][2] = -((far + near) / (far - near));
            mat[3][2] = -1.f;
            mat[2][3] = -((2 * far * near) / (far - near));
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

            (*this) = out;
            return *this;
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

        Vector2<T> operator* (const Vector2<T>& other){
            Matrix<T, rows, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp = (*this) * tmp;
            return { tmp[0][0], tmp[1][0] };
        }

        Vector3<T> operator* (const Vector3<T>& other){
            Matrix<T, rows, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp[2][0] = other.z;
            tmp = (*this) * tmp;
            return { tmp[0][0], tmp[1][0], tmp[2][0] };
        }

        Vector4<T> operator* (const Vector4<T>& other){
            Matrix<T, rows, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp[2][0] = other.z;
            tmp[3][0] = other.w;
            tmp = (*this) * tmp;
            return { tmp[0][0], tmp[1][0], tmp[2][0], tmp[3][0] };
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
            stream << "┘";

            return stream;
        }
    };

    // Aliases
    typedef Matrix<float, 2, 2> Matrix2;
    typedef Matrix<float, 3, 3> Matrix3;
    typedef Matrix<float, 4, 4> Matrix4;
};