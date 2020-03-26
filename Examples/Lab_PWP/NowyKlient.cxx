/*=========================================================================

  Program:   OpenIGTLink -- Example for Tracker Client Program
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <math.h>
#include <cstdlib>
#include "igtlOSUtil.h"
#include "igtlPointMessage.h"
#include "igtlClientSocket.h"

int ReceivePoint(igtl::Socket * socket, igtl::MessageHeader * header);

int main(int argc, char* argv[])
{
	if (argc != 3) // check number of arguments
	{
		std::cerr << "Usage: " << argv[0] << " <hostname> <port> <fps>" << std::endl;
		std::cerr << "    <hostname> : IP or host name" << std::endl;
		std::cerr << "    <port>     : Port # (18944 in Slicer default)" << std::endl;
		exit(0);
	}

	char*  hostname = argv[1];
	int    port = atoi(argv[2]);

	igtl::ClientSocket::Pointer socket;
	socket = igtl::ClientSocket::New();
	int r = socket->ConnectToServer(hostname, port);

	if (r != 0)
	{
		std::cerr << "Cannot connect to the server." << std::endl;
		exit(0);
	}

	igtl::PointMessage::Pointer pointMsg;
	pointMsg = igtl::PointMessage::New();
	pointMsg->SetDeviceName("PointSender");

	igtl::PointElement::Pointer point0;
	point0 = igtl::PointElement::New();
	point0->SetName("POINT_0");
	point0->SetGroupName("GROUP_0");
	point0->SetRGBA(0xFF, 0x00, 0x00, 0xFF);
	point0->SetPosition(10.0, 20.0, 30.0);
	point0->SetRadius(15.0);
	point0->SetOwner("IMAGE_0");

	igtl::PointElement::Pointer point1;
	point1 = igtl::PointElement::New();
	point1->SetName("POINT_1");
	point1->SetGroupName("GROUP_0");
	point1->SetRGBA(0x00, 0xFF, 0x00, 0xFF);
	point1->SetPosition(40.0, 50.0, 60.0);
	point1->SetRadius(45.0);
	point1->SetOwner("IMAGE_0");

	igtl::PointElement::Pointer point2;
	point2 = igtl::PointElement::New();
	point2->SetName("POINT_2");
	point2->SetGroupName("GROUP_0");
	point2->SetRGBA(0x00, 0x00, 0xFF, 0xFF);
	point2->SetPosition(70.0, 80.0, 90.0);
	point2->SetRadius(75.0);
	point2->SetOwner("IMAGE_0");

	pointMsg->AddPointElement(point0);
	pointMsg->AddPointElement(point1);
	pointMsg->AddPointElement(point2);
	pointMsg->Pack();

	socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());

	igtl::MessageHeader::Pointer headerMsg;
	headerMsg = igtl::MessageHeader::New();

	while (true) {
		headerMsg->InitPack();
		int r = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
		if (r != headerMsg->GetPackSize())
		{
			continue;
		}
		headerMsg->Unpack();
		ReceivePoint(socket, headerMsg);
	}
	socket->CloseSocket();
}

int ReceivePoint(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving POINT data type." << std::endl;

	igtl::PointMessage::Pointer pointMsg;
	pointMsg = igtl::PointMessage::New();
	pointMsg->SetMessageHeader(header);
	pointMsg->AllocatePack();

	socket->Receive(pointMsg->GetPackBodyPointer(), pointMsg->GetPackBodySize());

	int c = pointMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY)
	{
		int nElements = pointMsg->GetNumberOfPointElement();
		for (int i = 0; i < nElements; i++)
		{
			igtl::PointElement::Pointer pointElement;
			pointMsg->GetPointElement(i, pointElement);

			igtlUint8 rgba[4];
			pointElement->GetRGBA(rgba);

			igtlFloat32 pos[3];
			pointElement->GetPosition(pos);

			std::cerr << "==========Element# " << i << " ==========" << std::endl;
			std::cerr << " Position: ( " << std::fixed << pos[0] << ", " << pos[1] << ", " << pos[2] << " )" << std::endl;
			std::cerr << "================================" << std::endl;
		}
	}
	return 1;
}