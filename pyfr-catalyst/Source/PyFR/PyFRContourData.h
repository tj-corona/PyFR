#ifndef PYFRCONTOURDATA_H
#define PYFRCONTOURDATA_H

#define BOOST_SP_DISABLE_THREADS

#include <string>

#include <vtkDataObject.h>

#include "ArrayHandleExposed.h"

class PyFRContourData
{
public:
  PyFRContourData();
  virtual ~PyFRContourData();

  typedef vtkm::cont::ArrayHandleExposed<vtkm::Vec<FPType,3> > Vec3ArrayHandle;
  typedef vtkm::cont::ArrayHandleExposed<FPType> ScalarDataArrayHandle;

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
