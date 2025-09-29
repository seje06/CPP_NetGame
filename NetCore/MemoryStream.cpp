#include "MyGamePCH.h"

void OutputMemoryStream::Write(const Obj* inGameObject)
{
	uint32_t networkId = mLinkingContext->GetNetworkId(const_cast<Obj*>(inGameObject), false);
	Write(networkId);
}

void OutputMemoryStream::ReallocBuffer(uint32_t inNewLength)
{
	mBuffer = static_cast<char*>(std::realloc(mBuffer, inNewLength));
	//handle realloc failure
	//...
	mCapacity = inNewLength;
}

void OutputMemoryStream::Init()
{
	mHead = 0;
}

void OutputMemoryStream::Write(const void* inData,
	size_t inByteCount)
{
	//make sure we have space...
	uint32_t resultHead = mHead + static_cast<uint32_t>(inByteCount);
	if (resultHead > mCapacity)
	{
		ReallocBuffer(std::max(mCapacity * 2, resultHead));
	}

	//copy into buffer at head
	std::memcpy(mBuffer + mHead, inData, inByteCount);

	//increment head for next write
	mHead = resultHead;
}


void InputMemoryStream::Read(void* outData,
	uint32_t inByteCount)
{
	uint32_t resultHead = mHead + inByteCount;
	if (resultHead > mCapacity)
	{
		//handle error, no data to read!
		//...
	}

	std::memcpy(outData, mBuffer + mHead, inByteCount);

  	mHead = resultHead;
}

void InputMemoryStream::Read(Obj*& outGameObject)
{
	uint32_t networkId;
	Read(networkId);
	outGameObject = mLinkingContext->GetGameObject(networkId);
}

template< typename T > void OutputMemoryStream::Write(T inData)
{
	static_assert(std::is_arithmetic< T >::value ||
		std::is_enum< T >::value,
		"Generic Write only supports primitive data types");

	if (STREAM_ENDIANNESS == PLATFORM_ENDIANNESS)
	{
		Write(&inData, sizeof(inData));
	}
	else
	{
		T swappedData = ByteSwap(inData);
		Write(&swappedData, sizeof(swappedData));
	}
}