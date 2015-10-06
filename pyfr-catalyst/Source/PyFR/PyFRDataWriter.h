#ifndef PYFRCONTOURDATAWRITER_H
#define PYFRCONTOURDATAWRITER_H

#include <string>

#define BOOST_SP_DISABLE_THREADS

class PyFRData;
class PyFRContourData;

class PyFRDataWriter
{
public:
  PyFRDataWriter();
  virtual ~PyFRDataWriter();

  void operator()(PyFRData*) const;
  void operator()(PyFRContourData*) const;

  void SetFileName(std::string fileName) { FileName = fileName; }
  std::string GetFileName() const { return FileName; }

  void SetDataModeToAscii() { IsBinary = false; }
  void SetDataModeToBinary() { IsBinary = true; }

private:
  bool IsBinary;
  std::string FileName;
};
#endif
