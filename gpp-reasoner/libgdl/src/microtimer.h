#pragma once

#if LINUX || MAC_OSX
#include <sys/time.h> // not compatible to windows
#include <time.h>

inline uint64_t microtimer (){
	timeval t;
	gettimeofday (&t, NULL);
	return t.tv_sec * 1000000 + t.tv_usec;
}
#else
#include <windows.h>

typedef unsigned long long uint64_t; // why don't windows have this?
inline uint64_t microtimer (){
	/*
	Seems as windows doesn't provide a direct way for 
	fast high accuracy measurements
	*/
	return GetTickCount () * 1000;
}
#endif
