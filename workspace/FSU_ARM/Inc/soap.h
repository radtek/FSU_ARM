#ifndef SOAP_H_
#define SOAP_H_
#include <string>

bool SCLogin_Invoke(std::string outStr, std::string & sRtn);
bool SCGetter_Invoke(std::string outStr, std::string & sRtn);
bool FSUServiceStart();
void FSUServiceRestart();
void FSUServiceStop();
void SetSCTimeout(int, int, int);

int soapTest(void);

#endif /* SOAP_H_ */
