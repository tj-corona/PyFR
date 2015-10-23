#ifndef PYFRCRINKLECLIPFILTER_H
#define PYFRCRINKLECLIPFILTER_H

class PyFRData;
class vtkPlane;

struct PyFRCrinkleClipFilter
{
  void operator ()(PyFRData*,PyFRData*,vtkPlane*) const {}
};
#endif
