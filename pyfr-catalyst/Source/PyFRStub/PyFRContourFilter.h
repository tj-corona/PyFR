#ifndef PYFRCONTOURFILTER_H
#define PYFRCONTOURFILTER_H

class PyFRData;
class PyFRContourData;

struct PyFRContourFilter
{
  void operator ()(PyFRData*,PyFRContourData*) const {}

  void SetContourValue(FPType value) { this->ContourValue = value; }

  void SetContourField(int) {}
  void SetContourFieldToDensity()    { this->ContourField = "density"; }
  void SetContourFieldToPressure()   { this->ContourField = "pressure"; }
  void SetContourFieldToVelocity_u() { this->ContourField = "velocity_u"; }
  void SetContourFieldToVelocity_v() { this->ContourField = "velocity_v"; }
  void SetContourFieldToVelocity_w() { this->ContourField = "velocity_w"; }

protected:
  FPType ContourValue;
  std::string ContourField;
};
#endif
