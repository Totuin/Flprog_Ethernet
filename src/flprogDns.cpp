// Arduino DNS client for WizNet5100-based Ethernet shield
// (c) Copyright 2009-2010 MCQN Ltd.
// Released under Apache License, version 2.0

#include <Arduino.h>
#include "flprogEthernet.h"
#include "flprogDns.h"
#include "utility/flprogW5100.h"

#define FLPROG_SOCKET_NONE 255
// Various flags and header field values for a DNS message
#define FLPROG_UDP_HEADER_SIZE 8
#define FLPROG_DNS_HEADER_SIZE 12
#define FLPROG_TTL_SIZE 4
#define FLPROG_QUERY_FLAG (0)
#define FLPROG_RESPONSE_FLAG (1 << 15)
#define FLPROG_QUERY_RESPONSE_MASK (1 << 15)
#define FLPROG_OPCODE_STANDARD_QUERY (0)
#define FLPROG_OPCODE_INVERSE_QUERY (1 << 11)
#define FLPROG_OPCODE_STATUS_REQUEST (2 << 11)
#define FLPROG_OPCODE_MASK (15 << 11)
#define FLPROG_AUTHORITATIVE_FLAG (1 << 10)
#define FLPROG_TRUNCATION_FLAG (1 << 9)
#define FLPROG_RECURSION_DESIRED_FLAG (1 << 8)
#define FLPROG_RECURSION_AVAILABLE_FLAG (1 << 7)
#define FLPROG_RESP_NO_ERROR (0)
#define FLPROG_RESP_FORMAT_ERROR (1)
#define FLPROG_RESP_SERVER_FAILURE (2)
#define FLPROG_RESP_NAME_ERROR (3)
#define FLPROG_RESP_NOT_IMPLEMENTED (4)
#define FLPROG_RESP_REFUSED (5)
#define FLPROG_RESP_MASK (15)
#define FLPROG_TYPE_A (0x0001)
#define FLPROG_CLASS_IN (0x0001)
#define FLPROG_LABEL_COMPRESSION_MASK (0xC0)
// Port number that DNS servers listen on
#define FLPROG_DNS_PORT 53

// Possible return codes from ProcessResponse
#define FLPROG_SUCCESS 1
#define FLPROG_TIMED_OUT -1
#define FLPROG_INVALID_SERVER -2
#define FLPROG_TRUNCATED -3
#define FLPROG_INVALID_RESPONSE -4

FlprogDNSClient::FlprogDNSClient(FlprogEthernetClass *sourse)
{
	ethernet = sourse;
}

void FlprogDNSClient::begin(const IPAddress &aDNSServer)
{
	iDNSServer = aDNSServer;
	iRequestId = 0;
}

int FlprogDNSClient::inet_aton(const char *address, IPAddress &result)
{
	uint16_t acc = 0; // Accumulator
	uint8_t dots = 0;

	while (*address)
	{
		char c = *address++;
		if (c >= '0' && c <= '9')
		{
			acc = acc * 10 + (c - '0');
			if (acc > 255)
			{
				// Value out of [0..255] range
				return 0;
			}
		}
		else if (c == '.')
		{
			if (dots == 3)
			{
				// Too much dots (there must be 3 dots)
				return 0;
			}
			result[dots++] = acc;
			acc = 0;
		}
		else
		{
			// Invalid char
			return 0;
		}
	}

	if (dots != 3)
	{
		// Too few dots (there must be 3 dots)
		return 0;
	}
	result[3] = acc;
	return 1;
}

int FlprogDNSClient::getHostByName(const char *aHostname, IPAddress &aResult, uint16_t timeout)
{
	int ret = 0;

	// See if it's a numeric IP address
	if (inet_aton(aHostname, aResult))
	{
		// It is, our work here is done
		return 1;
	}

	// Check we've got a valid DNS server to use
	if (iDNSServer == INADDR_NONE)
	{
		return FLPROG_INVALID_SERVER;
	}

	// Find a socket to use
	if (ethernet->udpClient()->begin(1024 + (millis() & 0xF)) == 1)
	{
		// Try up to three times
		int retries = 0;
		// while ((retries < 3) && (ret <= 0)) {
		// Send DNS request
		ret = ethernet->udpClient()->beginPacket(iDNSServer, FLPROG_DNS_PORT);
		if (ret != 0)
		{
			// Now output the request data
			ret = BuildRequest(aHostname);
			if (ret != 0)
			{
				// And finally send the request
				ret = ethernet->udpClient()->endPacket();
				if (ret != 0)
				{
					// Now wait for a response
					int wait_retries = 0;
					ret = FLPROG_TIMED_OUT;
					while ((wait_retries < 3) && (ret == FLPROG_TIMED_OUT))
					{
						ret = ProcessResponse(timeout, aResult);
						wait_retries++;
					}
				}
			}
		}
		retries++;
		//}

		// We're done with the socket now
		ethernet->udpClient()->stop();
	}

	return ret;
}

uint16_t FlprogDNSClient::BuildRequest(const char *aName)
{
	// Build header
	//                                    1  1  1  1  1  1
	//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	//    |                      ID                       |
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	//    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	//    |                    QDCOUNT                    |
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	//    |                    ANCOUNT                    |
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	//    |                    NSCOUNT                    |
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	//    |                    ARCOUNT                    |
	//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	// As we only support one request at a time at present, we can simplify
	// some of this header
	iRequestId = millis(); // generate a random ID
	uint16_t twoByteBuffer;

	// FIXME We should also check that there's enough space available to write to, rather
	// FIXME than assume there's enough space (as the code does at present)
	ethernet->udpClient()->write((uint8_t *)&iRequestId, sizeof(iRequestId));
	twoByteBuffer = flprogW5100Htons((int32_t)(FLPROG_QUERY_FLAG | FLPROG_OPCODE_STANDARD_QUERY | FLPROG_RECURSION_DESIRED_FLAG));
	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));

	twoByteBuffer = flprogW5100Htons(1); // One question record
	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));

	twoByteBuffer = 0; // Zero answer records
	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));

	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	// and zero additional records
	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));

	// Build question
	const char *start = aName;
	const char *end = start;
	uint8_t len;
	// Run through the name being requested
	while (*end)
	{
		// Find out how long this section of the name is
		end = start;
		while (*end && (*end != '.'))
		{
			end++;
		}

		if (end - start > 0)
		{
			// Write out the size of this section
			len = end - start;
			ethernet->udpClient()->write(&len, sizeof(len));
			// And then write out the section
			ethernet->udpClient()->write((uint8_t *)start, end - start);
		}
		start = end + 1;
	}

	// We've got to the end of the question name, so
	// terminate it with a zero-length section
	len = 0;
	ethernet->udpClient()->write(&len, sizeof(len));
	// Finally the type and class of question
	twoByteBuffer = flprogW5100Htons(FLPROG_TYPE_A);
	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));

	twoByteBuffer = flprogW5100Htons(FLPROG_CLASS_IN); // Internet class of question
	ethernet->udpClient()->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	// Success!  Everything buffered okay
	return 1;
}

uint16_t FlprogDNSClient::ProcessResponse(uint16_t aTimeout, IPAddress &aAddress)
{
	uint32_t startTime = millis();

	// Wait for a response packet
	while (ethernet->udpClient()->parsePacket() <= 0)
	{
		if ((millis() - startTime) > aTimeout)
		{
			return FLPROG_TIMED_OUT;
		}
		delay(50);
	}

	// We've had a reply!
	// Read the UDP header
	// uint8_t header[DNS_HEADER_SIZE]; // Enough space to reuse for the DNS header
	union
	{
		uint8_t byte[FLPROG_DNS_HEADER_SIZE]; // Enough space to reuse for the DNS header
		uint16_t word[FLPROG_DNS_HEADER_SIZE / 2];
	} header;

	// Check that it's a response from the right server and the right port
	if ((iDNSServer != ethernet->udpClient()->remoteIP()) || (ethernet->udpClient()->remotePort() != FLPROG_DNS_PORT))
	{
		// It's not from who we expected
		return FLPROG_INVALID_SERVER;
	}

	// Read through the rest of the response
	if (ethernet->udpClient()->available() < FLPROG_DNS_HEADER_SIZE)
	{
		return FLPROG_TRUNCATED;
	}
	ethernet->udpClient()->read(header.byte, FLPROG_DNS_HEADER_SIZE);

	uint16_t header_flags = flprogW5100Htons(header.word[1]);
	// Check that it's a response to this request
	if ((iRequestId != (header.word[0])) ||
		((header_flags & FLPROG_QUERY_RESPONSE_MASK) != (uint16_t)FLPROG_RESPONSE_FLAG))
	{
		// Mark the entire packet as read
		ethernet->udpClient()->flush(); // FIXME
		return FLPROG_INVALID_RESPONSE;
	}
	// Check for any errors in the response (or in our request)
	// although we don't do anything to get round these
	if ((header_flags & FLPROG_TRUNCATION_FLAG) || (header_flags & FLPROG_RESP_MASK))
	{
		// Mark the entire packet as read
		ethernet->udpClient()->flush(); // FIXME
		return -5;	  // INVALID_RESPONSE;
	}

	// And make sure we've got (at least) one answer
	uint16_t answerCount = flprogW5100Htons(header.word[3]);
	if (answerCount == 0)
	{
		// Mark the entire packet as read
		ethernet->udpClient()->flush(); // FIXME
		return -6;	  // INVALID_RESPONSE;
	}

	// Skip over any questions
	for (uint16_t i = 0; i < flprogW5100Htons(header.word[2]); i++)
	{
		// Skip over the name
		uint8_t len;
		do
		{
			ethernet->udpClient()->read(&len, sizeof(len));
			if (len > 0)
			{
				// Don't need to actually read the data out for the string, just
				// advance ptr to beyond it
				ethernet->udpClient()->read((uint8_t *)NULL, (size_t)len);
			}
		} while (len != 0);

		// Now jump over the type and class
		ethernet->udpClient()->read((uint8_t *)NULL, 4);
	}

	// Now we're up to the bit we're interested in, the answer
	// There might be more than one answer (although we'll just use the first
	// type A answer) and some authority and additional resource records but
	// we're going to ignore all of them.

	for (uint16_t i = 0; i < answerCount; i++)
	{
		// Skip the name
		uint8_t len;
		do
		{
			ethernet->udpClient()->read(&len, sizeof(len));
			if ((len & FLPROG_LABEL_COMPRESSION_MASK) == 0)
			{
				// It's just a normal label
				if (len > 0)
				{
					// And it's got a length
					// Don't need to actually read the data out for the string,
					// just advance ptr to beyond it
					ethernet->udpClient()->read((uint8_t *)NULL, len);
				}
			}
			else
			{
				// This is a pointer to a somewhere else in the message for the
				// rest of the name.  We don't care about the name, and RFC1035
				// says that a name is either a sequence of labels ended with a
				// 0 length octet or a pointer or a sequence of labels ending in
				// a pointer.  Either way, when we get here we're at the end of
				// the name
				// Skip over the pointer
				ethernet->udpClient()->read((uint8_t *)NULL, 1); // we don't care about the byte
				// And set len so that we drop out of the name loop
				len = 0;
			}
		} while (len != 0);

		// Check the type and class
		uint16_t answerType;
		uint16_t answerClass;
		ethernet->udpClient()->read((uint8_t *)&answerType, sizeof(answerType));
		ethernet->udpClient()->read((uint8_t *)&answerClass, sizeof(answerClass));

		// Ignore the Time-To-Live as we don't do any caching
		ethernet->udpClient()->read((uint8_t *)NULL, FLPROG_TTL_SIZE); // don't care about the returned bytes

		// And read out the length of this answer
		// Don't need header_flags anymore, so we can reuse it here
		ethernet->udpClient()->read((uint8_t *)&header_flags, sizeof(header_flags));

		if ((flprogW5100Htons(answerType) == FLPROG_TYPE_A) && (flprogW5100Htons(answerClass) == FLPROG_CLASS_IN))
		{
			if (flprogW5100Htons(header_flags) != 4)
			{
				// It's a weird size
				// Mark the entire packet as read
				ethernet->udpClient()->flush(); // FIXME
				return -9;	  // INVALID_RESPONSE;
			}
			// FIXME: seeems to lock up here on ESP8266, but why??

			uint8_t flporgConvertTmpBytes[4];
			flporgConvertTmpBytes[0] = aAddress[0];
			flporgConvertTmpBytes[1] = aAddress[1];
			flporgConvertTmpBytes[2] = aAddress[2];
			flporgConvertTmpBytes[3] = aAddress[3];
			ethernet->udpClient()->read(flporgConvertTmpBytes, 4);
			aAddress[0] = flporgConvertTmpBytes[0];
			aAddress[1] = flporgConvertTmpBytes[1];
			aAddress[2] = flporgConvertTmpBytes[2];
			aAddress[3] = flporgConvertTmpBytes[3];
			return FLPROG_SUCCESS;
		}
		else
		{
			// This isn't an answer type we're after, move onto the next one
			ethernet->udpClient()->read((uint8_t *)NULL, flprogW5100Htons(header_flags));
		}
	}

	// Mark the entire packet as read
	ethernet->udpClient()->flush(); // FIXME

	// If we get here then we haven't found an answer
	return -10; // INVALID_RESPONSE;
}
