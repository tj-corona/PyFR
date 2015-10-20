#ifndef PYFRCONTOURDATA_H
#define PYFRCONTOURDATA_H

#define BOOST_SP_DISABLE_THREADS

#include "PyFRContour.h"

class PyFRContourData
{
public:
  PyFRContourData() {}
  virtual ~PyFRContourData() {}

  void SetNumberOfContours(unsigned i) { this->Contours.resize(i); }
  unsigned GetNumberOfContours() const { return this->Contours.size(); }
  const PyFRContour& GetContour(int i=0) const { return this->Contours[i]; }

private:
  std::vector<PyFRContour> Contours;
};

#endif
