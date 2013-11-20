#pragma once


#include "netdef.h"
#include "serializer.h"
#include "serializerless.h"
#include "dataset.h"
#include "timeplatform.h"

namespace NetDuke
{

class RPC
{
public:

	friend class RPCService;

	enum eState
	{
		STATE_UNDEFINED,
		STATE_SENDING,
		STATE_RECEIVING,
		STATE_DONE
	};

	enum eError
	{
		ERROR_UNDEFINED,
		ERROR_OK,
		ERROR_CANCEL,
		ERROR_KO,
		ERROR_TIME_OUT
	};

	RPC()
		: m_state(STATE_UNDEFINED)
		, m_error(ERROR_UNDEFINED)
		, m_start_time(0)
		, m_timeout_time(30000)
		, m_ser(Serializer::MTU)
		, m_seq(0)
	{
	}

	virtual netU32		GetType() const = 0;
	eState				GetState() const { return m_state; }
	eError				GetError() const { return m_error; }

	void				ChangeState(eState _state) { m_state = _state; }
	void				ChangeError(eError _error) { m_error = _error; }

	virtual Dataset&	In() = 0;
	virtual Dataset&	Out() = 0;

	netBool				Serialize(Dataset& _data, netU8 _seq)
						{ 
							m_seq = _seq;
							//if(m_ser.Write(_data.GetType()))
							if(m_ser.Write(GetType()))
							{
								m_ser << _data.GetType();
								m_ser << _seq; 
								_data.Write(m_ser); 
								m_ser.Close();
								return true;
							}
							return false;
						}

	netBool				UnSerialize(Dataset& _data, SerializerLess& _ser)
						{ 
							netBool ret = false;
							//if(_ser.Read(_data.GetType()))
							if(_ser.Read(GetType()))
							{
								netU32 type;
								static_cast<Serializer&>(_ser) >> type;

								if(type == _data.GetType())
								{
									static_cast<Serializer&>(_ser) >> m_seq; 
									_data.Read(_ser); 
									ret = true;
								}
								_ser.Close();
							}

							return ret;
						}

	netBool				IsTimeOut() const { return ((m_start_time + m_timeout_time) < Time::GetMsTime()); }
	netBool				IsComplete() const { return m_state==STATE_DONE; }
	netBool				IsSuccess() const { return m_error==ERROR_OK; }

	Serializer&			GetSerializer() { return m_ser; }
	netU8				GetSequence() const { return m_seq; }

	void				Reset()
	{
		ChangeState(STATE_UNDEFINED);
		ChangeError(ERROR_UNDEFINED);
		m_seq = 0;
		m_start_time = 0;
		m_ser.ResetCursor();
	}
	 
private:
	void		SetTimeOutTime(timer_t _ms) { m_timeout_time = _ms; } 
	void		SetStartTime(timer_t _ms) { m_start_time = _ms; } 

private:
	netU8		m_seq;
	eState		m_state;
	eError		m_error;
	timer_t		m_start_time;
	timer_t		m_timeout_time;
	Serializer	m_ser;
};



}