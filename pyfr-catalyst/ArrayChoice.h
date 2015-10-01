#ifndef VTKARRAYCHOICE_H
#define VTKARRAYCHOICE_H

class vtkFloatArray;
class vtkDoubleArray;

template <typename fptype>
struct ArrayChoice;

template <>
struct ArrayChoice<float>
{
  typedef vtkFloatArray type;
};

template <>
struct ArrayChoice<double>
{
  typedef vtkDoubleArray type;
};

#endif
