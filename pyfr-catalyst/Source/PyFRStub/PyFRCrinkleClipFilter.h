#ifndef PYFRCRINKLECLIPFILTER_H
#define PYFRCRINKLECLIPFILTER_H

class PyFRData;
class vtkSphere;
class vtkPlane;

struct PyFRCrinkleClipFilter
{
  void PyFRCrinkleClipFilter::operator ()(PyFRData*,PyFRData*,vtkSphere&) const {}
  void PyFRCrinkleClipFilter::operator ()(PyFRData*,PyFRData*,vtkPlane&) const {}
};
#endif
