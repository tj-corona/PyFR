#ifndef PYFRCONVERTER_H
#define PYFRCONVERTER_H

#define BOOST_SP_DISABLE_THREADS

class PyFRData;
class PyFRContourData;
class vtkPolyData;
class vtkUnstructuredGrid;

class PyFRConverter
{
public:
  PyFRConverter();
  virtual ~PyFRConverter();

  void operator()(const PyFRData*,vtkUnstructuredGrid*) const;
  void operator()(const PyFRContourData*,vtkPolyData*) const;
};
#endif
