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
  typedef vtkm::cont::ArrayHandleExposed<Color> ColorArrayHandle;
  typedef vtkm::cont::ArrayHandle<FPType> ScalarDataArrayHandle;

  PyFRContour(const ColorTable& table) : Vertices(),
                                         Normals(),
                                         Table(),
                                         ColorData(),
                                         ScalarData(),
                                         ScalarDataType(-1)
  {
  }

  ~PyFRContour() {}

  Vec3ArrayHandle GetVertices()         const { return this->Vertices; }
  Vec3ArrayHandle GetNormals()          const { return this->Normals; }
  ScalarDataArrayHandle GetScalarData() const { return this->ScalarData; }
  ColorArrayHandle GetColorData();
  int GetScalarDataType()               const { return this->ScalarDataType; }

  void ChangeColorTable(const ColorTable& table)
  {
    this->Table = table;
  }

  void SetScalarDataType(int i) { this->ScalarDataType = i; }

private:
  Vec3ArrayHandle Vertices;
  Vec3ArrayHandle Normals;
  ColorTable Table;
  ColorArrayHandle ColorData;
  ScalarDataArrayHandle ScalarData;
  int ScalarDataType;
};

#endif
