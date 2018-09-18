#ifndef _DLL_EXPORT_MP_H_
#define _DLL_EXPORT_MP_H_
#ifdef _MP_EXPORT_
#define DECLSPEC_MP __declspec(dllexport)
#else
#define DECLSPEC_MP __declspec(dllimport)
#endif
#endif