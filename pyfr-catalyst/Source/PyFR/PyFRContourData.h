#ifndef PYFRCONTOURDATA_H
#define PYFRCONTOURDATA_H

#define BOOST_SP_DISABLE_THREADS

#include <vector>

#include "PyFRContour.h"

class PyFRContourData
{
public:
  PyFRContourData() {}
  virtual ~PyFRContourData() {}

  void SetNumberOfContours(unsigned);
  unsigned GetNumberOfContours()       const { return this->Contours.size(); }
  PyFRContour& GetContour(int i)             { return this->Contours[i]; }
  const PyFRContour& GetContour(int i) const { return this->Contours[i]; }

private:
  std::vector<PyFRContour> Contours;
};

#endif
