#define MS_CLASS "RTC::Room"

#include "RTC/Room.h"
#include "MediaSoupError.h"
#include "Logger.h"

// TMP
#define NUM_PEERS  4
#define PAYLOAD_TYPE_AUDIO  111  // OPUS
#define PAYLOAD_TYPE_VIDEO  100  // VP8
#define SSRC_AUDIO_BASE  100000
#define SSRC_VIDEO_BASE  200000

namespace RTC
{
	/* Instance methods. */

	Room::Room(unsigned int roomId, Json::Value& data) :
		roomId(roomId)
	{
		MS_TRACE();

		// TODO: do something wit data and throw if incorrect.
	}

	Room::~Room()
	{
		MS_TRACE();
	}

	void Room::HandleCreatePeerRequest(Channel::Request* request)
	{
		MS_TRACE();

		RTC::Peer* peer;
		std::string peerId;

		try
		{
			peer = GetPeerFromRequest(request, &peerId);
		}
		catch (const MediaSoupError &error)
		{
			request->Reject(500, error.what());
			return;
		}

		if (peer)
		{
			MS_ERROR("Peer already exists");

			request->Reject(500, "Peer already exists");
			return;
		}

		try
		{
			peer = new RTC::Peer(this, peerId, request->data);
		}
		catch (const MediaSoupError &error)
		{
			request->Reject(500, error.what());
			return;
		}

		this->peers[peerId] = peer;

		MS_DEBUG("Peer created [peerId:%s]", peerId.c_str());
		request->Accept();
	}

	void Room::HandleClosePeerRequest(Channel::Request* request)
	{
		MS_TRACE();

		RTC::Peer* peer;
		std::string peerId;

		try
		{
			peer = GetPeerFromRequest(request, &peerId);
		}
		catch (const MediaSoupError &error)
		{
			request->Reject(500, error.what());
			return;
		}

		if (!peer)
		{
			MS_ERROR("Peer does not exist");

			request->Reject(500, "Peer does not exist");
			return;
		}

		peer->Close();

		// TODO: Instead of this, the Peer should fire an onPeerClosed() here.
		this->peers.erase(peerId);

		MS_DEBUG("Peer closed [peerId:%s]", peerId.c_str());
		request->Accept();
	}

	void Room::HandleDumpPeerRequest(Channel::Request* request)
	{
		MS_TRACE();

		RTC::Peer* peer;
		std::string peerId;

		try
		{
			peer = GetPeerFromRequest(request, &peerId);
		}
		catch (const MediaSoupError &error)
		{
			request->Reject(500, error.what());
			return;
		}

		if (!peer)
		{
			MS_ERROR("Peer does not exist");

			request->Reject(500, "Peer does not exist");
			return;
		}

		Json::Value jsonPeer = peer->Dump();

		request->Accept(jsonPeer);
	}

	Json::Value Room::Dump()
	{
		MS_TRACE();

		Json::Value json(Json::objectValue);
		Json::Value jsonPeers(Json::objectValue);

		for (auto& kv : this->peers)
		{
			auto peer = kv.second;

			jsonPeers[peer->peerId] = peer->Dump();
		}

		json["peers"] = jsonPeers;

		return json;
	}

	void Room::Close()
	{
		MS_TRACE();

		// Close all the Peers.
		for (auto& kv : this->peers)
		{
			RTC::Peer* peer = kv.second;

			peer->Close();
		}

		delete this;
	}

	RTC::Peer* Room::GetPeerFromRequest(Channel::Request* request, std::string* peerId = nullptr)
	{
		MS_TRACE();

		auto jsonPeerId = request->data["peerId"];

		if (!jsonPeerId.isString())
			MS_THROW_ERROR("Request has no string .peerId field");

		// If given, fill peerId.
		if (peerId)
			*peerId = jsonPeerId.asString();

		auto it = this->peers.find(jsonPeerId.asString());

		if (it != this->peers.end())
		{
			RTC::Peer* peer = it->second;

			return peer;
		}
		else
		{
			return nullptr;
		}
	}

	void Room::onRTPPacket(RTC::Peer* peer, RTC::RTPPacket* packet)
	{
		MS_TRACE();

		// int peer_id = *(int*)peer->GetUserData();
		// MS_BYTE payload_type = packet->GetPayloadType();
		// MS_4BYTES original_ssrc = packet->GetSSRC();

		// MS_DEBUG("valid RTP packet received from Peer %d [ssrc: %llu | payload: %hu]",
		// 	peer_id, (unsigned long long)packet->GetSSRC(), (unsigned short)packet->GetPayloadType());

		// switch (payload_type)
		// {
		// 	case PAYLOAD_TYPE_AUDIO:
		// 	case PAYLOAD_TYPE_VIDEO:
		// 		break;
		// 	default:
		// 		MS_ERROR("payload is not OPUS %d nor VP8 %d, packet ignored", PAYLOAD_TYPE_AUDIO, PAYLOAD_TYPE_VIDEO);
		// 		return;
		// }

		// // Deliver to all the peers.
		// // TODO: but this one? yes (TODO)
		// for (auto dst_peer : this->peers)
		// {
		// 	// if (dst_peer == peer)
		// 	// 	continue;

		// 	int dst_peer_id = *(int*)dst_peer->GetUserData();

		// 	switch (payload_type)
		// 	{
		// 		case PAYLOAD_TYPE_AUDIO:
		// 			packet->SetSSRC((MS_4BYTES)(SSRC_AUDIO_BASE + (peer_id * 10) + dst_peer_id));
		// 			break;
		// 		case PAYLOAD_TYPE_VIDEO:
		// 			packet->SetSSRC((MS_4BYTES)(SSRC_VIDEO_BASE + (peer_id * 10) + dst_peer_id));
		// 			break;
		// 		default:
		// 			MS_ABORT("no puede ser!!!");
		// 			return;
		// 	}

		// 	MS_DEBUG("sending RTP packet to Peer %d [ssrc: %llu | payload: %hu | size: %zu]",
		// 		dst_peer_id, (unsigned long long)packet->GetSSRC(), (unsigned short)packet->GetPayloadType(), packet->GetLength());

		// 	dst_peer->SendRTPPacket(packet);

		// 	// NOTE: recover the previous SSRC since other peers are going to
		// 	// send this same RTPPacket!
		// 	packet->SetSSRC(original_ssrc);
		// }
	}

	void Room::onRTCPPacket(RTC::Peer* peer, RTC::RTCPPacket* packet)
	{
		MS_TRACE();

		// TMP

		// Ignore RTCP packets.

		// for (auto dst_peer : this->peers) {
			// dst_peer->SendRTCPPacket(packet);
		// }
	}
}