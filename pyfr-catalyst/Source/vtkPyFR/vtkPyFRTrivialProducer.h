#ifndef VTKPYFRTRIVIALPRODUCER_H
#define VTKPYFRTRIVIALPRODUCER_H

#include <string>
#include <vector>

#include "vtkPVTrivialProducer.h"

class VTK_EXPORT vtkPyFRTrivialProducer : public vtkPVTrivialProducer
{
public:
  vtkTypeMacro(vtkPyFRTrivialProducer,vtkPVTrivialProducer)
  static vtkPyFRTrivialProducer* New();

  vtkSetMacro(Preset,int);

protected:
  vtkPyFRTrivialProducer();
  virtual ~vtkPyFRTrivialProducer();

  int Preset;

private:
  vtkPyFRTrivialProducer(const vtkPyFRTrivialProducer&);
  void operator=(const vtkPyFRTrivialProducer&);
};
#endif
