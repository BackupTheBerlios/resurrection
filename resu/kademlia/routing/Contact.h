/*
Copyright (C)2003 Barry Dunne (http://www.emule-project.net)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 


This work is based on the java implementation of the Kademlia protocol.
Kademlia: Peer-to-peer routing based on the XOR metric
Copyright (C) 2002  Petar Maymounkov [petar@post.harvard.edu]
http://kademlia.scs.cs.nyu.edu
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the offical client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/
#pragma once
#include "../utils/UInt128.h"
//KTS+ IP to Country
#include "IP2Country.h"
//KTS- IP to Country
////////////////////////////////////////
namespace Kademlia {
////////////////////////////////////////

class CRoutingZone;
class CRoutingBin;

class CContact
{
	friend class CRoutingZone;
	friend class CRoutingBin;
	//KTS+ IP to Country
	friend class CKadContactListCtrl;
	//KTS- IP to Country
public:

	~CContact();
	CContact();
	CContact(const CUInt128 &clientID, uint32 ip, uint16 udpPort, uint16 tcpPort);
	CContact(const CUInt128 &clientID, uint32 ip, uint16 udpPort, uint16 tcpPort, const CUInt128 &target);

	void getClientID(CUInt128 *id) const;
	CUInt128 getClientID() const {return m_clientID;}
	void getClientID(CString *id) const;
	void setClientID(const CUInt128 &clientID);

	void getDistance(CUInt128 *distance) const;
	void getDistance(CString *distance) const;

	uint32 getIPAddress(void) const;
	void getIPAddress(CString *ip) const;
	void setIPAddress(uint32 ip);

	uint16 getTCPPort(void) const;
	void getTCPPort(CString *port) const;
	void setTCPPort(uint16 port);

	uint16 getUDPPort(void) const;
	void getUDPPort(CString *port) const;
	void setUDPPort(uint16 port);

	byte getType(void) const;
	void updateType();
	void checkingType();

	bool getGuiRefs(void) const { return m_guiRefs; }
	void setGuiRefs(bool refs) { m_guiRefs = refs; }

	bool inUse(void) {return (m_inUse>0);}
	void incUse(void) {m_inUse++;}
	void decUse(void) {if(m_inUse)m_inUse--;else ASSERT(0);}

	time_t getCreatedTime() const {return m_created;}

	time_t getExpireTime() const {return m_expires;}

	time_t getLastTypeSet() const {return m_lastTypeSet;}
//KTS+ IP to Country
	CString	GetCountryName() const;
	int		GetCountryFlagIndex() const;
	void	ResetIP2Country();
	uint32		m_ip;
	struct	IPRange_Struct2* m_structServerCountry;
//KTS- IP to Country
private:
	void initContact(); // Common var initialization goes here
	CUInt128	m_clientID;
	CUInt128	m_distance;
	//uint32		m_ip;
	uint16		m_tcpPort;
	uint16		m_udpPort;
	byte		m_type;
	bool		m_guiRefs;
	time_t		m_lastTypeSet;
	time_t		m_expires;
	uint32		m_inUse;
	time_t		m_created;
};

} // End namespace