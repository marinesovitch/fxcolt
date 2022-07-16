#ifndef INC_BACKEND_BACKEND_H
#define INC_BACKEND_BACKEND_H

#ifdef BACKEND_EXPORTS
#define BACKEND_API __declspec(dllexport)
#else
#define BACKEND_API __declspec(dllimport)
#endif

#endif
