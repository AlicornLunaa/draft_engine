#pragma once

#include <algorithm>
#include <cmath>
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
        inline const T* arr_ptr() const { return &array[0][0]; }
        // TODO: Transpose and inverse

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
    template<typename T, size_t rows>
    class Matrix2 : public Matrix<T, rows, 2> {
    public:
        // Operators
        operator Matrix<T, rows, 2>&(){ return (Matrix<T, rows, 2>)(*this); }

        // Functions
        Vector2<T> operator* (const Vector2<T>& other){
            Matrix<T, 2, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp = (Matrix<T, rows, 2>(*this)) * tmp;
            return { tmp[0][0], tmp[1][0] };
        }

        // Static functions
        static Matrix2<T, rows> identity(){
            Matrix2<T, rows> mat{};

            // Create identity matrix
            for(int i = 0; i < std::min((int)rows, 2); i++){
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
    };

    template<typename T, size_t rows>
    class Matrix3 : public Matrix<T, rows, 3> {
    public:
        // Operators
        operator Matrix<T, rows, 3>&(){ return (Matrix<T, rows, 3>)(*this); }

        // Functions
        Vector3<T> operator* (const Vector3<T>& other){
            Matrix<T, 3, 1> tmp{}; // Makes multiplication easier
            tmp[0][0] = other.x;
            tmp[1][0] = other.y;
            tmp[2][0] = other.z;
            tmp = (Matrix<T, rows, 3>(*this)) * tmp;
            return { tmp[0][0], tmp[1][0], tmp[2][0] };
        }

        // Static functions
        static Matrix3<T, rows> identity(){
            Matrix3<T, rows> mat{};

            // Create identity matrix
            for(int i = 0; i < std::min((int)rows, 3); i++){
                mat[i][i] = 1;
            }

            return mat;
        }

        static Matrix<T, 3, 3> scale(const Vector3<T>& vec){
            Matrix<T, 3, 3> mat = Matrix<T, 3, 3>::identity();
            mat[0][0] = vec.x;
            mat[1][1] = vec.y;
            mat[2][2] = vec.z;
            return mat;
        }

        static Matrix<T, 3, 3> translation(const Vector3<T>& vec){
            Matrix<T, 3, 3> mat = Matrix<T, 3, 3>::identity();
            mat[0][2] = vec.x;
            mat[1][2] = vec.y;
            mat[2][2] = vec.z;
            return mat;
        }

        static Matrix<T, 3, 3> rotation(const Vector3<T>& angles){
            Matrix<T, 3, 3> xRotMat = Matrix<T, 3, 3>::identity();
            xRotMat[1][1] = std::cos(angles.x);
            xRotMat[1][2] = -std::sin(angles.x);
            xRotMat[2][1] = std::sin(angles.x);
            xRotMat[2][2] = std::cos(angles.x);

            Matrix<T, 3, 3> yRotMat = Matrix<T, 3, 3>::identity();
            yRotMat[0][0] = std::cos(angles.y);
            yRotMat[0][2] = std::sin(angles.y);
            yRotMat[2][0] = -std::sin(angles.y);
            yRotMat[2][2] = std::cos(angles.y);

            Matrix<T, 3, 3> zRotMat = Matrix<T, 3, 3>::identity();
            zRotMat[0][0] = std::cos(angles.z);
            zRotMat[0][1] = -std::sin(angles.z);
            zRotMat[1][0] = std::sin(angles.z);
            zRotMat[1][1] = std::cos(angles.z);

            return xRotMat * yRotMat * zRotMat;
        }
    };

    template<typename T, size_t rows>
    class Matrix4 : public Matrix<T, rows, 4> {
    public:
        // Operators
        operator Matrix<T, rows, 4>&(){ return (Matrix<T, rows, 4>)(*this); }

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

        // Static functions
        static Matrix4<T, rows> identity(){
            Matrix4<T, rows> mat{};

            // Create identity matrix
            for(int i = 0; i < std::min((int)rows, 4); i++){
                mat[i][i] = 1;
            }

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
    };

    // Aliases
    typedef Matrix2<float, 2> Matrix2x2;
    typedef Matrix3<float, 3> Matrix3x3;
    typedef Matrix4<float, 4> Matrix4x4;
};