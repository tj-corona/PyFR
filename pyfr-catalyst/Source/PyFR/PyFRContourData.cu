#include "PyFRContourData.h"

//----------------------------------------------------------------------------
void PyFRContourData::SetNumberOfContours(unsigned nContours)
{
  // NB: Cannot call resize to increase the lengths of vectors of array
  // handles (or classes containing them)! You will end up with a vector of
  // smart pointers to the same array instance. A specialization of
  // std::allocator<> for array handles should be created.
  for (unsigned i=this->Contours.size();i<nContours;i++)
    this->Contours.push_back(PyFRContour());
  this->Contours.resize(nContours);
}
