#ifndef PYFRADAPTOR_HEADER
#define PYFRADAPTOR_HEADER

#ifdef __cplusplus
extern "C"
{
#endif
  void CatalystInitialize(int numScripts, char* scripts[]);

  void CatalystFinalize();

  void CatalystCoProcess(double time, unsigned int timeStep, void* data);
#ifdef __cplusplus
}
#endif
#endif
