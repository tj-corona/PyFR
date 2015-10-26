#ifndef PYFRCRINKLECLIPFILTER_H
#define PYFRCRINKLECLIPFILTER_H

class PyFRData;

struct PyFRCrinkleClipFilter
{
  void operator ()(PyFRData*,PyFRData*,const FPType*,const FPType*) const {}
};
#endif
