#include "PyFRContourData.h"

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

PyFRContourData::PyFRContourData()
{
}

//------------------------------------------------------------------------------
PyFRContourData::~PyFRContourData()
{
}

//----------------------------------------------------------------------------
PyFRContourData::ScalarDataArrayHandle PyFRContourData::GetScalarData(int i) const
{
  switch (i)
    {
    case 0:
      return this->Density;
    case 1:
      return this->Pressure;
    case 2:
      return this->Velocity_u;
    case 3:
      return this->Velocity_v;
    case 4:
      return this->Velocity_w;
    default:
      return this->Density;
    }
}

//----------------------------------------------------------------------------
PyFRContourData::ScalarDataArrayHandle PyFRContourData::GetScalarData(std::string s) const
{
  if (s == "density") return this->Density;
  if (s == "pressure") return this->Pressure;
  if (s == "velocity_u") return this->Velocity_u;
  if (s == "velocity_v") return this->Velocity_v;
  if (s == "velocity_w") return this->Velocity_w;
}
