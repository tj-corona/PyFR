#ifndef PYFRCRINKLECLIPFILTER_H
#define PYFRCRINKLECLIPFILTER_H

class PyFRData;
class vtkSphere;
class vtkPlane;

struct PyFRCrinkleClipFilter
{
  void operator ()(PyFRData*,PyFRData*,vtkSphere*) const {}
  void operator ()(PyFRData*,PyFRData*,vtkPlane*) const {}
};
#endif
