#ifndef FIELDDATA_H
#define FIELDDATA_H

#include <vtkDataObject.h>
#include <vtkm/cont/DataSet.h>
#include "field.h"

class FieldData : public vtkDataObject
{
public:
  FieldData(void* field);
  virtual ~FieldData();

  virtual void Update();

private:
  size_t n;
  struct field* fld;
  int32_t* types;
  vtkm::cont::DataSet ds;
};
#endif
