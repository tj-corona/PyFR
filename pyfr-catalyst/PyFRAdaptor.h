#ifndef PYFRADAPTOR_HEADER
#define PYFRADAPTOR_HEADER

#ifdef __cplusplus
extern "C"
{
#endif
  void* CatalystInitialize(char* outputfile, void* p);

  void CatalystFinalize(void* p);

  void CatalystCoProcess(double time, unsigned int timeStep, void* p);
#ifdef __cplusplus
}
#endif
#endif
