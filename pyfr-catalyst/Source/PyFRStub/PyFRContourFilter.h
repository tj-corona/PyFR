#ifndef PYFRCONTOURFILTER_H
#define PYFRCONTOURFILTER_H

class PyFRData;
class PyFRContourData;

struct PyFRContourFilter
{
  void operator ()(PyFRData*,PyFRContourData*) const {}

  void AddContourValue(FPType) {}
  void ClearContourValues() {}

  void SetContourField(int) {}
  void SetContourFieldToDensity() {}
  void SetContourFieldToPressure() {}
  void SetContourFieldToVelocity_u() {}
  void SetContourFieldToVelocity_v() {}
  void SetContourFieldToVelocity_w() {}
};
#endif
