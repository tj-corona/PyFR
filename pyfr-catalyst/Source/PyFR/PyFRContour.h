#ifndef PYFRCONTOUR_H
#define PYFRCONTOUR_H

#define BOOST_SP_DISABLE_THREADS

#include <string>

#include "ArrayHandleExposed.h"

class PyFRContour
{
public:
  typedef vtkm::cont::ArrayHandleExposed<vtkm::Vec<FPType,3> > Vec3ArrayHandle;
  typedef vtkm::cont::ArrayHandleExposed<FPType> ScalarDataArrayHandle;

  PyFRContour() {}
  ~PyFRContour() {}

  Vec3ArrayHandle GetVertices() const { return this->Vertices; }
  Vec3ArrayHandle GetNormals() const { return this->Normals; }

  ScalarDataArrayHandle GetScalarData(int) const;
  ScalarDataArrayHandle GetScalarData(std::string) const;

private:
  Vec3ArrayHandle Vertices;
  Vec3ArrayHandle Normals;
  ScalarDataArrayHandle Density;
  ScalarDataArrayHandle Pressure;
  ScalarDataArrayHandle Velocity_u;
  ScalarDataArrayHandle Velocity_v;
  ScalarDataArrayHandle Velocity_w;
};

#endif
