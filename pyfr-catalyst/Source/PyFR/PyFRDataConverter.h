#ifndef PYFRCONTOURDATACONVERTER_H
#define PYFRCONTOURDATACONVERTER_H

#define BOOST_SP_DISABLE_THREADS

class PyFRData;
class PyFRContourData;
class vtkPolyData;
class vtkUnstructuredGrid;

class PyFRDataConverter
{
public:
  PyFRDataConverter();
  virtual ~PyFRDataConverter();

  void operator()(PyFRData*,vtkUnstructuredGrid*) const;
  void operator()(PyFRContourData*,vtkPolyData*) const;
};
#endif
