//-----------------------------------------------------------------------------
//
//	Configuration.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_CONFIGURATION
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

#include "CommandClasses.h"
#include "Configuration.h"
#include "Defs.h"
#include "Msg.h"
#include "Driver.h"
#include "Node.h"
#include "Log.h"
#include "ValueByte.h"
#include "ValueInt.h"
#include "ValueList.h"
#include "ValueShort.h"

using namespace OpenZWave;

enum ConfigurationCmd
{
	ConfigurationCmd_Set	= 0x04,
	ConfigurationCmd_Get	= 0x05,
	ConfigurationCmd_Report	= 0x06
};


//-----------------------------------------------------------------------------
// <Configuration::~Configuration>
// Destructor
//-----------------------------------------------------------------------------
Configuration::~Configuration
(
)
{
	for( list<Value*>::iterator it = m_params.begin(); it != m_params.end(); ++it )
	{
		(*it)->Release();
	}
}

//-----------------------------------------------------------------------------
// <Configuration::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
bool Configuration::HandleMsg
(
	uint8 const* _data,
	uint32 const _length,
	uint32 const _instance	// = 1
)
{
	if (ConfigurationCmd_Report == (ConfigurationCmd)_data[0])
	{
		// Extract the parameter index and value
		uint8 parameter = _data[1];
		uint8 size = _data[2] & 0x07;
		uint32 paramValue = 0;
		for( uint8 i=0; i<size; ++i )
		{
			paramValue <<= 8;
			paramValue |= (uint32)_data[i+3];
		}

		char label[16];
		snprintf( label, 16, "Parameter #%d", parameter );

		switch( size )
		{
			case 1:
			{
				if( ValueByte* valueByte = static_cast<ValueByte*>( GetParam( parameter ) ) )
				{
					valueByte->OnValueChanged( (uint8)paramValue );
				}
				else
				{
					// Create a new value
					if( Node* node = GetNode() )
					{
						AddParam( node->CreateValueByte( ValueID::ValueGenre_Config, GetCommandClassId(), _instance, parameter, label, "", false, (uint8)paramValue ) );
						ReleaseNode();
					}
				}
				break;
			}
			case 2:
			{
				if( ValueShort* valueShort = static_cast<ValueShort*>( GetParam( parameter ) ) )
				{
					valueShort->OnValueChanged( (uint16)paramValue );
					valueShort->Release();
				}
				else
				{
					// Create a new value
					if( Node* node = GetNode() )
					{
						AddParam( node->CreateValueShort( ValueID::ValueGenre_Config, GetCommandClassId(), _instance, parameter, label, "", false, (uint16)paramValue ) );
						ReleaseNode();
					}
				}
				break;
			}
			case 4:
			{
				if( ValueInt* valueInt = static_cast<ValueInt*>( GetParam( parameter ) ) )
				{
					valueInt->OnValueChanged( paramValue );
					valueInt->Release();
				}
				else
				{
					// Create a new value
					if( Node* node = GetNode() )
					{
						AddParam( node->CreateValueInt( ValueID::ValueGenre_Config, GetCommandClassId(), _instance, parameter, label, "", false, (int32)paramValue ) );
						ReleaseNode();
					}
				}
				break;
			}
			default:
			{
				Log::Write( "Invalid size of %d bytes for configuration parameter %d", size, parameter );
			}
		}

		Log::Write( "Received Configuration report from node %d: Parameter=%d, Value=%d", GetNodeId(), parameter, paramValue );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// <Configuration::Get>												   
// Request current parameter value from the device									   
//-----------------------------------------------------------------------------
void Configuration::Get
(
	uint8 const _parameter
)
{
	Msg* msg = new Msg( "ConfigurationCmd_Get", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
	msg->Append( GetNodeId() );
	msg->Append( 3 );
	msg->Append( GetCommandClassId() );
	msg->Append( ConfigurationCmd_Get );
	msg->Append( _parameter );
	msg->Append( TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE );
	GetDriver()->SendMsg( msg );
}

//-----------------------------------------------------------------------------
// <Configuration::Set>
// Set the device's 
//-----------------------------------------------------------------------------
void Configuration::Set
(
	uint8 const _parameter,
	int32 const _value
)
{
	Log::Write( "Configuration::Set - Node=%d, Parameter=%d, Value=%d", GetNodeId(), _parameter, _value );

	Msg* msg = new Msg( "ConfigurationCmd_Set", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true );
	if( _value & 0xffff0000 ) 
	{
		// four byte value
		msg->Append( GetNodeId() );
		msg->Append( 8 );
		msg->Append( GetCommandClassId() );
		msg->Append( ConfigurationCmd_Set );
		msg->Append( _parameter );
		msg->Append( 4 );
		msg->Append( (uint8)((_value>>24)&0xff) );
		msg->Append( (uint8)((_value>>16)&0xff) );
		msg->Append( (uint8)((_value>>8)&0xff) );
		msg->Append( (_value & 0xff) );
		msg->Append( TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE );
	}
	else if( _value & 0x0000ff00 )
	{
		// two byte value
		msg->Append( GetNodeId() );
		msg->Append( 6 );
		msg->Append( GetCommandClassId() );
		msg->Append( ConfigurationCmd_Set );
		msg->Append( _parameter );
		msg->Append( 2 );
		msg->Append( (uint8)((_value>>8)&0xff) );
		msg->Append( (uint8)(_value&0xff) );
		msg->Append( TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE );
	}
	else
	{
		// one byte value
		msg->Append( GetNodeId() );
		msg->Append( 5 );
		msg->Append( GetCommandClassId() );
		msg->Append( ConfigurationCmd_Set );
		msg->Append( _parameter );
		msg->Append( 1 );
		msg->Append( (uint8)_value );
		msg->Append( TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE );
	}
}

//-----------------------------------------------------------------------------
// <Configuration::GetParam>
// Get the value object that represents the specified parameter
//-----------------------------------------------------------------------------
Value* Configuration::GetParam
(
	uint8 const _paramId
)
{
	for( list<Value*>::iterator it = m_params.begin(); it != m_params.end(); ++it )
	{
		Value* value = *it;
		uint8 idx = value->GetID().GetIndex();
		if( _paramId == idx )
		{
			return value;
		}
		if( idx >= _paramId )
		{
			// The list is sorted by ascending parameter ID, and
			// we have passed the point where our parameter would be.
			break;
		}
	}

	// No value for this parameter was found
	return NULL;
}

//-----------------------------------------------------------------------------
// <Configuration::AddParam>
// Adds a value object representing a parameter to the parameters list
//-----------------------------------------------------------------------------
bool Configuration::AddParam
(
	Value* _value
)
{
	// First try to insert the new value at the correct position to
	// maintain a list sorted by ascending parameter ID.
	uint8 paramId = _value->GetID().GetIndex();
	for( list<Value*>::iterator it = m_params.begin(); it != m_params.end(); ++it )
	{
		Value* value = *it;
		uint8 idx = value->GetID().GetIndex();
		if( idx > paramId )
		{
			m_params.insert( it, _value );
			return true;
		}
	}

	m_params.push_back( _value );
	return true;
}

