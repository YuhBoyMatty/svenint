// SvenInt (c) Sw1ft
// memalloc.h

#ifndef SINT_MEMALLOC_H
#define SINT_MEMALLOC_H

#ifdef _WIN32
#pragma once
#endif

#include <forward_list>

#define MEMALLOC_ENABLED
#ifdef MEMALLOC_ENABLED

void *MemAlloc( unsigned int size, const char *pszMemoryName = NULL );
void *MemCalloc( unsigned int count, unsigned int size, const char *pszMemoryName = NULL );
void *MemRealloc( void *ptr, unsigned int size, const char *pszMemoryName = NULL );
char *MemStrdup( const char *str, const char *pszMemoryName = NULL );
void MemFree( void *ptr );
unsigned int MemConsumed( void );
bool MemCheckLeaks( void );
void MemDump( void );

template <class T, class... Args>
inline T *MemAllocInstance( T *ptr, Args&&... args )
{
	return new( MemAlloc( sizeof( T ) ) ) T( std::forward<Args>( args )... );
}

template <class T>
void MemFreeInstance( T *ptr )
{
	ptr->~T();
	MemFree( ptr );
}

#else

#define MemAlloc( size, name ) malloc( size )
#define MemCalloc( count, size, name ) calloc( count, size )
#define MemRealloc( ptr, size ) realloc( ptr, size )
#define MemStrdup( str ) strdup( str )
#define MemFree( ptr ) free( (void *)ptr )

template <class T, class... Args>
inline T *MemAllocInstance( T *ptr, Args&&... args )
{
	return new T( std::forward<Args>( args )... );
}

template <class T>
void MemFreeInstance( T *ptr )
{
	delete ptr;
}

#endif

#endif // SINT_MEMALLOC_H
