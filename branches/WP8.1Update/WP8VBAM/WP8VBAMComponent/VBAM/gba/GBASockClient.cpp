#ifndef NO_LINK
#if defined(_MSC_VER) && _MSC_VER >= 1400 
#pragma warning(push) 
#pragma warning(disable:4996) 
#endif 

#include "GBASockClient.h"

// Currently only for Joybus communications

GBASockClient::GBASockClient()
{
	server_addr = sf::IPAddress::LocalHost;
}

bool GBASockClient::Connect(sf::IPAddress _server_addr) {
	if (_server_addr.IsValid())
		server_addr = _server_addr;

	sf::Socket::Status result = client.Connect(0xd6ba, server_addr);
	//client.SetBlocking(false);

	return result == sf::Socket::Done;
}

GBASockClient::~GBASockClient()
{
	client.Close();
}

void GBASockClient::Send(std::vector<char> data)
{
	char* plain_data = new char[data.size()];
	std::copy(data.begin(), data.end(), plain_data);

	client.Send(plain_data, data.size());

	delete[] plain_data;
}

// Returns cmd for convenience
char GBASockClient::ReceiveCmd(char* data_in)
{
	std::size_t num_received;
	client.Receive(data_in, 5, num_received);

	return data_in[0];
}

#if defined(_MSC_VER) && _MSC_VER >= 1400 
#pragma warning(pop) 
#endif 

#endif // NO_LINK
