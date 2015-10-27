#ifndef PYFRPARALLELSLICEFILTER_H
#define PYFRPARALLELSLICEFILTER_H

#include <string>
#include <vector>

#define BOOST_SP_DISABLE_THREADS

class PyFRData;
class PyFRContourData;

class PyFRParallelSliceFilter
{
public:
  PyFRParallelSliceFilter();
  virtual ~PyFRParallelSliceFilter();

  void SetPlane(const FPType*,const FPType*);
  void SetSpacing(FPType spacing) { this->Spacing = spacing; }
  void SetNumberOfPlanes(unsigned n) { this->NPlanes = n; }

  void operator ()(PyFRData*,PyFRContourData*) const;

protected:

  FPType Origin[3];
  FPType Normal[3];
  FPType Spacing;
  unsigned NPlanes;
};
#endif
