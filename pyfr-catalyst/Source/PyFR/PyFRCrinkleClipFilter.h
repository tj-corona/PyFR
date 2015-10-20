#ifndef PYFRCRINKLECLIPFILTER_H
#define PYFRCRINKLECLIPFILTER_H

#include <string>
#include <vector>

#define BOOST_SP_DISABLE_THREADS

class PyFRData;

class PyFRCrinkleClipFilter
{
public:
  PyFRCrinkleClipFilter();
  virtual ~PyFRCrinkleClipFilter();

  void operator ()(PyFRData*,PyFRData*) const;

  void AddContourValue(FPType value) { this->ContourValues.push_back(value); }
  void ClearContourValues()          { this->ContourValues.clear(); }

  void SetContourField(int i);
  void SetContourFieldToDensity()    { this->ContourField = "density"; }
  void SetContourFieldToPressure()   { this->ContourField = "pressure"; }
  void SetContourFieldToVelocity_u() { this->ContourField = "velocity_u"; }
  void SetContourFieldToVelocity_v() { this->ContourField = "velocity_v"; }
  void SetContourFieldToVelocity_w() { this->ContourField = "velocity_w"; }

protected:
  std::vector<FPType> ContourValues;
  std::string ContourField;
};
#endif
