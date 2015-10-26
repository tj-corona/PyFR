#ifndef PYFRCRINKLECLIPFILTER_H
#define PYFRCRINKLECLIPFILTER_H

#define BOOST_SP_DISABLE_THREADS

class PyFRData;

class PyFRCrinkleClipFilter
{
public:
  PyFRCrinkleClipFilter() {}
  virtual ~PyFRCrinkleClipFilter() {}

  void operator ()(PyFRData*,PyFRData*,const FPType*,const FPType*) const;

};

#endif
