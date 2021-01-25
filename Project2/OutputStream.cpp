#include "OutputStream.h"
#include "SocketException.h"
#include "IOErrors.h"

OutputStream::OutputStream(SOCKET socket) : socket(socket), offset(0), maxSize(capacity)
{
	bytes = new byte[maxSize];
	memset(bytes, 0, maxSize);
}

void OutputStream::writeByte(unsigned char byte)
{
	if (check(sizeof(byte)))
	{
		bytes[offset++] = byte;
	}
	else
	{
		extend();
	}
}

void OutputStream::writeInt(int data)
{
	if (check(sizeof(data)))
	{
		for (int i = 0; i < sizeof(data); i++) {
			bytes[offset++] = data >> (8 * i);
		}
	}
	else
	{
		extend();
	}
}

void OutputStream::writeFloat(float data)
{
	if (check(sizeof(data)))
	{
		memcpy(bytes, &data, sizeof(data));
		offset = offset + sizeof(data);
	}
	else
	{
		extend();
	}
}

void OutputStream::writeLong(long data)
{
	if (check(sizeof(data)))
	{
		memcpy(bytes, &data, sizeof(data));
		offset = offset + sizeof(data);
	}
	else
	{
		extend();
	}
}

void OutputStream::writeDouble(double data)
{
	if (check(sizeof(data)))
	{
		memcpy(bytes, &data, sizeof(data));
		offset = offset + sizeof(data);
	}
	else
	{
		extend();
	}
}

void OutputStream::writeUTF8(std::string data)
{
	if (check(sizeof(data)))
	{
		const char* buff = data.c_str();

		int val = size(buff);

		writeInt(val * sizeof(char));

		for (int i = 0; i < val; i++)
		{
			writeByte(buff[i]);
		}
	}
	else
	{
		extend();
	}
}

int OutputStream::size(const char* buff)
{
	int t = 0;
	while (buff[t++] != 0)
	{

	}
	return t;
}

void OutputStream::flush()
{
	int lenght = offset;
	byte* data = bytes;
	while (lenght > 0)
	{
		int amount = send(socket, (const char*)data, lenght, 0);

		if (amount == SOCKET_ERROR)
		{
			throw SocketException(OUTPUT_STREAM_ERROR);
		}
		else
		{
			lenght = lenght - amount;
			data = data + amount;
		}
	}

	delete[] bytes;
	bytes = new byte[maxSize];
	offset = 0;
}

void OutputStream::extend()
{
	maxSize = maxSize + increment;
	byte* newBytes = new byte[maxSize];

	for (int i = 0; i < offset; i++)
	{
		newBytes[i] = bytes[i];
	}

	delete[] bytes;
	bytes = newBytes;
}

bool OutputStream::check(int size)
{
	return (offset + size) < maxSize;
}

