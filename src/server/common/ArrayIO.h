#ifndef ARRAYIO_H_
#define ARRAYIO_H_

#include <iostream>
#include "Array.h"
#include "MDArray.h"
#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

namespace sail {
template <typename T>
std::ostream &operator << (std::ostream &s, Array<T> arr) {
  s << "Array with " << arr.size() << " elements:\n  ";
  for (int i = 0; i < arr.size(); i++) {
    s << arr[i] << " ";
  }
  return s;
}

template <typename T>
void outputAsCArrayInitializer(std::string name, Array<T> arr, std::ostream *out) {
  *out << name << "[" << arr.size() << "] = {";
  int lastIndex = arr.size()-1;
  for (int i = 0; i < lastIndex; i++) {
    *out << arr[i] << ", ";
  }
  *out << arr[lastIndex] << "};\n";
}

#define OUTPUT_AS_C_ARRAY_INITIALIZER(stream, arr) outputAsCArrayInitializer(#arr, (arr), &(stream))

// Display an array assuming it stores a matrix
template <typename T>
void dispMat(std::ostream &s, MDArray<T, 2> mat, int width = 5, int precision = 3, int indent = 2) {
  int rows = mat.rows();
  int cols = mat.cols();
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < indent; j++) {
      s << " ";
    }
    for (int j = 0; j < cols; j++) {
      s << std::setw(width) << std::setprecision(precision) << mat.get(i, j) << " ";
    }
    s << "\n";
  }
}

template <typename T, int dims>
std::ostream &operator << (std::ostream &s, MDArray<T, dims> arr) {
  if (dims == 2) {
    dispMat(s, arr);
  } else {
    int count = arr.numel();
    s << "MDArray with " << count << " elements\n";
    if (count > 0) {
      // Initialize indices
      int inds[dims];
      for (int i = 0; i < dims; i++) {
        inds[i] = 0;
      }

      // Iterate
      for (int i = 0; i < count; i++) {
        s << "Element (";
        for (int j = 0; j < dims; j++) {
          s << inds[j] << (j < dims-1? ", " : "");
        }
        s << ") = \n";
        s << arr.get(inds) << "\n";
        arr.step(inds, 1);
      }
    }
  }
  return s;
}

// Display a type which supports the operations rows() cols() and get(i, j)
template <typename T>
T dispMatrix(std::ostream &s, T mat, int width = 5, int precision = 3, int indent = 2, bool lastLineBreak = true) {
  int rows = mat.rows();
  int cols = mat.cols();
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < indent; j++) {
      s << " ";
    }
    for (int j = 0; j < cols; j++) {
      s << std::setw(width) << std::setprecision(precision) << mat.get(i, j) << " ";
    }
    if (lastLineBreak || i < rows-1) {
      s << "\n";
    }
  }
  return mat;
}

template <typename T>
void saveMatrix(std::string filename, T mat, int width = 5, int precision = 3, int indent = 2, bool lastLineBreak = true) {
  std::ofstream file(filename);
  dispMatrix(file, mat, width, precision, indent, lastLineBreak);
}

// GOOD FOR DEBUGGING MATRIX EXPRESSIONS. See the macro DISPMAT
template <typename T>
T dispMatrixLabeled(std::ostream &s, std::string label,
                    T mat, int width = 5, int precision = 3, int indent = 2) {
  s << label << ":\n";
  return dispMatrix(s, mat, width, precision, indent);
}

template <typename T>
T dispMatrixMatlab(std::ostream &s, std::string label,
                   T mat, int width = 12, int precision = 8, int indent = 2) {
  s << label << " = [";
  dispMatrix(s, mat, width, precision, indent, false);
  s << "];\n";
  return mat;
}



/*void saveData(T)(string filename, T[] data)
{
	File file = File(filename, "w");
	file.rawWrite(data);
}

T[] loadData(T)(string filename)
{
	File file = File(filename, "r");

	ulong filesize = file.size;
	ulong itemsize = T.sizeof;
	assert(filesize % itemsize == 0);
	uint count = cast(uint)(filesize/itemsize);
	T[] data = new T[count];
	return file.rawRead(data);
}*/

template <typename T>
void saveRawArray(std::string filename, Array<T> data) {
  ofstream file(filename, ios::out | ios::binary);
  file.write(data.getData(), sizeof(T)*data.size());
}

template <typename T>
Array<T> loadRawArray(std::string filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate | std::ios::in);
  int size = file.tellg();
  int count = size/sizeof(T);
  assert(count*sizeof(T) == size);
  Array<T> data(count);
  file.seekg(0, ios::beg);
  file.read((char *)data.getData(), size);
  return data;
}

template <typename T>
MDArray<T, 2> loadMatrixText(std::string filename) {
  // http://stackoverflow.com/questions/8570157/load-float-double-matrix-from-txt-using-c
  std::ifstream infile(filename);
  std::string line;

  std::vector<T> matrixdata;

  int rows = 0;
  int cols = 0;
  while (std::getline(infile, line)) {
    int l = 0;
    std::istringstream iss(line);
    T d = 0;
    while (iss >> d) {
      matrixdata.push_back(d);
      l++;
    }
    if (l != 0) {
      assert(cols == l || cols == 0);
      cols = l;
      rows++;
    }
  }
  assert(rows*cols == matrixdata.size());
  MDArray<T, 2> dst(rows, cols);
  int counter = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      dst(i, j) = matrixdata[counter];
      counter++;
    }
  }
  assert(counter == matrixdata.size());
  return dst;
}

template <typename T>
void outputMatrixFmt(std::ostream &out, T mat, std::string fmt) {
  for (int i = 0; i < mat.rows(); i++) {
    for (int j = 0; j < mat.cols(); j++) {
      out << stringFormat(fmt, mat.get(i, j)) << " ";
    }
    out << "\n";
  }
}


template <typename T>
void saveMatrixFmt(std::string filename, T mat, std::string fmt = "%f") {
  std::ofstream file(filename);
  outputMatrixFmt(file, mat, fmt);
}


} /* namespace sail */
#endif /* ARRAYIO_H_ */
