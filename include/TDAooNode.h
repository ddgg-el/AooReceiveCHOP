#pragma once

#include "TDAooCommon.h"

namespace TD 
{

/**
 * @brief Base class for AooNode
 * 
 */
class INode {
	public:
		using ptr = std::shared_ptr<INode>;

		static INode::ptr get(int port);

		virtual ~INode() {}

		virtual int port() const = 0;
	
		virtual AooClient * client() = 0;
	
		virtual bool registerClient(AooClient *c) = 0;
	
		virtual void unregisterClient(AooClient *c) = 0;
	
		virtual void notify() = 0;
	
		// void sendReply(const osc::OutboundPacketStream& msg) {
		// 	sendReply(msg.Data(), msg.Size());
		// };

		virtual void sendReply(const char *data, size_t size) = 0;


	};
}