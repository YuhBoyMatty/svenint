//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef SINT_COMMANDLINE_H
#define SINT_COMMANDLINE_H

#ifdef _WIN32
#pragma once
#endif

class CCommandLine
{
public:
	// Construction
	CCommandLine( void );
	~CCommandLine( void );

	// Implements ICommandLine
	void		CreateCmdLine( const char *commandline );
	void		CreateCmdLine( int argc, char **argv );
	const char *GetCmdLine( void ) const;
	virtual	const char *CheckParm( const char *psz, const char **ppszValue = 0 ) const;
	// A bool return of whether param exists, useful for just checking if param that is just a flag is set
	bool		HasParm( const char *psz ) const;

	void		RemoveParm( const char *parm );
	void		AppendParm( const char *pszParm, const char *pszValues );

	int			ParmCount() const;
	int			FindParm( const char *psz ) const;
	const char *GetParm( int nIndex ) const;

	const char *ParmValue( const char *psz, const char *pDefaultVal = 0 ) const;
	int			ParmValue( const char *psz, int nDefaultVal ) const;
	float		ParmValue( const char *psz, float flDefaultVal ) const;
	void        SetParm( int nIndex, char const *pParm );

	const char **GetParms() const { return (const char **)m_ppParms; }

private:
	enum
	{
		MAX_PARAMETER_LEN = 128,
		MAX_PARAMETERS = 256,
	};

	// When the commandline contains @name, it reads the parameters from that file
	void LoadParametersFromFile( const char *&pSrc, char *&pDst, size_t maxDestLen, bool bInQuotes );

	// Parse command line...
	void ParseCommandLine();

	// Frees the command line arguments
	void CleanUpParms();

	// Adds an argument..
	void AddArgument( const char *pFirst, const char *pLast );

private:
	// Copy of actual command line
	char *m_pszCmdLine;

	// Pointers to each argument...
	int m_nParmCount;
	char *m_ppParms[ MAX_PARAMETERS ];
};

CCommandLine *CommandLine();

#endif // SINT_COMMANDLINE_H