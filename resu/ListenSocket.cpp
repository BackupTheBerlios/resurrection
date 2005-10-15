//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "DebugHelpers.h"
#include "emule.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "opcodes.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "IPFilter.h"
#include "SharedFileList.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "ServerList.h"
#include "Server.h"
#include "Sockets.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "ClientListCtrl.h"
#include "ChatWnd.h"
#include "PeerCacheFinder.h"
#include "Exceptions.h"
#include "Kademlia/Utils/uint128.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/Kademlia/prefs.h"
#include "ClientUDPSocket.h"
#include "SHAHashSet.h"
#include "Log.h"


// MORPH START - Added by Commander, WebCache 1.2e
#include "WebCache/WebCacheSocket.h"
#include "WebCache/WebCachedBlock.h"
#include "WebCache/WebCacheProxyClient.h"
#include "WebCache/WebCachedBlockList.h"
#include "WebCache/WebCacheOHCBManager.h"
// MORPH END - Added by Commander, WebCache 1.2e

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CClientReqSocket

IMPLEMENT_DYNCREATE(CClientReqSocket, CEMSocket)

CClientReqSocket::CClientReqSocket(CUpDownClient* in_client)
{
	SetClient(in_client);
	theApp.listensocket->AddSocket(this);
	ResetTimeOutTimer();
	deletethis = false;
	deltimer = 0;
	m_bPortTestCon=false;
	m_nOnConnect=SS_Other;
}

void CClientReqSocket::SetConState( SocketState val )
{
	//If no change, do nothing..
	if( val == m_nOnConnect )
		return;
	//Decrease count of old state..
	switch( m_nOnConnect )
	{
		case SS_Half:
			theApp.listensocket->m_nHalfOpen--;
			break;
		case SS_Complete:
			theApp.listensocket->m_nComp--;
	}
	//Set state to new state..
	m_nOnConnect = val;
	//Increase count of new state..
	switch( m_nOnConnect )
	{
		case SS_Half:
			theApp.listensocket->m_nHalfOpen++;
			break;
		case SS_Complete:
			theApp.listensocket->m_nComp++;
	}
}

void CClientReqSocket::WaitForOnConnect()
{
	SetConState(SS_Half);
}
	
CClientReqSocket::~CClientReqSocket()
{
	//This will update our statistics.
	SetConState(SS_Other);
	//Xman
	// Maella -Code Fix-
	if(client != NULL && client->socket == this){
		client->socket = NULL;
	}
	client = NULL;
	// Maella End
	theApp.listensocket->RemoveSocket(this);

	DEBUG_ONLY (theApp.clientlist->Debug_SocketDeleted(this));
}

void CClientReqSocket::SetClient(CUpDownClient* pClient)
{
	client = pClient;
	if (client)
		client->socket = this;
}

void CClientReqSocket::ResetTimeOutTimer(){
	timeout_timer = ::GetTickCount();
}

UINT CClientReqSocket::GetTimeOut()
{
	// yonatan http: Added WC stuff to CWebCacheSocket::ResetTimeOutTimer() - WC-TODO ?
	
	// PC-TODO
	// the PC socket may even already be disconnected and deleted and we still need to keep the
	// ed2k socket open because remote client may still be downloading from cache.
	if (client && client->IsUploadingToPeerCache() && (client->m_pPCUpSocket == NULL || !client->m_pPCUpSocket->IsConnected()))
	{
		// we are uploading (or at least allow uploading) but currently no socket
		return max(CEMSocket::GetTimeOut(), GetPeerCacheSocketUploadTimeout());
	}
	else if (client && client->m_pPCUpSocket && client->m_pPCUpSocket->IsConnected())
	{
		// we have an uploading PC socket, but that socket is not used (nor can it be closed)
		return max(CEMSocket::GetTimeOut(), client->m_pPCUpSocket->GetTimeOut());
	}
	else if (client && client->m_pPCDownSocket && client->m_pPCDownSocket->IsConnected())
	{
		// we have a downloading PC socket
		return max(CEMSocket::GetTimeOut(), client->m_pPCDownSocket->GetTimeOut());
	}
	else
		return CEMSocket::GetTimeOut();
}

bool CClientReqSocket::CheckTimeOut()
{
	if(m_nOnConnect == SS_Half)
	{
		//This socket is still in a half connection state.. Because of SP2, we don't know
		//if this socket is actually failing, or if this socket is just queued in SP2's new
		//protection queue. Therefore we give the socket a chance to either finally report
		//the connection error, or finally make it through SP2's new queued socket system..
		if (::GetTickCount() - timeout_timer > CEMSocket::GetTimeOut()*4){
			timeout_timer = ::GetTickCount();
			CString str;
			str.Format(_T("Timeout: State:%u"), m_nOnConnect);
			Disconnect(str);
			return true;
		}
		return false;
	}
	UINT uTimeout = GetTimeOut();
	if(client)
	{
		if (client->GetKadState() == KS_CONNECTED_BUDDY)
		{
			//We originally ignored the timeout here for buddies.
			//This was a stupid idea on my part. There is now a ping/pong system
			//for buddies. This ping/pong system now prevents timeouts.
			//This release will allow lowID clients with KadVersion 0 to remain connected.
			//But a soon future version needs to allow these older clients to time out to prevent dead connections from continuing.
			//JOHNTODO: Don't forget to remove backward support in a future release.
			if( client->GetKadVersion() == 0 )
				return false;
			uTimeout += MIN2MS(15);
		}
		if (client->GetChatState()!=MS_NONE)
		{
			//We extend the timeout time here to avoid people chatting from disconnecting to fast.
			uTimeout += CONNECTION_TIMEOUT;
		}
	}
	if (::GetTickCount() - timeout_timer > uTimeout){
		timeout_timer = ::GetTickCount();
		CString str;
		str.Format(_T("Timeout: State:%u"), m_nOnConnect);
		Disconnect(str);
		return true;
	}
	return false;
}

void CClientReqSocket::OnClose(int nErrorCode){
	ASSERT (theApp.listensocket->IsValidSocket(this));
	CEMSocket::OnClose(nErrorCode);

	LPCTSTR pszReason;
	CString* pstrReason = NULL;
	if (nErrorCode == 0)
		pszReason = _T("Close");
	else if (thePrefs.GetVerbose()){
		pstrReason = new CString;
		*pstrReason = GetErrorMessage(nErrorCode, 1);
		pszReason = *pstrReason;
	}
	else
		pszReason = NULL;
	Disconnect(pszReason);
	delete pstrReason;
}

void CClientReqSocket::Disconnect(LPCTSTR pszReason){
	AsyncSelect(0);
	byConnected = ES_DISCONNECTED;
	if (!client)
		Safe_Delete();
	else
		if(client->Disconnected(pszReason, true)){
			CUpDownClient* temp = client;
			client->socket = NULL;
			client = NULL;
			delete temp;
			Safe_Delete();
		}
		else{
			client = NULL;
			Safe_Delete();
		}
};

void CClientReqSocket::Delete_Timed(){
// it seems that MFC Sockets call socketfunctions after they are deleted, even if the socket is closed
// and select(0) is set. So we need to wait some time to make sure this doesn't happens
	if (::GetTickCount() - deltimer > 10000)
		delete this;
}

//Xman
// - Maella -Code Fix-
void CClientReqSocket::Safe_Delete()
{
	ASSERT (theApp.listensocket->IsValidSocket(this));
	AsyncSelect(0);
	deltimer = ::GetTickCount();
	if (m_SocketData.hSocket != INVALID_SOCKET) // deadlake PROXYSUPPORT - changed to AsyncSocketEx
		ShutDown(SD_BOTH);
	if(client != NULL && client->socket == this){
		// They might have an error in the cross link somewhere
		client->socket = NULL;
	}
	client = NULL;
	byConnected = ES_DISCONNECTED;
	deletethis = true;
}
// Maella end

bool CClientReqSocket::ProcessPacket(const BYTE* packet, uint32 size, UINT opcode)
{
	try
	{
		try
		{
			if (!client && opcode != OP_HELLO)
			{
				theStats.AddDownDataOverheadOther(size);
				throw GetResString(IDS_ERR_NOHELLO);
			}
			else if (client && opcode != OP_HELLO && opcode != OP_HELLOANSWER)
				client->CheckHandshakeFinished(OP_EDONKEYPROT, opcode);
			switch(opcode)
			{
				case OP_HELLOANSWER:
				{
					theStats.AddDownDataOverheadOther(size);
					client->ProcessHelloAnswer(packet,size);
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_HelloAnswer", client);
						Debug(_T("  %s\n"), client->DbgGetHelloInfo());
					}

					// start secure identification, if
					//  - we have received OP_EMULEINFO and OP_HELLOANSWER (old eMule)
					//	- we have received eMule-OP_HELLOANSWER (new eMule)
					if (client->GetInfoPacketsReceived() == IP_BOTH)
						client->InfoPacketsReceived();

					if (client)
					{
						client->ConnectionEstablished();
						theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(client);
					}
					break;
				}
				case OP_HELLO:
				{
					theStats.AddDownDataOverheadOther(size);
					//Xman
					// Maella -Code Fix-
					if(client != NULL)
					{
						CString oldHash;
						CString newHash;
						if(size > 17){
							oldHash = (client->GetUserHash() == NULL) ? _T("null") : EncodeBase16(client->GetUserHash(), 16);
							newHash = EncodeBase16((const uchar*)&packet[1], 16);
						}
						if(oldHash != newHash){
							AddLogLine(true,  _T("User %s (client=%s) try to send multiple OP_HELLO, old hash=%s, new has=%s"), client->GetUserName, client->GetClientSoftVer(), oldHash, newHash);
						}
						else {
							AddLogLine(true, _T("User %s (client=%s) try to send multiple OP_HELLO"), client->GetUserName, client->GetClientSoftVer());
						}
						throw CString(_T("Invalid request received"));
					}
					// Maella end
					bool bNewClient = (client == NULL);
					if (bNewClient)
					{
						// create new client to save standart informations
						client = new CUpDownClient(this);
					}

					bool bIsMuleHello = false;
					try
					{
						bIsMuleHello = client->ProcessHelloPacket(packet,size);
					}
					catch(...)
					{
						if (bNewClient)
						{
							// Don't let CUpDownClient::Disconnected be processed for a client which is not in the list of clients.
							delete client;
							client = NULL;
						}
						throw;
					}

					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_Hello", client);
						Debug(_T("  %s\n"), client->DbgGetHelloInfo());
					}

					// now we check if we know this client already. if yes this socket will
					// be attached to the known client, the new client will be deleted
					// and the var. "client" will point to the known client.
					// if not we keep our new-constructed client ;)
					if (theApp.clientlist->AttachToAlreadyKnown(&client,this))
					{
						// update the old client informations
						bIsMuleHello = client->ProcessHelloPacket(packet,size);
					}
					else 
					{
						theApp.clientlist->AddClient(client);
						client->SetCommentDirty();
					}

					theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(client);

					// send a response packet with standart informations
					if (client->GetHashType() == SO_EMULE && !bIsMuleHello)
						client->SendMuleInfoPacket(false);

					client->SendHelloAnswer();

					if (client)
						client->ConnectionEstablished();

					// TODO: How does ConnectionEstablished() delete this client object????
					ASSERT( client );
					if(client)
					{
						// start secure identification, if
						//	- we have received eMule-OP_HELLO (new eMule)
						if (client->GetInfoPacketsReceived() == IP_BOTH)
							client->InfoPacketsReceived();

						if( client->GetKadPort() )
							Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());
					}
					break;
				}
				case OP_REQUESTFILENAME:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileRequest", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (size >= 16)
					{
												if (!client->GetWaitStartTime())
							client->SetWaitStartTime();
					
						CSafeMemFile data_in(packet, size);
						uchar reqfilehash[16];
						data_in.ReadHash16(reqfilehash);

						CKnownFile* reqfile;
						if ( (reqfile = theApp.sharedfiles->GetFileByID(reqfilehash)) == NULL ){
							if ( !((reqfile = theApp.downloadqueue->GetFileByID(reqfilehash)) != NULL
								   && reqfile->GetFileSize() > PARTSIZE) )
							{
								client->CheckFailedFileIdReqs(reqfilehash);
								break;
							}
						}

						// if we are downloading this file, this could be a new source
						// no passive adding of files with only one part
						if (reqfile->IsPartFile() && reqfile->GetFileSize() > PARTSIZE)
						{
							if (((CPartFile*)reqfile)->GetFileHardLimit() > ((CPartFile*)reqfile)->GetSourceCount()) //>>> WiZaRd - AutoHL added by lama	
								theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
						}

						// check to see if this is a new file they are asking for
						if (md4cmp(client->GetUploadFileID(), reqfilehash) != 0)
							client->SetCommentDirty();

						client->SetUploadFileID(reqfile);
						client->ProcessExtendedInfo(&data_in, reqfile);
						
						// send filename etc
						CSafeMemFile data_out(128);
						data_out.WriteHash16(reqfile->GetFileHash());
						data_out.WriteString(reqfile->GetFileName(), client->GetUnicodeSupport());
						Packet* packet = new Packet(&data_out);
						packet->opcode = OP_REQFILENAMEANSWER;
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileReqAnswer", client, reqfile->GetFileHash());
						theStats.AddUpDataOverheadFileRequest(packet->size);
						SendPacket(packet, true);
						
						client->SendCommentInfo(reqfile);
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
				case OP_SETREQFILEID:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_SetReqFileID", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (size == 16)
					{
												if (!client->GetWaitStartTime())
							client->SetWaitStartTime();
						
						CKnownFile* reqfile;
						if ( (reqfile = theApp.sharedfiles->GetFileByID(packet)) == NULL ){
							if ( !((reqfile = theApp.downloadqueue->GetFileByID(packet)) != NULL
								   && reqfile->GetFileSize() > PARTSIZE) )
							{
								// send file request no such file packet (0x48)
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugSend("OP__FileReqAnsNoFil", client, packet);
								Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
								md4cpy(replypacket->pBuffer, packet);
								theStats.AddUpDataOverheadFileRequest(replypacket->size);
								SendPacket(replypacket, true);
								client->CheckFailedFileIdReqs(packet);
								break;
							}
						}

						// check to see if this is a new file they are asking for
						if (md4cmp(client->GetUploadFileID(), packet) != 0)
							client->SetCommentDirty();

						client->SetUploadFileID(reqfile);

						// send filestatus
						CSafeMemFile data(16+16);
						data.WriteHash16(reqfile->GetFileHash());
						if (reqfile->IsPartFile())
							((CPartFile*)reqfile)->WritePartStatus(&data);
						else
							data.WriteUInt16(0);
						Packet* packet = new Packet(&data);
						packet->opcode = OP_FILESTATUS;
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileStatus", client, reqfile->GetFileHash());
						theStats.AddUpDataOverheadFileRequest(packet->size);
						SendPacket(packet, true);
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
				case OP_FILEREQANSNOFIL:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileReqAnsNoFil", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					if (size == 16)
					{
						CPartFile* reqfile = theApp.downloadqueue->GetFileByID(packet);
						if (!reqfile){
							client->CheckFailedFileIdReqs(packet);
							break;
						}
						//KTS+ remove deadsource
						//else
						//	reqfile->m_DeadSourceList.AddDeadSource(client);
						//KTS- remove deadsource
						// if that client does not have my file maybe has another different
						// we try to swap to another file ignoring no needed parts files
						switch (client->GetDownloadState())
						{
							case DS_CONNECTED:
							case DS_ONQUEUE:
							case DS_NONEEDEDPARTS:
                                client->DontSwapTo(client->GetRequestFile()); // ZZ:DownloadManager
                                if (!client->SwapToAnotherFile(_T("Source says it doesn't have the file. CClientReqSocket::ProcessPacket()"), true, true, true, NULL, false, false)) { // ZZ:DownloadManager
    								theApp.downloadqueue->RemoveSource(client);
                                }
							break;
						}
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
				case OP_REQFILENAMEANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileReqAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar cfilehash[16];
					data.ReadHash16(cfilehash);
					CPartFile* file = theApp.downloadqueue->GetFileByID(cfilehash);
					if (file == NULL)
						client->CheckFailedFileIdReqs(cfilehash);
					client->ProcessFileInfo(&data, file);
					break;
				}
				case OP_FILESTATUS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileStatus", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar cfilehash[16];
					data.ReadHash16(cfilehash);
					CPartFile* file = theApp.downloadqueue->GetFileByID(cfilehash);
					if (file == NULL)
						client->CheckFailedFileIdReqs(cfilehash);
					client->ProcessFileStatus(false, &data, file);
					break;
				}
				case OP_STARTUPLOADREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_StartUpLoadReq", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					if (!client->CheckHandshakeFinished(OP_EDONKEYPROT, opcode))
						break;
					if (size == 16)
					{
						CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(packet);
						if (reqfile)
						{
							if (md4cmp(client->GetUploadFileID(), packet) != 0)
								client->SetCommentDirty();
							client->SetUploadFileID(reqfile);
							client->SendCommentInfo(reqfile);
							theApp.uploadqueue->AddClientToQueue(client);
						}
						else
							client->CheckFailedFileIdReqs(packet);
					}
					break;
				}
				case OP_QUEUERANK:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_QueueRank", client);
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessEdonkeyQueueRank(packet, size);
					break;
				}
				case OP_ACCEPTUPLOADREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_AcceptUploadReq", client, (size >= 16) ? packet : NULL);
						if (size > 0)
							Debug(_T("  ***NOTE: Packet contains %u additional bytes\n"), size);
						Debug(_T("  QR=%d\n"), client->IsRemoteQueueFull() ? (UINT)-1 : (UINT)client->GetRemoteQueueRank());
					}
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessAcceptUpload();
					break;
				}
				case OP_REQUESTPARTS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestParts", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar reqfilehash[16];
					data.ReadHash16(reqfilehash);

					uint32 auStartOffsets[3];
					auStartOffsets[0] = data.ReadUInt32();
					auStartOffsets[1] = data.ReadUInt32();
					auStartOffsets[2] = data.ReadUInt32();

					uint32 auEndOffsets[3];
					auEndOffsets[0] = data.ReadUInt32();
					auEndOffsets[1] = data.ReadUInt32();
					auEndOffsets[2] = data.ReadUInt32();

					if (thePrefs.GetDebugClientTCPLevel() > 0){
						Debug(_T("  Start1=%u  End1=%u  Size=%u\n"), auStartOffsets[0], auEndOffsets[0], auEndOffsets[0] - auStartOffsets[0]);
						Debug(_T("  Start2=%u  End2=%u  Size=%u\n"), auStartOffsets[1], auEndOffsets[1], auEndOffsets[1] - auStartOffsets[1]);
						Debug(_T("  Start3=%u  End3=%u  Size=%u\n"), auStartOffsets[2], auEndOffsets[2], auEndOffsets[2] - auStartOffsets[2]);
					}

					for (int i = 0; i < ARRSIZE(auStartOffsets); i++)
					{
						if (auEndOffsets[i] > auStartOffsets[i])
						{
							Requested_Block_Struct* reqblock = new Requested_Block_Struct;
							reqblock->StartOffset = auStartOffsets[i];
							reqblock->EndOffset = auEndOffsets[i];
							md4cpy(reqblock->FileID, reqfilehash);
							reqblock->transferred = 0;
							client->AddReqBlock(reqblock);
						}
						else
						{
							if (thePrefs.GetVerbose())
							{
								if (auEndOffsets[i] != 0 || auStartOffsets[i] != 0)
									DebugLogWarning(_T("Client requests invalid %u. file block %u-%u (%d bytes): %s"), i, auStartOffsets[i], auEndOffsets[i], auEndOffsets[i] - auStartOffsets[i], client->DbgGetClientInfo());
							}
						}
					}
					break;
				}
				case OP_CANCELTRANSFER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_CancelTransfer", client);
					theStats.AddDownDataOverheadFileRequest(size);
					theApp.uploadqueue->RemoveFromUploadQueue(client, _T("Remote client canceled transfer."));
					break;
				}
				case OP_END_OF_DOWNLOAD:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_EndOfDownload", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					if (size>=16 && !md4cmp(client->GetUploadFileID(),packet))
						theApp.uploadqueue->RemoveFromUploadQueue(client, _T("Remote client ended transfer."));
					else
						client->CheckFailedFileIdReqs(packet);
					break;
				}
				case OP_HASHSETREQUEST:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetReq", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (size != 16)
						throw GetResString(IDS_ERR_WRONGHPACKAGESIZE);
					client->SendHashsetPacket(packet);
					break;
				}
				case OP_HASHSETANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessHashSet(packet,size);
					break;
				}
				case OP_SENDINGPART:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 1)
						DebugRecv("OP_SendingPart", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(24);
					if (client->GetRequestFile() && !client->GetRequestFile()->IsStopped() && (client->GetRequestFile()->GetStatus()==PS_READY || client->GetRequestFile()->GetStatus()==PS_EMPTY))
					{
						client->ProcessBlockPacket(packet,size);
						if (client->GetRequestFile()->IsStopped() || client->GetRequestFile()->GetStatus()==PS_PAUSED || client->GetRequestFile()->GetStatus()==PS_ERROR)
						{
							client->SendCancelTransfer();
							client->SetDownloadState(client->GetRequestFile()->IsStopped() ? DS_NONE : DS_ONQUEUE);
						}
					}
					else
					{
						client->SendCancelTransfer();
						client->SetDownloadState((client->GetRequestFile()==NULL || client->GetRequestFile()->IsStopped()) ? DS_NONE : DS_ONQUEUE);
					}
					break;
				}
				case OP_OUTOFPARTREQS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_OutOfPartReqs", client);
					theStats.AddDownDataOverheadFileRequest(size);
					if (client->GetDownloadState() == DS_DOWNLOADING)
					{
						client->SetDownloadState(DS_ONQUEUE, _T("The remote client decided to stop/complete the transfer (got OP_OutOfPartReqs)."));
					}
					break;
				}
				case OP_CHANGE_CLIENT_ID:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ChangedClientID", client);
					theStats.AddDownDataOverheadOther(size);

					CSafeMemFile data(packet, size);
					uint32 nNewUserID = data.ReadUInt32();
					uint32 nNewServerIP = data.ReadUInt32();
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						Debug(_T("  NewUserID=%u (%08x, %s)  NewServerIP=%u (%08x, %s)\n"), nNewUserID, nNewUserID, ipstr(nNewUserID), nNewServerIP, nNewServerIP, ipstr(nNewServerIP));
					if (IsLowID(nNewUserID))
					{	// client changed server and has a LowID
						CServer* pNewServer = theApp.serverlist->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							client->SetUserIDHybrid(nNewUserID); // update UserID only if we know the server
							client->SetServerIP(nNewServerIP);
							client->SetServerPort(pNewServer->GetPort());
						}
					}
					else if (nNewUserID == client->GetIP())
					{	// client changed server and has a HighID(IP)
						client->SetUserIDHybrid(ntohl(nNewUserID));
						CServer* pNewServer = theApp.serverlist->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							client->SetServerIP(nNewServerIP);
							client->SetServerPort(pNewServer->GetPort());
						}
					}
					else{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							Debug(_T("***NOTE: OP_ChangedClientID unknown contents\n"));
					}
					UINT uAddData = (UINT)(data.GetLength() - data.GetPosition());
					if (uAddData > 0){
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							Debug(_T("***NOTE: OP_ChangedClientID contains add. data %s\n"), DbgGetHexDump(packet + data.GetPosition(), uAddData));
					}
					break;
				}
				case OP_CHANGE_SLOT:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ChangeSlot", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					// sometimes sent by Hybrid
					break;
				}
				case OP_MESSAGE:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Message", client);
					theStats.AddDownDataOverheadOther(size);
					
					if (size < 2)
						throw CString(_T("invalid message packet"));
					CSafeMemFile data(packet, size);
					UINT length = data.ReadUInt16();
					if (length+2 != size)
						throw CString(_T("invalid message packet"));
					
					//filter me?
					if ( (thePrefs.MsgOnlyFriends() && !client->IsFriend()) || (thePrefs.MsgOnlySecure() && client->GetUserName()==NULL) )
					{
						if (!client->GetMessageFiltered()){
							if (thePrefs.GetVerbose())
								AddDebugLogLine(false,_T("Filtered Message from '%s' (IP:%s)"), client->GetUserName(), ipstr(client->GetConnectIP()));
						}
						client->SetMessageFiltered(true);
						break;
					}

					if (length > MAX_CLIENT_MSG_LEN){
						if (thePrefs.GetVerbose())
							AddDebugLogLine(false, _T("Message from '%s' (IP:%s) exceeds limit by %u chars, truncated."), client->GetUserName(), ipstr(client->GetConnectIP()), length - MAX_CLIENT_MSG_LEN);
						length = MAX_CLIENT_MSG_LEN;
					}

					AddLogLine(true, GetResString(IDS_NEWMSG), client->GetUserName(), ipstr(client->GetConnectIP()));

					CString strMessage(data.ReadString(client->GetUnicodeSupport()!=utf8strNone, length));
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						Debug(_T("  %s\n"), strMessage);
					theApp.emuledlg->chatwnd->chatselector.ProcessMessage(client, strMessage);
					break;
				}
				case OP_ASKSHAREDFILES:
				{	
					// client wants to know what we have in share, let's see if we allow him to know that
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFiles", client);
					theStats.AddDownDataOverheadOther(size);

					//KTS+
					// Dark :: Flood Ask Protection ("Geaiez")					
					if ( client->GetClientSoft() == SO_MLDONKEY	||(_tcsnicmp(client->GetUserName(), _T("geaiez"),5)==0))
					{
					    client->Ban();
						/*if(thePrefs.IsLogLeecher())
							AddModLogLine(false, _T("Ban :  %s (%s), Banreason: Flood AskShareFiles"), client->GetUserName(), ipstr(client->GetConnectIP()));
						*/break;
					}
					//KTS-
					CPtrList list;
					if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
						CCKey bufKey;
						CKnownFile* cur_file;
						for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition();pos != 0;)
						{
							theApp.sharedfiles->m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
							//FRTK(kts)+
							// xMule_MOD: showSharePermissions - filter out hidden files
							if ( cur_file->GetPermissions() == PERM_NOONE
								|| (cur_file->GetPermissions() == PERM_FRIENDS && !client->IsFriend()) )
								continue;
							// xMule_MOD: showSharePermissions
							//FRTK(kts)-
							list.AddTail((void*&)cur_file);
						}
						AddLogLine(true, GetResString(IDS_REQ_SHAREDFILES), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_ACCEPTED));
					}
					else
					{
						AddLogLine(true, GetResString(IDS_REQ_SHAREDFILES), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_DENIED));
					}

					// now create the memfile for the packet
					uint32 iTotalCount = list.GetCount();
					CSafeMemFile tempfile(80);
					tempfile.WriteUInt32(iTotalCount);
					while (list.GetCount())
					{
						theApp.sharedfiles->CreateOfferedFilePacket((CKnownFile*)list.GetHead(), &tempfile, NULL, client);
						list.RemoveHead();
					}

					// create a packet and send it
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__AskSharedFilesAnswer", client);
					Packet* replypacket = new Packet(&tempfile);
					replypacket->opcode = OP_ASKSHAREDFILESANSWER;
					theStats.AddUpDataOverheadOther(replypacket->size);
					SendPacket(replypacket, true, true);
					break;
				}
				case OP_ASKSHAREDFILESANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesAnswer", client);
					theStats.AddDownDataOverheadOther(size);
					client->ProcessSharedFileList(packet,size);
					break;
				}
                case OP_ASKSHAREDDIRS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDirectories", client);
                    theStats.AddDownDataOverheadOther(size);

                    if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
						AddLogLine(true, GetResString(IDS_SHAREDREQ1), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_ACCEPTED));

						//TODO: Don't send shared directories which do not contain any files
						// add shared directories
						CString strDir;
						CStringArray arFolders;
                        POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
                        while (pos)
						{
                            strDir = thePrefs.shareddir_list.GetNext(pos);
                            PathRemoveBackslash(strDir.GetBuffer());
                            strDir.ReleaseBuffer();
							bool bFoundFolder = false;
							for (int i = 0; i < arFolders.GetCount(); i++)
							{
								if (strDir.CompareNoCase(arFolders.GetAt(i)) == 0)
								{
									bFoundFolder = true;
									break;
								}
							}
							if (!bFoundFolder)
								arFolders.Add(strDir);
						}

						// add incoming folders
                       	for (int iCat = 0; iCat < thePrefs.GetCatCount(); iCat++)
						{
							strDir = thePrefs.GetCategory(iCat)->incomingpath;
							PathRemoveBackslash(strDir.GetBuffer());
							strDir.ReleaseBuffer();
							bool bFoundFolder = false;
							for (int i = 0; i < arFolders.GetCount(); i++)
							{
								if (strDir.CompareNoCase(arFolders.GetAt(i)) == 0)
								{
									bFoundFolder = true;
									break;
								}
							}
							if (!bFoundFolder)
								arFolders.Add(strDir);
						}

						// add temporary folder
						strDir = OP_INCOMPLETE_SHARED_FILES;
						bool bFoundFolder = false;
						for (int i = 0; i < arFolders.GetCount(); i++)
						{
							if (strDir.CompareNoCase(arFolders.GetAt(i)) == 0)
							{
								bFoundFolder = true;
								break;
							}
						}
						if (!bFoundFolder)
							arFolders.Add(strDir);

						// build packet
                        CSafeMemFile tempfile(80);
                        tempfile.WriteUInt32(arFolders.GetCount());
						for (int i = 0; i < arFolders.GetCount(); i++)
                            tempfile.WriteString(arFolders.GetAt(i), client->GetUnicodeSupport());

						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDirsAnswer", client);
						Packet* replypacket = new Packet(&tempfile);
                        replypacket->opcode = OP_ASKSHAREDDIRSANS;
                        theStats.AddUpDataOverheadOther(replypacket->size);
                        SendPacket(replypacket, true, true);
					}
					else
					{
						AddLogLine(true, GetResString(IDS_SHAREDREQ1), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_DENIED));
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDeniedAnswer", client);
                        Packet* replypacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);
						theStats.AddUpDataOverheadOther(replypacket->size);
                        SendPacket(replypacket, true, true);
                    }
                    break;
                }
                case OP_ASKSHAREDFILESDIR:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesInDirectory", client);
                    theStats.AddDownDataOverheadOther(size);

					CSafeMemFile data(packet, size);
                    CString strReqDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
                    PathRemoveBackslash(strReqDir.GetBuffer());
                    strReqDir.ReleaseBuffer();
                    if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
						AddLogLine(true, GetResString(IDS_SHAREDREQ2), client->GetUserName(), client->GetUserIDHybrid(), strReqDir, GetResString(IDS_ACCEPTED));
                        ASSERT( data.GetPosition() == data.GetLength() );
                        CTypedPtrList<CPtrList, CKnownFile*> list;
						if (strReqDir == OP_INCOMPLETE_SHARED_FILES)
						{
							// get all shared files from download queue
							int iQueuedFiles = theApp.downloadqueue->GetFileCount();
							for (int i = 0; i < iQueuedFiles; i++)
							{
								CPartFile* pFile = theApp.downloadqueue->GetFileByIndex(i);
								if (pFile == NULL || pFile->GetStatus(true) != PS_READY)
									break;
								list.AddTail(pFile);
							}
						}
						else
						{
							// get all shared files from requested directory
							for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition();pos != 0;)
							{
								CCKey bufKey;
								CKnownFile* cur_file;
								theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
								//FRTK(kts)+
								// xMule_MOD: showSharePermissions - filter out hidden files
								if ( cur_file->GetPermissions() == PERM_NOONE 
									|| (cur_file->GetPermissions() == PERM_FRIENDS && !client->IsFriend()) )
									continue;
								// xMule_MOD: showSharePermissions
								//FRTK(kts)-
								CString strSharedFileDir(cur_file->GetPath());
								PathRemoveBackslash(strSharedFileDir.GetBuffer());
								strSharedFileDir.ReleaseBuffer();
								if (strReqDir.CompareNoCase(strSharedFileDir) == 0)
									list.AddTail(cur_file);
							}
						}

						// Currently we are sending each shared directory, even if it does not contain any files.
						// Because of this we also have to send an empty shared files list..
						CSafeMemFile tempfile(80);
						tempfile.WriteString(strReqDir, client->GetUnicodeSupport());
						tempfile.WriteUInt32(list.GetCount());
						while (list.GetCount())
						{
							theApp.sharedfiles->CreateOfferedFilePacket(list.GetHead(), &tempfile, NULL, client);
							list.RemoveHead();
						}

						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedFilesInDirectoryAnswer", client);
						Packet* replypacket = new Packet(&tempfile);
						replypacket->opcode = OP_ASKSHAREDFILESDIRANS;
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket, true, true);
					}
                    else
					{
						AddLogLine(true, GetResString(IDS_SHAREDREQ2), client->GetUserName(), client->GetUserIDHybrid(), strReqDir, GetResString(IDS_DENIED));
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDeniedAnswer", client);
                        Packet* replypacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);
                        theStats.AddUpDataOverheadOther(replypacket->size);
                        SendPacket(replypacket, true, true);
                    }
                    break;
                }
				case OP_ASKSHAREDDIRSANS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDirectoriesAnswer", client);
                    theStats.AddDownDataOverheadOther(size);
                    if (client->GetFileListRequested() == 1)
					{
						CSafeMemFile data(packet, size);
						UINT uDirs = data.ReadUInt32();
                        for (UINT i = 0; i < uDirs; i++)
						{
                            CString strDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
							// Better send the received and untouched directory string back to that client
							//PathRemoveBackslash(strDir.GetBuffer());
							//strDir.ReleaseBuffer();
							AddLogLine(true, GetResString(IDS_SHAREDANSW), client->GetUserName(), client->GetUserIDHybrid(), strDir);

							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__AskSharedFilesInDirectory", client);
                            CSafeMemFile tempfile(80);
							tempfile.WriteString(strDir, client->GetUnicodeSupport());
                            Packet* replypacket = new Packet(&tempfile);
                            replypacket->opcode = OP_ASKSHAREDFILESDIR;
                            theStats.AddUpDataOverheadOther(replypacket->size);
                            SendPacket(replypacket, true, true);
                        }
                        ASSERT( data.GetPosition() == data.GetLength() );
                        client->SetFileListRequested(uDirs);
                    }
					else
						AddLogLine(true, GetResString(IDS_SHAREDANSW2), client->GetUserName(), client->GetUserIDHybrid());
                    break;
                }
                case OP_ASKSHAREDFILESDIRANS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesInDirectoryAnswer", client);
                    theStats.AddDownDataOverheadOther(size);

					CSafeMemFile data(packet, size);
                    CString strDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
					PathRemoveBackslash(strDir.GetBuffer());
					strDir.ReleaseBuffer();
                    if (client->GetFileListRequested() > 0)
					{
						AddLogLine(true, GetResString(IDS_SHAREDINFO1), client->GetUserName(), client->GetUserIDHybrid(), strDir);
						client->ProcessSharedFileList(packet + (UINT)data.GetPosition(), (UINT)(size - data.GetPosition()), strDir);
						if (client->GetFileListRequested() == 0)
							AddLogLine(true, GetResString(IDS_SHAREDINFO2), client->GetUserName(), client->GetUserIDHybrid());
                    }
					else
						AddLogLine(true, GetResString(IDS_SHAREDANSW3), client->GetUserName(), client->GetUserIDHybrid(), strDir);
                    break;
                }
                case OP_ASKSHAREDDENIEDANS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDeniedAnswer", client);
                    theStats.AddDownDataOverheadOther(size);

					AddLogLine(true, GetResString(IDS_SHAREDREQDENIED), client->GetUserName(), client->GetUserIDHybrid());
					client->SetFileListRequested(0);
                    break;
                }
        		// MORPH START - Added by Commander, WebCache 1.2e
				case OP_HTTP_CACHED_BLOCK:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP__Http_Cached_Block", client);
					theStats.AddDownDataOverheadOther(size);
					if( thePrefs.IsWebCacheDownloadEnabled() && client->SupportsWebCache() ) {
						// CHECK HANDSHAKE?
						if (thePrefs.GetLogWebCacheEvents())
						AddDebugLogLine( false, _T("Received WCBlock - TCP") );
						CWebCachedBlock* newblock = new CWebCachedBlock( (const BYTE*)packet, size, client ); // Starts DL or places block on queue
					}
					break;
				}
                // MORPH END - Added by Commander, WebCache 1.2e
				default:
					theStats.AddDownDataOverheadOther(size);
					PacketToDebugLogLine(_T("eDonkey"), packet, size, opcode, DLP_LOW);
					break;
			}
		}
		catch(CFileException* error)
		{
			error->Delete();
			throw GetResString(IDS_ERR_INVALIDPACKAGE);
		}
		catch(CMemoryException* error)
		{
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch(CClientException* ex) // nearly same as the 'CString' exception but with optional deleting of the client
	{
		if (thePrefs.GetVerbose() && !ex->m_strMsg.IsEmpty())
			DebugLogWarning(_T("%s - while processing eDonkey packet: opcode=%s  size=%u; %s"), ex->m_strMsg, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client && ex->m_bDelete)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet (CClientException): ") + ex->m_strMsg);
		Disconnect(ex->m_strMsg);
		ex->Delete();
		return false;
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose() && !error.IsEmpty()){
			if (opcode == OP_REQUESTFILENAME /*low priority for OP_REQUESTFILENAME*/)
				DebugLogWarning(_T("%s - while processing eDonkey packet: opcode=%s  size=%u; %s"), error, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
			else
				DebugLogWarning(_T("%s - while processing eDonkey packet: opcode=%s  size=%u; %s"), error, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
		}
		if (client)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet (CString exception): ") + error);	
		Disconnect(_T("Error when processing packet.") + error);
		return false;
	}
	return true;
}

bool CClientReqSocket::ProcessExtPacket(const BYTE* packet, uint32 size, UINT opcode, UINT uRawSize)
{
	try
	{
		try
		{
			if (!client && opcode!=OP_PORTTEST)
			{
				theStats.AddDownDataOverheadOther(uRawSize);
				throw GetResString(IDS_ERR_UNKNOWNCLIENTACTION);
			}
			if (thePrefs.m_iDbgHeap >= 2 && opcode!=OP_PORTTEST)
				ASSERT_VALID(client);
			switch(opcode)
			{
                case OP_MULTIPACKET:
				{
					bool webcacheInfoReceived = false; // MORPH - Added by Commander, WebCache 1.2e

					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_MultiPacket", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);

					if( client->GetKadPort() )
						Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());

					CSafeMemFile data_in(packet, size);
					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);

					CKnownFile* reqfile;
					if ( (reqfile = theApp.sharedfiles->GetFileByID(reqfilehash)) == NULL ){
						if ( !((reqfile = theApp.downloadqueue->GetFileByID(reqfilehash)) != NULL
								&& reqfile->GetFileSize() > PARTSIZE) )
						{
							// send file request no such file packet (0x48)
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, packet);
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
							client->CheckFailedFileIdReqs(reqfilehash);
							break;
						}
					}
					
											if (!client->GetWaitStartTime())
							client->SetWaitStartTime();
						
					// if we are downloading this file, this could be a new source
					// no passive adding of files with only one part
					if (reqfile->IsPartFile() && reqfile->GetFileSize() > PARTSIZE)
					{
						if (((CPartFile*)reqfile)->GetFileHardLimit() > ((CPartFile*)reqfile)->GetSourceCount()) //>>> WiZaRd - AutoHL added by lama
							theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
					}

					// check to see if this is a new file they are asking for
					if (md4cmp(client->GetUploadFileID(), reqfilehash) != 0)
						client->SetCommentDirty();

					client->SetUploadFileID(reqfile);

					uint8 opcode_in;
					CSafeMemFile data_out(128);
					data_out.WriteHash16(reqfile->GetFileHash());
					while (data_in.GetLength()-data_in.GetPosition())
					{
						opcode_in = data_in.ReadUInt8();
						switch (opcode_in)
						{
							case OP_REQUESTFILENAME:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqFileName", client, packet);

								client->ProcessExtendedInfo(&data_in, reqfile);
								data_out.WriteUInt8(OP_REQFILENAMEANSWER);
								data_out.WriteString(reqfile->GetFileName(), client->GetUnicodeSupport());
								break;
							}
							case OP_AICHFILEHASHREQ:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPAichFileHashReq", client, packet);

								if (client->IsSupportingAICH() && reqfile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE
									&& reqfile->GetAICHHashset()->HasValidMasterHash())
								{
									data_out.WriteUInt8(OP_AICHFILEHASHANS);
									reqfile->GetAICHHashset()->GetMasterHash().Write(&data_out);
								}
								break;
							}
							case OP_SETREQFILEID:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPSetReqFileID", client, packet);

								data_out.WriteUInt8(OP_FILESTATUS);
								if (reqfile->IsPartFile())
									((CPartFile*)reqfile)->WritePartStatus(&data_out);
								else
									data_out.WriteUInt16(0);
								break;
							}
							//We still send the source packet seperately.. 
							//We could send it within this packet.. If agreeded, I will fix it..
							case OP_REQUESTSOURCES:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqSources", client, packet);

								if (thePrefs.GetDebugSourceExchange())
									AddDebugLogLine(false, _T("SXRecv: Client source request; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());

								//Although this shouldn't happen, it's a just in case to any Mods that mess with version numbers.
								if (client->GetSourceExchangeVersion() > 1)
								{
									//data_out.WriteUInt8(OP_ANSWERSOURCES);
									DWORD dwTimePassed = ::GetTickCount() - client->GetLastSrcReqTime() + CONNECTION_LATENCY;
									bool bNeverAskedBefore = client->GetLastSrcReqTime() == 0;
									if (
										//if not complete and file is rare
										(    reqfile->IsPartFile()
										  && (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS)
										  && ((CPartFile*)reqfile)->GetSourceCount() <= RARE_FILE
										) ||
										//OR if file is not rare or if file is complete
										(bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS * MINCOMMONPENALTY)
									   )
									{
										client->SetLastSrcReqTime();
										Packet* tosend = reqfile->CreateSrcInfoPacket(client);
										if (tosend)
										{
											if (thePrefs.GetDebugClientTCPLevel() > 0)
												DebugSend("OP__AnswerSources", client, reqfile->GetFileHash());
											theStats.AddUpDataOverheadSourceExchange(tosend->size);
											SendPacket(tosend, true);
										}
									}
									/*else
									{
										if (thePrefs.GetVerbose())
											AddDebugLogLine(false, _T("RCV: Source Request to fast. (This is testing the new timers to see how much older client will not receive this)"));
									}*/
								}
								break;
							}
							// MORPH START - Added by Commander, WebCache 1.2e                                
							// Superlexx - webcache - moved WC_TAG_WEBCACHENAME, WC_TAG_WEBCACHEID and WC_TAG_MASTERKEY here from the hello packet
							case WC_TAG_WEBCACHENAME:
							{
								CString webcachename = data_in.ReadString( false ); // shouldn't these always be ASCII?
								if( client->SupportsWebCache() )
								{
									webcacheInfoReceived = true;
									webcachename.Trim();
									webcachename.MakeLower();
									if ( webcachename.IsEmpty() )
										client->m_WA_webCacheIndex = -1;
									else
									{
										int index = GetWebCacheIndex(webcachename);
										client->m_WA_webCacheIndex = (index >= 0) ? index : AddWebCache(webcachename);
									}
								}
								break;
							}
							/*case WC_TAG_WEBCACHEID:
							{
								uint32 tmpID;
								tmpID = data_in.ReadUInt32();
								if( client->SupportsWebCache() )
								{
									webcacheInfoReceived = true;
									client->m_uWebCacheDownloadId = tmpID;
								}
								break;
							}*/
							case WC_TAG_MASTERKEY:
							{
								byte tmpKey[WC_KEYLENGTH];
								data_in.Read( tmpKey, WC_KEYLENGTH );
								if( client->SupportsWebCache() )
								{
									webcacheInfoReceived = true;
									for(int i=0;i<WC_KEYLENGTH;i++)
										client->Crypt.localMasterKey[i] = tmpKey[i];

									byte tmpID[4];
									for (int i=0; i<4; i++)
										tmpID[i] = client->Crypt.localMasterKey[i] ^ (client->GetUserHash())[i];
									client->m_uWebCacheDownloadId = *((uint32*)tmpID);
								}
								break;
							}
/*							case WC_TAG_REQ_OHCBS:
								{
									if (!client->SupportsMultiOHCBs() || !client->SupportsWebCache())
										break;
									client->requestedFiles.RemoveAll();
									client->requestedFiles.AddFiles(&data_in);
									
									Packet* tosend = WC_OHCBManager.GetWCBlocksForClient(client);
									if(tosend)
									{
										if (thePrefs.GetDebugClientTCPLevel() > 0)
											DebugSend("OP__multiOHCB", client, (char*)reqfile->GetFileHash());
										theStats.AddUpDataOverheadOther(tosend->size);
										SendPacket(tosend, true);
									}
								}
								break;*/
							// Superlexx - webcache - end
							// MORPH END - Added by Commander, WebCache 1.2e
							default:
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									Debug(_T("***NOTE: Invalid sub opcode 0x%02x with OP_MultiPacket received; %s"), opcode_in, client->DbgGetClientInfo());
						}
					}

					// MORPH START - Added by Commander, WebCache 1.2e
					// Superlexx - webcache - send webcacheInfo in the answer when needed
					if( client->SupportsWebCache() && (client->WebCacheInfoNeeded() || webcacheInfoReceived))
					{
						data_out.WriteUInt8(WC_TAG_WEBCACHENAME);
						data_out.WriteString(thePrefs.webcacheName);
						
						/*data_out.WriteUInt8(WC_TAG_WEBCACHEID);
						client->m_uWebCacheUploadId = GetRandomUInt32();
						data_out.WriteUInt32(client->m_uWebCacheUploadId);*/
						
						data_out.WriteUInt8(WC_TAG_MASTERKEY);
						data_out.Write( client->Crypt.remoteMasterKey, WC_KEYLENGTH );

						byte tmpID[4];
						for (int i=0; i<4; i++)
							tmpID[i] = client->Crypt.remoteMasterKey[i] ^ (thePrefs.GetUserHash())[i];
						client->m_uWebCacheUploadId = *((uint32*)tmpID);
						
						client->SetWebCacheInfoNeeded(false);
					}
					// MORPH END - Added by Commander, WebCache 1.2e

					if (data_out.GetLength() > 16)
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__MultiPacketAns", client, reqfile->GetFileHash());
						Packet* reply = new Packet(&data_out, OP_EMULEPROT);
						reply->opcode = OP_MULTIPACKETANSWER;
						theStats.AddUpDataOverheadFileRequest(reply->size);
						SendPacket(reply, true);
					}
					break;
				}
				case OP_MULTIPACKETANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_MultiPacketAns", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);

					if( client->GetKadPort() )
						Kademlia::CKademlia::bootstrap(ntohl(client->GetIP()), client->GetKadPort());

					CSafeMemFile data_in(packet, size);
					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);
					CPartFile* reqfile = theApp.downloadqueue->GetFileByID(reqfilehash);
					//Make sure we are downloading this file.
					if (reqfile==NULL){
						client->CheckFailedFileIdReqs(reqfilehash);
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; reqfile==NULL)");
					}
					if (client->GetRequestFile()==NULL)
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; client->GetRequestFile()==NULL)");
					if (reqfile != client->GetRequestFile())
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; reqfile!=client->GetRequestFile())");
					uint8 opcode_in;
					while(data_in.GetLength()-data_in.GetPosition())
					{
						opcode_in = data_in.ReadUInt8();
						switch(opcode_in)
						{
							case OP_REQFILENAMEANSWER:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqFileNameAns", client, packet);

								client->ProcessFileInfo(&data_in, reqfile);
								break;
							}
							case OP_FILESTATUS:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPFileStatus", client, packet);

								client->ProcessFileStatus(false, &data_in, reqfile);
								break;
							}
							case OP_AICHFILEHASHANS:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPAichFileHashAns", client);
								
								client->ProcessAICHFileHash(&data_in, reqfile);
								break;
							}
							//KTS+ webcache
                            // WebCache ////////////////////////////////////////////////////////////////////////////////////
							// Superlexx - webcache - moved WC_TAG_WEBCACHENAME, WC_TAG_WEBCACHEID and WC_TAG_MASTERKEY here from the hello packet
							case WC_TAG_WEBCACHENAME:
							{
								CString webcachename = data_in.ReadString(false);
								if( client->SupportsWebCache() )
								{
									webcachename.Trim();
									webcachename.MakeLower();
									if ( webcachename.IsEmpty() )
										client->m_WA_webCacheIndex = -1;
									else
									{
										int index = GetWebCacheIndex(webcachename);
										client->m_WA_webCacheIndex = (index >= 0) ? index : AddWebCache(webcachename);
									}
								}
								break;
							}
							/*case WC_TAG_WEBCACHEID:
							{
								client->m_uWebCacheDownloadId = data_in.ReadUInt32();
								if( !client->SupportsWebCache() )
									client->m_uWebCacheDownloadId = 0;
								break;
							}*/
							case WC_TAG_MASTERKEY:
							{
								byte tmpKey[WC_KEYLENGTH];
								data_in.Read( tmpKey, WC_KEYLENGTH );
								//data_in.Read( client->Crypt.remoteMasterKey, WC_KEYLENGTH );
								if( client->SupportsWebCache() )
								{
									for(int i=0;i<WC_KEYLENGTH;i++)
										client->Crypt.localMasterKey[i] = tmpKey[i];

									byte tmpID[4];
									for (int i=0; i<4; i++)
										tmpID[i] = client->Crypt.localMasterKey[i] ^ (client->GetUserHash())[i];
									client->m_uWebCacheDownloadId = *((uint32*)tmpID);
								}
								break;
							}
/*							case WC_TAG_REQ_OHCBS:
							{
								if (!client->SupportsMultiOHCBs() || !client->SupportsWebCache())
									break;
								client->requestedFiles.RemoveAll();
								//CWebCacheMFRList*
								client->requestedFiles.AddFiles(&data_in);
								//client->requestedFiles = requestedFiles;

								Packet* tosend = WC_OHCBManager.GetWCBlocksForClient(client);
								if(tosend)
								{
									if (thePrefs.GetDebugClientTCPLevel() > 0)
										DebugSend("OP__multiOHCB", client, (char*)reqfile->GetFileHash());
									theStats.AddUpDataOverheadOther(tosend->size);
									SendPacket(tosend, true);
								}
								break;
							}*/
							// Superlexx - webcache - end
							// MORPH END - Added by Commander, WebCache 1.2e
							default:
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									Debug(_T("***NOTE: Invalid sub opcode 0x%02x with OP_MultiPacketAns received; %s"), opcode_in, client->DbgGetClientInfo());
						}
					}

					//MORPH START - Added by Commander, WebCache 1.9a
					if (client->SupportsWebCache()
						&& client->SupportsMultiOHCBs()
						&& client->IsTrustedOHCBSender())
					{
						Packet* tosend = client->CreateMFRPacket();
						if (tosend)
						{
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__multiFileRequest", client, (uchar*)reqfile->GetFileHash());
							theStats.AddUpDataOverheadOther(tosend->size);
							SendPacket(tosend, true);
						}
					}
					//MORPH END   - Added by Commander, WebCache 1.9a				
					break;
				}
				case OP_EMULEINFO:
				{
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessMuleInfoPacket(packet,size);
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_EmuleInfo", client);
						Debug(_T("  %s\n"), client->DbgGetMuleInfo());
					}

					// start secure identification, if
					//  - we have received eD2K and eMule info (old eMule)
					if (client->GetInfoPacketsReceived() == IP_BOTH)
						client->InfoPacketsReceived();

					client->SendMuleInfoPacket(true);
					break;
				}
				case OP_EMULEINFOANSWER:
				{
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessMuleInfoPacket(packet,size);
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_EmuleInfoAnswer", client);
						Debug(_T("  %s\n"), client->DbgGetMuleInfo());
					}

					// start secure identification, if
					//  - we have received eD2K and eMule info (old eMule)
					if (client->GetInfoPacketsReceived() == IP_BOTH)
						client->InfoPacketsReceived();
					break;
				}
				case OP_SECIDENTSTATE:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_SecIdentState", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					client->ProcessSecIdentStatePacket(packet, size);
					if (client->GetSecureIdentState() == IS_SIGNATURENEEDED)
						client->SendSignaturePacket();
					else if (client->GetSecureIdentState() == IS_KEYANDSIGNEEDED)
					{
						client->SendPublicKeyPacket();
						client->SendSignaturePacket();
					}
					break;
				}
				case OP_PUBLICKEY:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicKey", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					client->ProcessPublicKeyPacket(packet, size);
					break;
				}
  				case OP_SIGNATURE:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Signature", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					client->ProcessSignaturePacket(packet, size);
					break;
				}
				case OP_COMPRESSEDPART:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 1)
						DebugRecv("OP_CompressedPart", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(24);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);
					if (client->GetRequestFile() && !client->GetRequestFile()->IsStopped() && (client->GetRequestFile()->GetStatus()==PS_READY || client->GetRequestFile()->GetStatus()==PS_EMPTY))
					{
						client->ProcessBlockPacket(packet,size,true);
						if (client->GetRequestFile()->IsStopped() || client->GetRequestFile()->GetStatus()==PS_PAUSED || client->GetRequestFile()->GetStatus()==PS_ERROR)
						{
							client->SendCancelTransfer();
							client->SetDownloadState(client->GetRequestFile()->IsStopped() ? DS_NONE : DS_ONQUEUE);
						}
					}
					else
					{
						client->SendCancelTransfer();
						client->SetDownloadState((client->GetRequestFile()==NULL || client->GetRequestFile()->IsStopped()) ? DS_NONE : DS_ONQUEUE);
					}
					break;
				}
				case OP_QUEUERANKING:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_QueueRanking", client);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);
					client->ProcessEmuleQueueRank(packet, size);
					break;
				}
 				case OP_REQUESTSOURCES:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestSources", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadSourceExchange(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);
					if (client->GetSourceExchangeVersion() > 1)
					{
						if (size != 16)
							throw GetResString(IDS_ERR_BADSIZE);

						if (thePrefs.GetDebugSourceExchange())
							AddDebugLogLine(false, _T("SXRecv: Client source request; %s, %s"), client->DbgGetClientInfo(), DbgGetFileInfo(packet));

						//first check shared file list, then download list
						CKnownFile* reqfile;
						if ((reqfile = theApp.sharedfiles->GetFileByID(packet)) != NULL ||
							(reqfile = theApp.downloadqueue->GetFileByID(packet)) != NULL)
						{
							// There are some clients which do not follow the correct protocol procedure of sending
							// the sequence OP_REQUESTFILENAME, OP_SETREQFILEID, OP_REQUESTSOURCES. If those clients
							// are doing this, they will not get the optimal set of sources which we could offer if
							// the would follow the above noted protocol sequence. They better to it the right way
							// or they will get just a random set of sources because we do not know their download
							// part status which may get cleared with the call of 'SetUploadFileID'.
							client->SetUploadFileID(reqfile);

							DWORD dwTimePassed = ::GetTickCount() - client->GetLastSrcReqTime() + CONNECTION_LATENCY;
							bool bNeverAskedBefore = client->GetLastSrcReqTime() == 0;
							if (
								//if not complete and file is rare
								(    reqfile->IsPartFile()
								  && (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS)
								  && ((CPartFile*)reqfile)->GetSourceCount() <= RARE_FILE
								) ||
								//OR if file is not rare or if file is complete
								(bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS * MINCOMMONPENALTY)
							   )
							{
								client->SetLastSrcReqTime();
								Packet* tosend = reqfile->CreateSrcInfoPacket(client);
								if (tosend)
								{
									if (thePrefs.GetDebugClientTCPLevel() > 0)
										DebugSend("OP__AnswerSources", client, reqfile->GetFileHash());
									theStats.AddUpDataOverheadSourceExchange(tosend->size);
									SendPacket(tosend, true, true);
								}
							}
						}
						else
							client->CheckFailedFileIdReqs(packet);
					}
					break;
				}
 				case OP_ANSWERSOURCES:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AnswerSources", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadSourceExchange(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);

					CSafeMemFile data(packet, size);
					uchar hash[16];
					data.ReadHash16(hash);
					CKnownFile* file = theApp.downloadqueue->GetFileByID(hash);
					if(file)
					{
						if (file->IsPartFile())
						{
							//set the client's answer time
							client->SetLastSrcAnswerTime();
							//and set the file's last answer time
							((CPartFile*)file)->SetLastAnsweredTime();
							((CPartFile*)file)->AddClientSources(&data, client->GetSourceExchangeVersion(), client);
						}
					}
					else
						client->CheckFailedFileIdReqs(hash);
					break;
				}
				case OP_FILEDESC:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileDesc", client);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);
					client->ProcessMuleCommentPacket(packet,size);
					break;
				}
				case OP_REQUESTPREVIEW:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestPreView", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadOther(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);
	
					if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))	
					{
						client->ProcessPreviewReq(packet,size);	
						if (thePrefs.GetVerbose())
							AddDebugLogLine(true,_T("Client '%s' (%s) requested Preview - accepted"), client->GetUserName(), ipstr(client->GetConnectIP()));
					}
					else
					{
						// we don't send any answer here, because the client should know that he was not allowed to ask
						if (thePrefs.GetVerbose())
							AddDebugLogLine(true,_T("Client '%s' (%s) requested Preview - denied"), client->GetUserName(), ipstr(client->GetConnectIP()));
					}
					break;
				}
				case OP_PREVIEWANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PreviewAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadOther(uRawSize);
					client->CheckHandshakeFinished(OP_EMULEPROT, opcode);
					client->ProcessPreviewAnswer(packet, size);
					break;
				}
				case OP_PEERCACHE_QUERY:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					if (!client->ProcessPeerCacheQuery(packet, size))
					{
						CSafeMemFile dataSend(128);
						dataSend.WriteUInt8(PCPCK_VERSION);
						dataSend.WriteUInt8(PCOP_NONE);
						if (thePrefs.GetDebugClientTCPLevel() > 0){
							DebugSend("OP__PeerCacheAnswer", client);
							Debug(_T("  %s\n"), _T("Not supported"));
						}
						Packet* pEd2kPacket = new Packet(&dataSend, OP_EMULEPROT, OP_PEERCACHE_ANSWER);
						theStats.AddUpDataOverheadFileRequest(pEd2kPacket->size);
						SendPacket(pEd2kPacket);
					}
					break;
				}
				case OP_PEERCACHE_ANSWER:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					if ( (!client->ProcessPeerCacheAnswer(packet, size)) && client->GetDownloadState() != DS_NONEEDEDPARTS)
					{
						// We have sent a PeerCache Query to the remote client, for any reason the remote client
						// can not process it -> fall back to ed2k download.
						client->SetPeerCacheDownState(PCDS_NONE);
						ASSERT( client->m_pPCDownSocket == NULL );

						// PC-TODO: Check client state.
						ASSERT( client->GetDownloadState() == DS_DOWNLOADING );
						client->SetDownloadState(DS_ONQUEUE, _T("Peer cache query trouble")); // clear block requests
						if (client)
							client->StartDownload();
					}
					break;
				}
				case OP_PEERCACHE_ACK:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->ProcessPeerCacheAcknowledge(packet, size);
					break;
				}
				case OP_PUBLICIP_ANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicIPAns", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					client->ProcessPublicIPAnswer(packet, size);
					break;
				}
				case OP_PUBLICIP_REQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicIPReq", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__PublicIPAns", client);
					Packet* pPacket = new Packet(OP_PUBLICIP_ANSWER, 4, OP_EMULEPROT);
					PokeUInt32(pPacket->pBuffer, client->GetIP());
					theStats.AddUpDataOverheadOther(pPacket->size);
					SendPacket(pPacket);
					break;
				}
				case OP_PORTTEST:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PortTest", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					m_bPortTestCon=true;
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__PortTest", client);
					Packet* replypacket = new Packet(OP_PORTTEST, 1);
					replypacket->pBuffer[0]=0x12;
					theStats.AddUpDataOverheadOther(replypacket->size);
					SendPacket(replypacket);
					break;
				}
				case OP_CALLBACK:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Callback", client);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					if(!Kademlia::CKademlia::isRunning())
						break;
					CSafeMemFile data(packet, size);
					Kademlia::CUInt128 check;
					data.ReadUInt128(&check);
					check.xor(Kademlia::CUInt128(true));
					if( check.compareTo(Kademlia::CKademlia::getPrefs()->getKadID()))
						break;
					Kademlia::CUInt128 fileid;
					data.ReadUInt128(&fileid);
					uchar fileid2[16];
					fileid.toByteArray(fileid2);
					CKnownFile* reqfile;
					if ( (reqfile = theApp.sharedfiles->GetFileByID(fileid2)) == NULL )
					{
						if ( (reqfile = theApp.downloadqueue->GetFileByID(fileid2)) == NULL)
						{
							client->CheckFailedFileIdReqs(fileid2);
							break;
						}
					}

					uint32 ip = data.ReadUInt32();
					uint16 tcp = data.ReadUInt16();
					CUpDownClient* callback;
					callback = theApp.clientlist->FindClientByIP(ntohl(ip), tcp);
					if( callback == NULL )
					{
						callback = new CUpDownClient(NULL,tcp,ip,0,0);
						theApp.clientlist->AddClient(callback);
					}
					callback->TryToConnect(true);
					break;
				}
				case OP_BUDDYPING:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_BuddyPing", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					CUpDownClient* buddy = theApp.clientlist->GetBuddy();
					if( buddy != client || client->GetKadVersion() == 0 || !client->AllowIncomeingBuddyPingPong() )
						//This ping was not from our buddy or wrong version or packet sent to fast. Ignore
						break;
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__BuddyPong", client);
					Packet* replypacket = new Packet(OP_BUDDYPONG, 0, OP_EMULEPROT);
					theStats.AddDownDataOverheadOther(replypacket->size);
					SendPacket(replypacket);
					client->SetLastBuddyPingPongTime();
					break;
				}
				case OP_BUDDYPONG:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_BuddyPong", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					CUpDownClient* buddy = theApp.clientlist->GetBuddy();
					if( buddy != client || client->GetKadVersion() == 0 )
						//This pong was not from our buddy or wrong version. Ignore
						break;
					client->SetLastBuddyPingPongTime();
					//All this is for is to reset our socket timeout.
					break;
				}
				case OP_REASKCALLBACKTCP:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					CUpDownClient* buddy = theApp.clientlist->GetBuddy();
					if (buddy != client) {
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_ReaskCallbackTCP", client, NULL);
						//This callback was not from our buddy.. Ignore.
						break;
					}
					CSafeMemFile data_in(packet, size);
					uint32 destip = data_in.ReadUInt32();
					uint16 destport = data_in.ReadUInt16();
					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ReaskCallbackTCP", client, reqfilehash);
					CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(reqfilehash);
					if (!reqfile)
					{
						if (thePrefs.GetDebugClientUDPLevel() > 0)
							DebugSend("OP__FileNotFound", NULL);
						Packet* response = new Packet(OP_FILENOTFOUND,0,OP_EMULEPROT);
						theStats.AddUpDataOverheadFileRequest(response->size);
						theApp.clientudp->SendPacket(response, destip, destport);
						break;
					}
					CUpDownClient* sender = theApp.uploadqueue->GetWaitingClientByIP_UDP(destip, destport);
					if (sender)
					{
						//Make sure we are still thinking about the same file
						if (md4cmp(reqfilehash, sender->GetUploadFileID()) == 0)
						{
							sender->AddAskedCount();
							sender->SetLastUpRequest();
							//I messed up when I first added extended info to UDP
							//I should have originally used the entire ProcessExtenedInfo the first time.
							//So now I am forced to check UDPVersion to see if we are sending all the extended info.
							//For now on, we should not have to change anything here if we change
							//anything to the extended info data as this will be taken care of in ProcessExtendedInfo()
							//Update extended info. 
							if (sender->GetUDPVersion() > 3)
							{
								sender->ProcessExtendedInfo(&data_in, reqfile);
							}
							//Update our complete source counts.
							else if (sender->GetUDPVersion() > 2)
							{
								uint16 nCompleteCountLast= sender->GetUpCompleteSourcesCount();
								uint16 nCompleteCountNew = data_in.ReadUInt16();
								sender->SetUpCompleteSourcesCount(nCompleteCountNew);
								if (nCompleteCountLast != nCompleteCountNew)
								{
									reqfile->UpdatePartsInfo();
								}
							}
							CSafeMemFile data_out(128);
							if(sender->GetUDPVersion() > 3)
							{
								if (reqfile->IsPartFile())
									((CPartFile*)reqfile)->WritePartStatus(&data_out);
								else
									data_out.WriteUInt16(0);
							}
							data_out.WriteUInt16(theApp.uploadqueue->GetWaitingPosition(sender));
							if (thePrefs.GetDebugClientUDPLevel() > 0)
								DebugSend("OP__ReaskAck", sender);
							Packet* response = new Packet(&data_out, OP_EMULEPROT);
							response->opcode = OP_REASKACK;
							theStats.AddUpDataOverheadFileRequest(response->size);
							theApp.clientudp->SendPacket(response, destip, destport);
						}
						else
						{
							DebugLogWarning(_T("Client UDP socket; OP_REASKCALLBACKTCP; reqfile does not match"));
							TRACE(_T("reqfile:         %s\n"), DbgGetFileInfo(reqfile->GetFileHash()));
							TRACE(_T("sender->GetRequestFile(): %s\n"), sender->GetRequestFile() ? DbgGetFileInfo(sender->GetRequestFile()->GetFileHash()) : _T("(null)"));
						}
					}
					else
					{
						if (((uint32)theApp.uploadqueue->GetWaitingUserCount() + 50) > thePrefs.GetQueueSize())
						{
							if (thePrefs.GetDebugClientUDPLevel() > 0)
								DebugSend("OP__QueueFull", NULL);
							Packet* response = new Packet(OP_QUEUEFULL,0,OP_EMULEPROT);
							theStats.AddUpDataOverheadFileRequest(response->size);
							theApp.clientudp->SendPacket(response, destip, destport);
						}
					}
					break;
				}
				case OP_AICHANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->ProcessAICHAnswer(packet,size);
					break;
				}
				case OP_AICHREQUEST:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichRequest", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->ProcessAICHRequest(packet,size);
					break;
				}
				case OP_AICHFILEHASHANS:
				{
					// those should not be received normally, since we should only get those in MULTIPACKET
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichFileHashAns", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					CSafeMemFile data(packet, size);
					client->ProcessAICHFileHash(&data, NULL);
					break;
				}
				case OP_AICHFILEHASHREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichFileHashReq", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					// those should not be received normally, since we should only get those in MULTIPACKET
					CSafeMemFile data(packet, size);
					uchar abyHash[16];
					data.ReadHash16(abyHash);
					CKnownFile* pPartFile = theApp.sharedfiles->GetFileByID(abyHash);
					if (pPartFile == NULL){
						client->CheckFailedFileIdReqs(abyHash);
						break;
					}
					if (client->IsSupportingAICH() && pPartFile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE
						&& pPartFile->GetAICHHashset()->HasValidMasterHash())
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AichFileHashAns", client, abyHash);
						CSafeMemFile data_out;
						data_out.WriteHash16(abyHash);
						pPartFile->GetAICHHashset()->GetMasterHash().Write(&data_out);
						Packet* response = new Packet(&data_out, OP_EMULEPROT, OP_AICHFILEHASHANS);
						theStats.AddUpDataOverheadFileRequest(response->size);
						SendPacket(response);
					}
					break;
				}
				default:
					theStats.AddDownDataOverheadOther(uRawSize);
					PacketToDebugLogLine(_T("eMule"), packet, size, opcode, DLP_DEFAULT);
					break;
			}
		}
		catch(CFileException* error)
		{
			error->Delete();
			throw GetResString(IDS_ERR_INVALIDPACKAGE);
		}
		catch(CMemoryException* error)
		{
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch(CClientException* ex) // nearly same as the 'CString' exception but with optional deleting of the client
	{
		if (thePrefs.GetVerbose() && !ex->m_strMsg.IsEmpty())
			DebugLogWarning(_T("%s - while processing eMule packet: opcode=%s  size=%u; %s"), ex->m_strMsg, DbgGetMuleClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client && ex->m_bDelete)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eMule packet: ") + ex->m_strMsg);
		Disconnect(ex->m_strMsg);
		ex->Delete();
		return false;
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose() && !error.IsEmpty())
			DebugLogWarning(_T("%s - while processing eMule packet: opcode=%s  size=%u; %s"), error, DbgGetMuleClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client)
			client->SetDownloadState(DS_ERROR, _T("ProcessExtPacket error. ") + error);
		Disconnect(_T("ProcessExtPacket error. ") + error);
		return false;
	}
	return true;
}

void CClientReqSocket::PacketToDebugLogLine(LPCTSTR protocol, const uchar* packet, uint32 size, UINT opcode, EDebugLogPriority dlpPriority)
{
	if (thePrefs.GetVerbose())
	{
		CString buffer; 
	    buffer.Format(_T("Unknown %s Protocol Opcode: 0x%02x, Size=%u, Data=["), protocol, opcode, size);
		for (uint32 i = 0; i < size && i < 50; i++){
			if (i > 0)
				buffer += _T(' ');
			TCHAR temp[3];
		    _stprintf(temp, _T("%02x"), packet[i]);
			buffer += temp;
		}
		buffer += (i == size) ? _T("]") : _T("..]");
		DbgAppendClientInfo(buffer);
		DebugLogWarning(_T("%s"), buffer);
	}
}

CString CClientReqSocket::DbgGetClientInfo()
{
	CString str;
	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	if (sockAddr.sin_addr.S_un.S_addr != 0 && (client == NULL || sockAddr.sin_addr.S_un.S_addr != client->GetIP()))
		str.AppendFormat(_T("IP=%s"), ipstr(sockAddr.sin_addr));
	if (client){
		if (!str.IsEmpty())
			str += _T("; ");
		str += _T("Client=") + client->DbgGetClientInfo();
	}
	return str;
}

void CClientReqSocket::DbgAppendClientInfo(CString& str)
{
	CString strClientInfo(DbgGetClientInfo());
	if (!strClientInfo.IsEmpty()){
		if (!str.IsEmpty())
			str += _T("; ");
		str += strClientInfo;
	}
}

void CClientReqSocket::OnConnect(int nErrorCode)
{
	SetConState(SS_Complete);
	CEMSocket::OnConnect(nErrorCode);
	if (nErrorCode)
	{
	    CString strTCPError;
		if (thePrefs.GetVerbose())
		{
		    strTCPError = GetErrorMessage(nErrorCode, 1);
		    if (nErrorCode != WSAECONNREFUSED && nErrorCode != WSAETIMEDOUT)
			    DebugLogError(_T("Client TCP socket (OnConnect): %s; %s"), strTCPError, DbgGetClientInfo());
		}
		Disconnect(strTCPError);

        if(client && (client->GetUploadState() == US_CONNECTING || client->GetUploadState() == US_UPLOADING)) {
            theApp.uploadqueue->RemoveFromUploadQueue(client, _T("Failed to connect: ") + strTCPError);
        }
	}
	else
	{
		//This socket may have been delayed by SP2 protection, lets make sure it doesn't time out instantly.
		ResetTimeOutTimer();
	}
}

void CClientReqSocket::OnSend(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnSend(nErrorCode);
}

void CClientReqSocket::OnError(int nErrorCode)
{
	CString strTCPError;
	if (thePrefs.GetVerbose())
	{
		if (nErrorCode == ERR_WRONGHEADER)
			strTCPError = _T("Error: Wrong header");
		else if (nErrorCode == ERR_TOOBIG)
			strTCPError = _T("Error: Too much data sent");
		else
			strTCPError = GetErrorMessage(nErrorCode);
		DebugLogWarning(_T("Client TCP socket: %s; %s"), strTCPError, DbgGetClientInfo());
	}

	Disconnect(strTCPError);
}

bool CClientReqSocket::PacketReceivedCppEH(Packet* packet)
{
	bool bResult;
	UINT uRawSize = packet->size;
	switch (packet->prot){
		case OP_EDONKEYPROT:
			bResult = ProcessPacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode);
			break;
		case OP_PACKEDPROT:
			if (!packet->UnPackPacket()){
				if (thePrefs.GetVerbose())
					DebugLogError(_T("Failed to decompress client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());
				bResult = false;
				break;
			}
		case OP_EMULEPROT:
			bResult = ProcessExtPacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode, uRawSize);
			break;
// MORPH START - Added by Commander, WebCache 1.2f
// WebCache ////////////////////////////////////////////////////////////////////////////////////
		// yonatan - webcache protocol packets
		case OP_WEBCACHEPACKEDPROT:	// Superlexx - packed WC protocol
			if (!packet->UnPackPacket())
			{
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Failed to decompress client WC TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());
				bResult = false;
				break;
			}
		case OP_WEBCACHEPROT:
			bResult = ProcessWebCachePacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode, uRawSize);
			break;
		// yonatan - webcache protocol packets end
// MORPH END - Added by SiRoB, WebCache 1.2f
		default:{
			theStats.AddDownDataOverheadOther(uRawSize);
			if (thePrefs.GetVerbose())
				DebugLogWarning(_T("Received unknown client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());

			if (client)
				client->SetDownloadState(DS_ERROR, _T("Unknown protocol"));
			Disconnect(_T("Unknown protocol"));
			bResult = false;
		}
	}
	return bResult;
}

#ifdef USE_CLIENT_TCP_CATCH_ALL_HANDLER
int FilterSE(DWORD dwExCode, LPEXCEPTION_POINTERS pExPtrs, CClientReqSocket* reqsock, Packet* packet)
{
	if (thePrefs.GetVerbose())
	{
		CString strExError;
		if (pExPtrs){
			const EXCEPTION_RECORD* er = pExPtrs->ExceptionRecord;
			strExError.Format(_T("Error: Unknown exception %08x in CClientReqSocket::PacketReceived at 0x%08x"), er->ExceptionCode, er->ExceptionAddress);
		}
		else
			strExError.Format(_T("Error: Unknown exception %08x in CClientReqSocket::PacketReceived"), dwExCode);

		// we already had an unknown exception, better be prepared for dealing with invalid data -> use another exception handler
		try{
			CString strError = strExError;
			strError.AppendFormat(_T("; %s"), DbgGetClientTCPPacket(packet?packet->prot:0, packet?packet->opcode:0, packet?packet->size:0));
			reqsock->DbgAppendClientInfo(strError);
			DebugLogError(_T("%s"), strError);
		}
		catch(...){
			ASSERT(0);
			DebugLogError(_T("%s"), strExError);
		}
	}
	
	// this searches the next exception handler -> catch(...) in 'CAsyncSocketExHelperWindow::WindowProc'
	// as long as I do not know where and why we are crashing, I prefere to have it handled that way which
	// worked fine in 28a/b.
	//
	// 03-J�n-2004 [bc]: Returning the execution to the catch-all handler in 'CAsyncSocketExHelperWindow::WindowProc'
	// can make things even worse, because in some situations, the socket will continue fireing received events. And
	// because the processed packet (which has thrown the exception) was not removed from the EMSocket buffers, it would
	// be processed again and again.
	//return EXCEPTION_CONTINUE_SEARCH;

	// this would continue the program "as usual" -> return execution to the '__except' handler
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef USE_CLIENT_TCP_CATCH_ALL_HANDLER
int CClientReqSocket::PacketReceivedSEH(Packet* packet)
{
	int iResult;
	// this function is only here to get a chance of determining the crash address via SEH
	__try{
		iResult = PacketReceivedCppEH(packet);
	}
	__except(FilterSE(GetExceptionCode(), GetExceptionInformation(), this, packet)){
		iResult = -1;
	}
	return iResult;
}
#endif

bool CClientReqSocket::PacketReceived(Packet* packet)
{
	bool bResult;
#ifdef USE_CLIENT_TCP_CATCH_ALL_HANDLER
	int iResult = PacketReceivedSEH(packet);
	if (iResult < 0)
	{
		if (client)
			client->SetDownloadState(DS_ERROR, _T("Unknown Exception"));
		Disconnect(_T("Unknown Exception"));
		bResult = false;
	}
	else
		bResult = iResult!=0;
#else
	bResult = PacketReceivedCppEH(packet);
#endif
	return bResult;
}

void CClientReqSocket::OnReceive(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnReceive(nErrorCode);
}

bool CClientReqSocket::Create()
{
	theApp.listensocket->AddConnection();
	return (CAsyncSocketEx::Create(0, SOCK_STREAM, FD_WRITE|FD_READ|FD_CLOSE|FD_CONNECT) != FALSE);
}

SocketSentBytes CClientReqSocket::SendControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend)
{
    SocketSentBytes returnStatus = CEMSocket::SendControlData(maxNumberOfBytesToSend, overchargeMaxBytesToSend);
    if (returnStatus.success && (returnStatus.sentBytesControlPackets > 0 || returnStatus.sentBytesStandardPackets > 0))
        ResetTimeOutTimer();
    return returnStatus;
}

SocketSentBytes CClientReqSocket::SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend)
{
    SocketSentBytes returnStatus = CEMSocket::SendFileAndControlData(maxNumberOfBytesToSend, overchargeMaxBytesToSend);
    if (returnStatus.success && (returnStatus.sentBytesControlPackets > 0 || returnStatus.sentBytesStandardPackets > 0))
        ResetTimeOutTimer();
    return returnStatus;
}

void CClientReqSocket::SendPacket(Packet* packet, bool delpacket, bool controlpacket, uint32 actualPayloadSize)
{
	ResetTimeOutTimer();
	CEMSocket::SendPacket(packet, delpacket, controlpacket, actualPayloadSize);
}

bool CListenSocket::SendPortTestReply(char result, bool disconnect)
{
	POSITION pos2;
	for(POSITION pos1 = socket_list.GetHeadPosition(); ( pos2 = pos1 ) != NULL; )
	{
		socket_list.GetNext(pos1);
		CClientReqSocket* cur_sock = socket_list.GetAt(pos2);
		if (cur_sock->m_bPortTestCon)
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__PortTest", cur_sock->client);
			Packet* replypacket = new Packet(OP_PORTTEST, 1);
			replypacket->pBuffer[0]=result;
			theStats.AddUpDataOverheadOther(replypacket->size);
			cur_sock->SendPacket(replypacket);
			if (disconnect)
				cur_sock->m_bPortTestCon = false;
			return true;
		}
	}
	return false;
}

CListenSocket::CListenSocket()
{
	bListening = false;
	maxconnectionreached = 0;
	m_OpenSocketsInterval = 0;
	m_nPendingConnections = 0;
	MEMSET(m_ConnectionStates, 0, sizeof m_ConnectionStates);
	peakconnections = 0;
	totalconnectionchecks = 0;
	averageconnections = 0.0;
	activeconnections = 0;
	m_port=0;
	m_nHalfOpen = 0;
	m_nComp = 0;
}

CListenSocket::~CListenSocket()
{
	Close();
	KillAllSockets();
}

bool CListenSocket::Rebind()
{
	if (thePrefs.GetPort() == m_port)
		return false;

	Close();
	KillAllSockets();

	return StartListening();
}

bool CListenSocket::StartListening()
{
	bListening = true;

	// Creating the socket with SO_REUSEADDR may solve LowID issues if emule was restarted
	// quickly or started after a crash, but(!) it will also create another problem. If the
	// socket is already used by some other application (e.g. a 2nd emule), we though bind
	// to that socket leading to the situation that 2 applications are listening at the same
	// port!
	if (!Create(thePrefs.GetPort(), SOCK_STREAM, FD_ACCEPT, NULL, FALSE/*bReuseAddr*/))
		return false;

	// Rejecting a connection with conditional WSAAccept and not using SO_CONDITIONAL_ACCEPT
	// -------------------------------------------------------------------------------------
	// recv: SYN
	// send: SYN ACK (!)
	// recv: ACK
	// send: ACK RST
	// recv: PSH ACK + OP_HELLO packet
	// send: RST
	// --- 455 total bytes (depending on OP_HELLO packet)
	// In case SO_CONDITIONAL_ACCEPT is not used, the TCP/IP stack establishes the connection
	// before WSAAccept has a chance to reject it. That's why the remote peer starts to send
	// it's first data packet.
	// ---
	// Not using SO_CONDITIONAL_ACCEPT gives us 6 TCP packets and the OP_HELLO data. We
	// have to lookup the IP only 1 time. This is still way less traffic than rejecting the
	// connection by closing it after the 'Accept'.

	// Rejecting a connection with conditional WSAAccept and using SO_CONDITIONAL_ACCEPT
	// ---------------------------------------------------------------------------------
	// recv: SYN
	// send: ACK RST
	// recv: SYN
	// send: ACK RST
	// recv: SYN
	// send: ACK RST
	// --- 348 total bytes
	// The TCP/IP stack tries to establish the connection 3 times until it gives up. 
	// Furthermore the remote peer experiences a total timeout of ~ 1 minute which is
	// supposed to be the default TCP/IP connection timeout (as noted in MSDN).
	// ---
	// Although we get a total of 6 TCP packets in case of using SO_CONDITIONAL_ACCEPT,
	// it's still less than not using SO_CONDITIONAL_ACCEPT. But, we have to lookup
	// the IP 3 times instead of 1 time.

	//if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxy().UseProxy) {
	//	int iOptVal = 1;
	//	VERIFY( SetSockOpt(SO_CONDITIONAL_ACCEPT, &iOptVal, sizeof iOptVal) );
	//}

	if (!Listen())
		return false;

	m_port = thePrefs.GetPort();
	return true;
}

void CListenSocket::ReStartListening()
{
	bListening = true;

	ASSERT( m_nPendingConnections >= 0 );
	if (m_nPendingConnections > 0)
	{
		m_nPendingConnections--;
		OnAccept(0);
	}
}

void CListenSocket::StopListening()
{
	bListening = false;
	maxconnectionreached++;
}

static bool _bAcceptConnectionCondRejected;

int CALLBACK AcceptConnectionCond(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS lpSQOS, LPQOS lpGQOS,
								  LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP FAR *g, DWORD dwCallbackData)
{
	if (lpCallerId && lpCallerId->buf && lpCallerId->len >= sizeof SOCKADDR_IN)
	{
		LPSOCKADDR_IN pSockAddr = (LPSOCKADDR_IN)lpCallerId->buf;
		ASSERT( pSockAddr->sin_addr.S_un.S_addr != 0 && pSockAddr->sin_addr.S_un.S_addr != INADDR_NONE );

		if (theApp.ipfilter->IsFiltered(pSockAddr->sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter (%s)"), ipstr(pSockAddr->sin_addr.S_un.S_addr), theApp.ipfilter->GetLastHit());
			theStats.filteredclients++; //Xtreme Mod Bugfix
			_bAcceptConnectionCondRejected = true;
			return CF_REJECT;
		}

		if (theApp.clientlist->IsBannedClient(pSockAddr->sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogBannedClients()){
				CUpDownClient* pClient = theApp.clientlist->FindClientByIP(pSockAddr->sin_addr.S_un.S_addr);
				AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(pSockAddr->sin_addr.S_un.S_addr), pClient->DbgGetClientInfo());
			}
			_bAcceptConnectionCondRejected = true;
			return CF_REJECT;
		}
	}
	else {
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Client TCP socket: AcceptConnectionCond unexpected lpCallerId"));
	}

	return CF_ACCEPT;
}

void CListenSocket::OnAccept(int nErrorCode)
{
	if (!nErrorCode)
	{
		m_nPendingConnections++;
		if (m_nPendingConnections < 1){
			ASSERT(0);
			m_nPendingConnections = 1;
		}

		if (TooManySockets(true) && !theApp.serverconnect->IsConnecting()){
			StopListening();
			return;
		}
		else if (!bListening)
			ReStartListening(); //If the client is still at maxconnections, this will allow it to go above it.. But if you don't, you will get a lowID on all servers.
	
		uint32 nFataErrors = 0;
		while (m_nPendingConnections > 0)
		{
// WebCache ////////////////////////////////////////////////////////////////////////////////////
			// JP detect fake HighID
			// MOD BEGIN netfinity: Fake HighID
			thePrefs.m_bHighIdPossible = true;
			// MOD END netfinity
			m_nPendingConnections--;

			CClientReqSocket* newclient;
			SOCKADDR_IN SockAddr = {0};
			int iSockAddrLen = sizeof SockAddr;
			if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxy().UseProxy)
			{
				_bAcceptConnectionCondRejected = false;
				SOCKET sNew = WSAAccept(m_SocketData.hSocket, (SOCKADDR*)&SockAddr, &iSockAddrLen, AcceptConnectionCond, 0);
				if (sNew == INVALID_SOCKET){
				    DWORD nError = GetLastError();
				    if (nError == WSAEWOULDBLOCK){
					    DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
					    m_nPendingConnections = 0;
					    break;
				    }
				    else{
						if (nError != WSAECONNREFUSED || !_bAcceptConnectionCondRejected){
						    DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
							nFataErrors++;
						}
				    }
				    if (nFataErrors > 10){
					    // the question is what todo on a error. We cant just ignore it because then the backlog will fill up
					    // and lock everything. We can also just endlos try to repeat it because this will lock up eMule
					    // this should basically never happen anyway
					    // however if we are in such a position, try to reinitalize the socket.
					    DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
					    Close();
					    StartListening();
					    m_nPendingConnections = 0;
					    break;
				    }
					continue;
				}
				newclient = new CClientReqSocket;
				VERIFY( newclient->InitAsyncSocketExInstance() );
				newclient->m_SocketData.hSocket = sNew;
				newclient->AttachHandle(sNew);

				AddConnection();
			}
			else
			{
				newclient = new CClientReqSocket;
			    if (!Accept(*newclient, (SOCKADDR*)&SockAddr, &iSockAddrLen)){
				    newclient->Safe_Delete();
				    DWORD nError = GetLastError();
				    if (nError == WSAEWOULDBLOCK){
					    DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
					    m_nPendingConnections = 0;
					    break;
				    }
				    else{
					    DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
					    nFataErrors++;
				    }
				    if (nFataErrors > 10){
					    // the question is what todo on a error. We cant just ignore it because then the backlog will fill up
					    // and lock everything. We can also just endlos try to repeat it because this will lock up eMule
					    // this should basically never happen anyway
					    // however if we are in such a position, try to reinitalize the socket.
					    DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
					    Close();
					    StartListening();
					    m_nPendingConnections = 0;
					    break;
				    }
				    continue;
			    }
	    
			    AddConnection();
    
			    if (SockAddr.sin_addr.S_un.S_addr == 0) // for safety..
			    {
				    iSockAddrLen = sizeof SockAddr;
				    newclient->GetPeerName((SOCKADDR*)&SockAddr, &iSockAddrLen);
				    DebugLogWarning(_T("SockAddr.sin_addr.S_un.S_addr == 0;  GetPeerName returned %s"), ipstr(SockAddr.sin_addr.S_un.S_addr));
			    }
    
			    ASSERT( SockAddr.sin_addr.S_un.S_addr != 0 && SockAddr.sin_addr.S_un.S_addr != INADDR_NONE );
    
			    if (theApp.ipfilter->IsFiltered(SockAddr.sin_addr.S_un.S_addr)){
				    if (thePrefs.GetLogFilteredIPs())
					    AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter (%s)"), ipstr(SockAddr.sin_addr.S_un.S_addr), theApp.ipfilter->GetLastHit());
				    newclient->Safe_Delete();
				    theStats.filteredclients++;
				    continue;
			    }
    
			    if (theApp.clientlist->IsBannedClient(SockAddr.sin_addr.S_un.S_addr)){
				    if (thePrefs.GetLogBannedClients()){
					    CUpDownClient* pClient = theApp.clientlist->FindClientByIP(SockAddr.sin_addr.S_un.S_addr);
					    AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(SockAddr.sin_addr.S_un.S_addr), pClient->DbgGetClientInfo());
				    }
				    newclient->Safe_Delete();
				    continue;
			    }
			}
			newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);
		}

		ASSERT( m_nPendingConnections >= 0 );
	}
}

// Maella -Code Improvement-
void CListenSocket::Process()
{
	// Update counter
	m_OpenSocketsInterval = 0;
	if( !SINGLEProxyClient )
		WebCachedBlockList.TryToDL();
	else
		SINGLEProxyClient->CheckDownloadTimeout();
        // Check state	
	for(POSITION pos = socket_list.GetHeadPosition(); pos != NULL; ){
		CClientReqSocket* cur_sock = socket_list.GetNext(pos);
		if(cur_sock->deletethis == true){
			if (cur_sock->m_SocketData.hSocket != INVALID_SOCKET){
				cur_sock->Close();			// calls 'closesocket'
			}
			else{
				// Remove instance from the list (Recursive)
				cur_sock->Delete_Timed();;	// may delete 'cur_sock'
			}
		}
		else{
			cur_sock->CheckTimeOut();		// may call 'shutdown'
		}
	}

	if((bListening == false) && 
		(GetOpenSockets()+5 < thePrefs.GetMaxConnections() || theApp.serverconnect->IsConnecting() == true)){
		ReStartListening();
}
}
// Maella end

void CListenSocket::RecalculateStats()
{
	MEMSET(m_ConnectionStates, 0, sizeof m_ConnectionStates);
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL; )
	{
		switch (socket_list.GetNext(pos)->GetConState())
		{
			case ES_DISCONNECTED:
				m_ConnectionStates[0]++;
				break;
			case ES_NOTCONNECTED:
				m_ConnectionStates[1]++;
				break;
			case ES_CONNECTED:
				m_ConnectionStates[2]++;
				break;
		}
   }
}

void CListenSocket::AddSocket(CClientReqSocket* toadd)
{
	socket_list.AddTail(toadd);
}

// - Maella -Code Improvement-
void CListenSocket::RemoveSocket(CClientReqSocket* todel)
{
	POSITION pos = socket_list.Find(todel);
	if (pos)
		socket_list.RemoveAt(pos);
}

void CListenSocket::KillAllSockets()
{
	while(socket_list.GetHeadPosition() != NULL)
	{
		CClientReqSocket* cur_socket = socket_list.GetHead();
		if (cur_socket->client)
			delete cur_socket->client;
		else
			delete cur_socket;
	}
}
//Xman end

void CListenSocket::AddConnection()
{
	m_OpenSocketsInterval++;
}

bool CListenSocket::TooManySockets(bool bIgnoreInterval)
{
	if (   GetOpenSockets() > thePrefs.GetMaxConnections()
		|| (m_OpenSocketsInterval > (thePrefs.GetMaxConperFive() * GetMaxConperFiveModifier()) && !bIgnoreInterval)
		|| (m_nHalfOpen >= thePrefs.GetMaxHalfConnections() && !bIgnoreInterval))
		return true;
	return false;
}

bool CListenSocket::IsValidSocket(CClientReqSocket* totest)
{
	return socket_list.Find(totest) != NULL;
}

#ifdef _DEBUG
void CListenSocket::Debug_ClientDeleted(CUpDownClient* deleted)
{
	// Spanish; dazzle: cannot do check if socket_list is empty
    return;
	//Spanish;  end dazzle
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL;)
	{
		CClientReqSocket* cur_sock = socket_list.GetNext(pos);
		if (!AfxIsValidAddress(cur_sock, sizeof(CClientReqSocket)))
			AfxDebugBreak();
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_sock);
		if (cur_sock->client == deleted)
			AfxDebugBreak();
	}
}
#endif

void CListenSocket::UpdateConnectionsStatus()
{
	activeconnections = GetOpenSockets();
	if (peakconnections < activeconnections)
		peakconnections = activeconnections;
	if (peakconnections > thePrefs.GetConnPeakConnections())
		thePrefs.SetConnPeakConnections(peakconnections);
	if (theApp.IsConnected())
	{
		totalconnectionchecks++;
		float percent = (float)(totalconnectionchecks - 1) / (float)totalconnectionchecks;
		if (percent > 0.99F)
			percent = 0.99F;
		averageconnections = (averageconnections * percent) + (float)activeconnections * (1.0F - percent);
	}
}

float CListenSocket::GetMaxConperFiveModifier()
{
	float SpikeSize = GetOpenSockets() - averageconnections;
	if (SpikeSize < 1.0F)
		return 1.0F;

	float SpikeTolerance = 25.0F * (float)thePrefs.GetMaxConperFive() / 10.0F;
	if (SpikeSize > SpikeTolerance)
		return 0;

	float Modifier = 1.0F - SpikeSize / SpikeTolerance;
	return Modifier;
}
// MORPH START - Added by SiRoB, WebCache 1.2f
// WebCache ////////////////////////////////////////////////////////////////////////////////////
// yonatan - webcache protocol packets
bool CClientReqSocket::ProcessWebCachePacket(const BYTE* packet, uint32 size, UINT opcode, UINT uRawSize)
{
	if (thePrefs.m_iDbgHeap >= 2)
		ASSERT_VALID(client);
	switch(opcode)
	{
		case OP_DONT_SEND_OHCBS:
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugRecv("OP__Dont_Send_Ohcbs recv", client);
			if (thePrefs.GetLogWebCacheEvents())
			AddDebugLogLine( false, _T("OP__Dont_Send_Ohcbs RECEIVED")); // yonatan tmp
			theStats.AddDownDataOverheadOther(size);
			if( client->SupportsWebCache() && client->SupportsOhcbSuppression() )
				client->m_bIsAcceptingOurOhcbs = false;
			return true;
		//JP TEST THIS!!! (WE ARE NOT USING IT YET)
		case OP_RESUME_SEND_OHCBS: //we are not using it yet, but might in the future
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugRecv("OP_RESUME_SEND_OHCBS received from", client);
			if (thePrefs.GetLogWebCacheEvents())
				AddDebugLogLine( false, _T("OP_RESUME_SEND_OHCBS RECEIVED")); // yonatan tmp
			theStats.AddDownDataOverheadOther(size);
			if( client->SupportsWebCache() && client->SupportsOhcbSuppression() )
				client->m_bIsAcceptingOurOhcbs = true;
			return true;
		case OP_HTTP_CACHED_BLOCK:
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugRecv("OP__Http_Cached_Block", client);
			theStats.AddDownDataOverheadOther(size);
			if( thePrefs.IsWebCacheDownloadEnabled() && client->SupportsWebCache() ) 
			{
				// CHECK HANDSHAKE?
				if (thePrefs.GetLogWebCacheEvents())
					AddDebugLogLine( false, _T("Received WCBlock - TCP") );
				CWebCachedBlock* newblock = new CWebCachedBlock( (const BYTE*)packet, size, client ); // Starts DL or places block on queue
			}
			return true;
		case OP_XPRESS_MULTI_HTTP_CACHED_BLOCKS:
		case OP_MULTI_HTTP_CACHED_BLOCKS:
            if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugRecv("OP__Multi_Http_Cached_Blocks", client);
			theStats.AddDownDataOverheadOther(size);
			if( thePrefs.IsWebCacheDownloadEnabled() && client->SupportsWebCache() ) 
			{
				// CHECK HANDSHAKE?
				if (thePrefs.GetLogWebCacheEvents())
					AddDebugLogLine( false, _T("Received MultiWCBlocks - TCP") );
				//CWebCachedBlock* newblock = new CWebCachedBlock( (char*)packet, size, client ); // Starts DL or places block on queue
				CSafeMemFile data((BYTE*)packet,size);
				uint32 uploadID;
				uploadID = data.ReadUInt32();
				if (client->m_uWebCacheUploadId != uploadID)
					return false;

				return WebCachedBlockList.ProcessWCBlocks((const BYTE*)packet, size, opcode, client);
			}
			return false;
		case OP_MULTI_FILE_REQ:
			{
				if (!client->SupportsMultiOHCBs() || !client->SupportsWebCache())
					break;
//				client->requestedFiles.RemoveAll();
//				client->requestedFiles.AddFiles(&CSafeMemFile(BYTE(packet)));
				CSafeMemFile data_in((BYTE*)packet,size);
				client->requestedFiles.AddFiles(&data_in, client);

				Packet* toSend = WC_OHCBManager.GetWCBlocksForClient(client);
				if(toSend)
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__multiOHCB", client);
					theStats.AddUpDataOverheadOther(toSend->size);
					SendPacket(toSend, true);
				}
				return true;
			}
			return false;
		default:
			theStats.AddDownDataOverheadOther(uRawSize);
			PacketToDebugLogLine(_T("WebCache"), packet, size, opcode, DLP_DEFAULT);
	}
			return false;
}
// yonatan - webcache protocol packets end
// MORPH END   - Added by SiRoB, WebCache 1.2f
