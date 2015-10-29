#ifndef PYFRCONTOURDATA_H
#define PYFRCONTOURDATA_H

struct PyFRContourData
{
  unsigned GetNumberOfContours() const { return 0; }
  unsigned GetContourSize(int) const { return 0; }
  void ComputeContourBounds(int,FPType*) const {}
  void ComputeBounds(FPType*) const {}
};

#endif
