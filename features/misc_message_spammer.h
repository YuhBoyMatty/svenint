// SvenInt (c) Sw1ft
// misc_message_spammer.h

#ifndef SINT_FEATURE_MESSAGE_SPAMMER_H
#define SINT_FEATURE_MESSAGE_SPAMMER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class ISpamOperator;
class CSpamTask;

//-----------------------------------------------------------------------------
// Type of spam operator to run
//-----------------------------------------------------------------------------

enum SpamOperatorType
{
	SPAM_OPERATOR_SEND = 0,
	SPAM_OPERATOR_SLEEP
};

//-----------------------------------------------------------------------------
// Shared vars of a spam task
//-----------------------------------------------------------------------------

class CSpamInfo
{
public:
	float flNextRunTime = 0.f;
};

//-----------------------------------------------------------------------------
// Spam task to run
//-----------------------------------------------------------------------------

class CSpamTask
{
public:
	CSpamTask( const char *pszName );
	~CSpamTask();

	bool Run( void );

	bool IsWaiting( void );
	bool IsFinished( void );
	bool IsLooped( void );

	inline const char *GetName() { return m_pszName; }

	void SetLoop( bool bLoop );
	void ResetWaiting( void );
	void AddOperator( ISpamOperator *pOperator );

public:
	CSpamInfo m_spamInfo;

private:
	std::vector<ISpamOperator *> m_operators;
	const char *m_pszName;
	int m_iOperatorBegin;
	bool m_bLoop;
};

//-----------------------------------------------------------------------------
// Spam operator interface
//-----------------------------------------------------------------------------

class ISpamOperator
{
public:
	virtual ~ISpamOperator() {}

	virtual void Run( CSpamInfo &spamInfo ) = 0;

	virtual SpamOperatorType GetType( void ) = 0;
	virtual const char *GetSyntax( void ) = 0;
};

//-----------------------------------------------------------------------------
// Implement spam operators
//-----------------------------------------------------------------------------

class CSpamOperatorSend : public ISpamOperator
{
public:
	CSpamOperatorSend();
	~CSpamOperatorSend() override;

	void Run( CSpamInfo &spamInfo ) override;

	SpamOperatorType GetType( void ) override { return SPAM_OPERATOR_SEND; }
	const char *GetSyntax( void ) override { return "send [message]"; }

public:
	void SetOperand( const char *pszMessage );

private:
	const char *m_pszMessage;
};

class CSpamOperatorSleep : public ISpamOperator
{
public:
	CSpamOperatorSleep();

	void Run( CSpamInfo &spamInfo ) override;

	SpamOperatorType GetType( void ) override { return SPAM_OPERATOR_SLEEP; }
	const char *GetSyntax( void ) override { return "sleep [delay]"; }

public:
	void SetOperand( float flSleepDelay );

private:
	float m_flSleepDelay;
};

//-----------------------------------------------------------------------------
// Message spammer feature
//-----------------------------------------------------------------------------

class CMessageSpammer final : public CBaseFeature, IHookEventListener
{
public:
	CMessageSpammer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void PrintTasks( void );

	bool AddTask( const char *pszTaskName );
	bool ReloadTask( const char *pszTaskName );
	bool RemoveTask( const char *pszTaskName );

	CSpamTask *GetTask( const char *pszTaskName );

private:
	std::vector<CSpamTask *> m_tasks;
};

EXTERN_FEATURE( CMessageSpammer, messagespammer );

#endif // SINT_FEATURE_MESSAGE_SPAMMER_H