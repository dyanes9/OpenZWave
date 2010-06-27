//-----------------------------------------------------------------------------
//
//	WakeUp.h
//
//	Implementation of the Z-Wave COMMAND_CLASS_WAKE_UP
//
//	Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#ifndef _WakeUp_H
#define _WakeUp_H

#include <list>
#include "CommandClass.h"
#include "Mutex.h"

namespace OpenZWave
{
	class Msg;
	class ValueInt;

	class WakeUp: public CommandClass
	{
	public:
		static CommandClass* Create( uint32 const _homeId, uint8 const _nodeId ){ return new WakeUp( _homeId, _nodeId ); }
		virtual ~WakeUp();

		static uint8 const StaticGetCommandClassId(){ return 0x84; }		
		static string const StaticGetCommandClassName(){ return "COMMAND_CLASS_WAKE_UP"; }

		void QueueMsg( Msg* msg );
		void SendPending();
		bool IsAwake()const{ return m_awake; }
		void SetAwake( bool _state );
		void SetPollRequired(){ m_pollRequired = true; }

		// From CommandClass
		virtual void RequestState( uint32 const _requestFlags );
		virtual uint8 const GetCommandClassId()const{ return StaticGetCommandClassId(); }
		virtual string const GetCommandClassName()const{ return StaticGetCommandClassName(); }
		virtual bool HandleMsg( uint8 const* _data, uint32 const _length, uint32 const _instance = 1 );
		virtual bool SetValue( Value const& _value );

	protected:
		virtual void CreateVars( uint8 const _instance );

	private:
		WakeUp( uint32 const _homeId, uint8 const _nodeId );

		Mutex		m_mutex;			// Serialize access to the pending queue
		list<Msg*>	m_pendingQueue;		// Messages waiting to be sent when the device wakes up
		bool		m_awake;
		bool		m_pollRequired;

		ValueInstances<ValueInt>	m_interval;
	};

} // namespace OpenZWave

#endif


