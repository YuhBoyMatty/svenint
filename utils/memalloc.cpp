// SvenInt (c) Sw1ft
// memalloc.cpp

#include "stdafx.h"
#include "memalloc.h"
#include "hashtable.h"

#include <mutex>

//-----------------------------------------------------------------------------
// Auto lock access to the called function when doing multithreading
//-----------------------------------------------------------------------------

#if 1
#define AUTO_LOCK( mutex ) CMutexAutoLock __##mutex##__autolock(mutex)

class CMutexAutoLock
{
public:
    CMutexAutoLock( std::mutex &mutex )
    {
        m_mutex = &mutex;
        m_mutex->lock();
    }

    ~CMutexAutoLock()
    {
        m_mutex->unlock();
    }

private:
    std::mutex *m_mutex;
};

static std::mutex mempool_mutex;
#else
#define AUTO_LOCK( mutex ) (void)0;
#endif

//-----------------------------------------------------------------------------
// Memory block
//-----------------------------------------------------------------------------

typedef struct memblock_s
{
    unsigned int size;
    const char *name;
} memblock_t;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static CHashTable<void *, memblock_t> mempool( 63 );
static int mempoolsize = 0;

//-----------------------------------------------------------------------------
// MemAlloc
//-----------------------------------------------------------------------------

void *MemAlloc( unsigned int size, const char *pszMemoryName /* = NULL */ )
{
    AUTO_LOCK( mempool_mutex );

	void *ptr = malloc( size );
    if ( ptr == NULL )
        return NULL;

    if ( mempool.Insert( ptr, { size, pszMemoryName } ) )
        mempoolsize += size;

	return ptr;
}

//-----------------------------------------------------------------------------
// MemCalloc
//-----------------------------------------------------------------------------

void *MemCalloc( unsigned int count, unsigned int size, const char *pszMemoryName /* = NULL */ )
{
    AUTO_LOCK( mempool_mutex );

	void *ptr = calloc( count, size );
    if ( ptr == NULL )
        return NULL;

    if ( mempool.Insert( ptr, { count * size, pszMemoryName } ) )
        mempoolsize += count * size;

	return ptr;
}

//-----------------------------------------------------------------------------
// MemRealloc
//-----------------------------------------------------------------------------

void *MemRealloc( void *ptr, unsigned int size, const char *pszMemoryName /* = NULL */ )
{
    AUTO_LOCK( mempool_mutex );

    if ( ptr == NULL )
    {
        return MemAlloc( size, pszMemoryName );
    }
    if ( size == 0 )
    {
        MemFree( ptr );
        return NULL;
    }

	void *reallocPtr = realloc( ptr, size );
    if ( reallocPtr == NULL )
        return NULL;

    memblock_t *block = mempool.Find( ptr );
    if ( block != NULL )
    {
        pszMemoryName = block->name;
        mempoolsize -= block->size;
        mempool.Remove( ptr );
    }

    if ( mempool.Insert( reallocPtr, { size, pszMemoryName } ) )
        mempoolsize += size;

	return reallocPtr;
}

//-----------------------------------------------------------------------------
// MemStrdup
//-----------------------------------------------------------------------------

char *MemStrdup( const char *str, const char *pszMemoryName /* = NULL */ )
{
    AUTO_LOCK( mempool_mutex );

    char *ptr = strdup( str );
    if ( ptr == NULL )
        return NULL;

    size_t len = strlen( str ) + 1;
    if ( mempool.Insert( ptr, { len, pszMemoryName } ) )
        mempoolsize += len;

    return ptr;
}

//-----------------------------------------------------------------------------
// MemFree
//-----------------------------------------------------------------------------

void MemFree( void *ptr )
{
    AUTO_LOCK( mempool_mutex );

	free( ptr );

    memblock_t *block = mempool.Find( ptr );
    if ( block == NULL )
        return;

    mempoolsize -= block->size;
    mempool.Remove( ptr );
}

//-----------------------------------------------------------------------------
// MemConsumed
//-----------------------------------------------------------------------------

unsigned int MemConsumed( void )
{
	return mempoolsize;
}

//-----------------------------------------------------------------------------
// MemCheckLeaks
//-----------------------------------------------------------------------------

bool MemCheckLeaks( void )
{
	if ( mempoolsize == 0 )
        return false;

    Msg( "[SvenInt] Detected leak of %d bytes (%.2f kilobytes) of memory!\n", MemConsumed(), (float)MemConsumed() / 1024.f );

    MemDump();
    return true;
}

//-----------------------------------------------------------------------------
// MemDump
//-----------------------------------------------------------------------------

void MemDump( void )
{
    for ( int i = 0; i < mempool.Count(); i++ )
	{
        HashTableIterator_t it = mempool.First( i );

		while ( mempool.IsValidIterator( it ) )
		{
			void *&ptr = mempool.KeyAt( i, it );
			memblock_t &block = mempool.ValueAt( i, it );
            Msg( "<SvenInt::MemDump> Block 0x%.8X of size %d bytes (name: %s)\n", ptr, block.size, block.name );

			it = mempool.Next( i, it );
		}
	}
}
