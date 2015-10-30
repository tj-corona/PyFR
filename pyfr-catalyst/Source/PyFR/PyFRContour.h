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
  typedef vtkm::cont::ArrayHandleTransform<FPType,
                                           ColorArrayHandle,
                                           ColorTable,
                                           ColorTable> ScalarDataArrayHandle;

  PyFRContour() : Vertices(),
                  Normals(),
                  ColorData(),
                  Table(),
                  ScalarData(this->ColorData,
                             this->Table,
                             this->Table),
                  ScalarDataType(-1)
  {
  }

  ~PyFRContour() {}

  Vec3ArrayHandle GetVertices()         const { return this->Vertices; }
  Vec3ArrayHandle GetNormals()          const { return this->Normals; }
  ScalarDataArrayHandle GetScalarData() const { return this->ScalarData; }
  ColorArrayHandle GetColorData()       const { return this->ColorData; }
  int GetScalarDataType()               const { return this->ScalarDataType; }

  ColorTable& GetColorTable() { return this->Table; }
  const ColorTable& GetColorTable() const { return this->Table; }

  void SetScalarDataType(int i) { this->ScalarDataType = i; }

private:
  Vec3ArrayHandle Vertices;
  Vec3ArrayHandle Normals;
  ColorArrayHandle ColorData;
  ColorTable Table;
  ScalarDataArrayHandle ScalarData;
  int ScalarDataType;
};

#endif
