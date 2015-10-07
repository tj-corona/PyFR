#ifndef VTKPYFRPIPELINE_H
#define VTKPYFRPIPELINE_H

#include <vtkCPPipeline.h>
#include <string>

class vtkCPDataDescription;
class vtkLiveInsituLink;
class vtkPyFRContourData;

class vtkPyFRPipeline : public vtkCPPipeline
{
public:
  static vtkPyFRPipeline* New();
  vtkTypeMacro(vtkPyFRPipeline,vtkCPPipeline)
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Initialize(char* hostName, int port, char* fileName,
                          vtkCPDataDescription* dataDescription);

  virtual int RequestDataDescription(vtkCPDataDescription* dataDescription);

  virtual int CoProcess(vtkCPDataDescription* dataDescription);

  vtkPyFRContourData* GetOutputData() { return this->OutputData; }

protected:
  vtkPyFRPipeline();
  virtual ~vtkPyFRPipeline();

private:
  vtkPyFRPipeline(const vtkPyFRPipeline&); // Not implemented
  void operator=(const vtkPyFRPipeline&); // Not implemented

  vtkLiveInsituLink* InsituLink;

  std::string FileName;

  vtkPyFRContourData* OutputData;
};
#endif
