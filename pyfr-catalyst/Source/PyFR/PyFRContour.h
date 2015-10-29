#ifndef PYFRCONTOUR_H
#define PYFRCONTOUR_H

#define BOOST_SP_DISABLE_THREADS

#include <string>

#include "ArrayHandleExposed.h"
#include "ColorTable.h"

#include <vtkm/cont/ArrayHandleTransform.h>

class PyFRContour
{
public:
  typedef vtkm::cont::ArrayHandleExposed<vtkm::Vec<FPType,3> > Vec3ArrayHandle;
  typedef internal::Color Color;
  typedef internal::ColorTable ColorFunctor;
  typedef vtkm::cont::ArrayHandleExposed<Color> ColorArrayHandle;
  typedef ArrayHandleTransform<FPType,
                               ColorArrayHandle,
                               internal::DummyConverter,
                               ColorFunctor> ScalarDataArrayHandle;

  PyFRContour() : ScalarData(this->ColorData,
                             this->ColorTable,
                             this->ColorTable)
  {
  }

  ~PyFRContour() {}

  Vec3ArrayHandle GetVertices()     const { return this->Vertices; }
  Vec3ArrayHandle GetNormals()      const { return this->Normals; }
  ScalarArrayHandle GetScalarData() const { return this->ScalarData; }
  ColorArrayHandle GetColorData()   const { return this->ColorData; }
  int GetScalarDataType()           const { return this->ScalarDataType; }

  ColorFunctor& GetColorTable() { return this->ColorTable; }
  const ColorFunctor& GetColorTable() const { return this->ColorTable; }

  void SetScalarDataType(int i) { this->ScalarDataType = i; }

private:
  Vec3ArrayHandle Vertices;
  Vec3ArrayHandle Normals;
  ScalarDataArrayHandle ScalarData;
  ColorArrayHandle ColorData;
  ColorFunctor ColorTable;
  int ScalarDataType;
};

#endif
