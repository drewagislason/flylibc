/**************************************************************************************************
  FlyErr.h
  Copyright 2024 Drew Gislason
  license: MIT <https://mit-license.org>
**************************************************************************************************/
#ifndef FLY_ERR_H
#define FLY_ERR_H

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  extern "C" {
#endif

typedef enum
{
  ERR_NONE   = 0,
  ERR_FAILED = 1,  // general failure
  ERR_NOMEM,       // not enought memory
  ERR_BADPATH,     // bad path to file
  ERR_NOTFOUND,    // file not found, item not found, etc...
  ERR_BADPARM,     // bad parameter
  ERR_READ,        // read permission error
  ERR_WRITE,       // write permission error
  ERR_TIMEOUT,     // timeout
  ERR_TOOLARGE     // item to large
} FlyErr_t;

// allows source to be compiled with gcc or g++ compilers
#ifdef __cplusplus
  }
#endif

#endif // FLY_ERR_H
