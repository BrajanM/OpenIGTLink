/*=========================================================================

  Program:   OpenIGTLink -- Example for Data Receiving Server Program
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <iomanip>
#include <math.h>
#include <cstdlib>
#include <cstring>
#include "igtlOSUtil.h"
#include "igtlMessageHeader.h"
#include "igtlTransformMessage.h"
#include "igtlImageMessage.h"
#include "igtlServerSocket.h"
#include "igtlStatusMessage.h"
#include "igtlPositionMessage.h"

#if OpenIGTLink_PROTOCOL_VERSION >= 2
#include "igtlPointMessage.h"
#include "igtlTrajectoryMessage.h"
#include "igtlStringMessage.h"
#include "igtlBindMessage.h"
#include "igtlCapabilityMessage.h"
#endif 

int ReceiveTransform(igtl::Socket * socket, igtl::MessageHeader * header);
int ReceivePosition(igtl::Socket * socket, igtl::MessageHeader * header);
int ReceiveImage(igtl::Socket * socket, igtl::MessageHeader * header);
int ReceiveStatus(igtl::Socket * socket, igtl::MessageHeader * header);

#if OpenIGTLink_PROTOCOL_VERSION >= 2
int ReceivePoint(igtl::Socket * socket, igtl::MessageHeader * header);
int ReceiveTrajectory(igtl::Socket * socket, igtl::MessageHeader::Pointer& header);
int ReceiveString(igtl::Socket * socket, igtl::MessageHeader * header);
int ReceiveBind(igtl::Socket * socket, igtl::MessageHeader * header);
int ReceiveCapability(igtl::Socket * socket, igtl::MessageHeader * header);
#endif

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
		std::cerr << "    <port>     : Port # (18944 in Slicer default)" << std::endl;
		exit(0);
	}

	int    port = atoi(argv[1]);

	igtl::ServerSocket::Pointer serverSocket;
	serverSocket = igtl::ServerSocket::New();
	int r = serverSocket->CreateServer(port);

	if (r < 0)
	{
		std::cerr << "Cannot create a server socket." << std::endl;
		exit(0);
	}

	igtl::Socket::Pointer socket;

	while (1)
	{
		socket = serverSocket->WaitForConnection(1000);

		if (socket.IsNotNull())
		{
			igtl::MessageHeader::Pointer headerMsg;
			headerMsg = igtl::MessageHeader::New();

			igtl::TimeStamp::Pointer ts;
			ts = igtl::TimeStamp::New();

			for (int i = 0; i < 100; i++)
			{
				headerMsg->InitPack();

				int r = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
				if (r == 0)
				{
					socket->CloseSocket();
				}
				if (r != headerMsg->GetPackSize())
				{
					continue;
				}

				headerMsg->Unpack();

				igtlUint32 sec;
				igtlUint32 nanosec;

				headerMsg->GetTimeStamp(ts);
				ts->GetTimeStamp(&sec, &nanosec);

				std::cerr << "Time stamp: "
					<< sec << "." << std::setw(9) << std::setfill('0')
					<< nanosec << std::endl;

				if (strcmp(headerMsg->GetDeviceType(), "TRANSFORM") == 0)
				{
					ReceiveTransform(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "POSITION") == 0)
				{
					ReceivePosition(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "IMAGE") == 0)
				{
					ReceiveImage(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "STATUS") == 0)
				{
					ReceiveStatus(socket, headerMsg);
				}
#if OpenIGTLink_PROTOCOL_VERSION >= 2
				else if (strcmp(headerMsg->GetDeviceType(), "POINT") == 0)
				{
					ReceivePoint(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "TRAJ") == 0)
				{
					ReceiveTrajectory(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "STRING") == 0)
				{
					ReceiveString(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "BIND") == 0)
				{
					ReceiveBind(socket, headerMsg);
				}
				else if (strcmp(headerMsg->GetDeviceType(), "CAPABILITY") == 0)
				{
					ReceiveCapability(socket, headerMsg);
				}
#endif
				else
				{
					std::cerr << "Receiving : " << headerMsg->GetDeviceType() << std::endl;
					std::cerr << "Size : " << headerMsg->GetBodySizeToRead() << std::endl;
					socket->Skip(headerMsg->GetBodySizeToRead(), 0);
				}
			}
		}
	}
	socket->CloseSocket();
}

int ReceiveTransform(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving TRANSFORM data type." << std::endl;

	igtl::TransformMessage::Pointer transMsg;
	transMsg = igtl::TransformMessage::New();
	transMsg->SetMessageHeader(header);
	transMsg->AllocatePack();

	socket->Receive(transMsg->GetPackBodyPointer(), transMsg->GetPackBodySize());

	int c = transMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY)
	{
		igtl::Matrix4x4 matrix;
		transMsg->GetMatrix(matrix);
		igtl::PrintMatrix(matrix);
		return 1;
	}
	return 0;
}

int ReceivePosition(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving POSITION data type." << std::endl;

	igtl::PositionMessage::Pointer positionMsg;
	positionMsg = igtl::PositionMessage::New();
	positionMsg->SetMessageHeader(header);
	positionMsg->AllocatePack();

	socket->Receive(positionMsg->GetPackBodyPointer(), positionMsg->GetPackBodySize());

	int c = positionMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY)
	{
		float position[3];
		float quaternion[4];

		positionMsg->GetPosition(position);
		positionMsg->GetQuaternion(quaternion);

		std::cerr << "position   = (" << position[0] << ", " << position[1] << ", " << position[2] << ")" << std::endl;
		std::cerr << "quaternion = (" << quaternion[0] << ", " << quaternion[1] << ", "
			<< quaternion[2] << ", " << quaternion[3] << ")" << std::endl << std::endl;

		return 1;
	}
	return 0;
}

int ReceiveImage(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving IMAGE data type." << std::endl;

	igtl::ImageMessage::Pointer imgMsg;
	imgMsg = igtl::ImageMessage::New();
	imgMsg->SetMessageHeader(header);
	imgMsg->AllocatePack();

	socket->Receive(imgMsg->GetPackBodyPointer(), imgMsg->GetPackBodySize());

	int c = imgMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY)
	{
		int   size[3];          
		float spacing[3];      
		int   svsize[3];       
		int   svoffset[3];     
		int   scalarType;      
		int   endian;           

		scalarType = imgMsg->GetScalarType();
		endian = imgMsg->GetEndian();
		imgMsg->GetDimensions(size);
		imgMsg->GetSpacing(spacing);
		imgMsg->GetSubVolume(svsize, svoffset);

		std::cerr << "Device Name           : " << imgMsg->GetDeviceName() << std::endl;
		std::cerr << "Scalar Type           : " << scalarType << std::endl;
		std::cerr << "Endian                : " << endian << std::endl;
		std::cerr << "Dimensions            : ("
			<< size[0] << ", " << size[1] << ", " << size[2] << ")" << std::endl;
		std::cerr << "Spacing               : ("
			<< spacing[0] << ", " << spacing[1] << ", " << spacing[2] << ")" << std::endl;
		std::cerr << "Sub-Volume dimensions : ("
			<< svsize[0] << ", " << svsize[1] << ", " << svsize[2] << ")" << std::endl;
		std::cerr << "Sub-Volume offset     : ("
			<< svoffset[0] << ", " << svoffset[1] << ", " << svoffset[2] << ")" << std::endl;
		return 1;
	}
	return 0;
}

int ReceiveStatus(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving STATUS data type." << std::endl;

	igtl::StatusMessage::Pointer statusMsg;
	statusMsg = igtl::StatusMessage::New();
	statusMsg->SetMessageHeader(header);
	statusMsg->AllocatePack();

	socket->Receive(statusMsg->GetPackBodyPointer(), statusMsg->GetPackBodySize());

	int c = statusMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY)
	{
		std::cerr << "========== STATUS ==========" << std::endl;
		std::cerr << " Code      : " << statusMsg->GetCode() << std::endl;
		std::cerr << " SubCode   : " << statusMsg->GetSubCode() << std::endl;
		std::cerr << " Error Name: " << statusMsg->GetErrorName() << std::endl;
		std::cerr << " Status    : " << statusMsg->GetStatusString() << std::endl;
		std::cerr << "============================" << std::endl;
	}
	return 0;
}

#if OpenIGTLink_PROTOCOL_VERSION >= 2
int ReceivePoint(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving POINT data type." << std::endl;

	igtl::PointMessage::Pointer pointMsg;
	pointMsg = igtl::PointMessage::New();
	pointMsg->SetMessageHeader(header);
	pointMsg->AllocatePack();

	igtl::PointMessage::Pointer pointMsgSend;
	pointMsgSend = igtl::PointMessage::New();
	pointMsgSend->SetDeviceName("PointSender");

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

			std::cerr << "========== Element #" << i << " ==========" << std::endl;
			std::cerr << " Name      : " << pointElement->GetName() << std::endl;
			std::cerr << " GroupName : " << pointElement->GetGroupName() << std::endl;
			std::cerr << " RGBA      : ( " << (int)rgba[0] << ", " << (int)rgba[1] << ", " << (int)rgba[2] << ", " << (int)rgba[3] << " )" << std::endl;
			std::cerr << " Position  : ( " << std::fixed << pos[0] << ", " << pos[1] << ", " << pos[2] << " )" << std::endl;
			std::cerr << " Radius    : " << std::fixed << pointElement->GetRadius() << std::endl;
			std::cerr << " Owner     : " << pointElement->GetOwner() << std::endl;
			std::cerr << "================================" << std::endl;

			igtlFloat32 negpos[3];
			for (int j = 0; j < 3; j++) {
				negpos[j] = pos[j] * -1;
			}

			igtl::PointElement::Pointer negpoint;
			negpoint = igtl::PointElement::New();
			negpoint->SetName(pointElement->GetName());
			negpoint->SetGroupName(pointElement->GetGroupName());
			negpoint->SetRGBA(rgba);
			negpoint->SetPosition(negpos);
			negpoint->SetRadius(pointElement->GetRadius());
			negpoint->SetOwner(pointElement->GetOwner());


			pointMsgSend->AddPointElement(negpoint);
		}
		igtl::Sleep(5000);
		pointMsgSend->Pack();
		socket->Send(pointMsgSend->GetPackPointer(), pointMsgSend->GetPackSize());
	}
	return 1;
}

int ReceiveTrajectory(igtl::Socket * socket, igtl::MessageHeader::Pointer& header)
{
	std::cerr << "Receiving TRAJECTORY data type." << std::endl;

	igtl::TrajectoryMessage::Pointer trajectoryMsg;
	trajectoryMsg = igtl::TrajectoryMessage::New();
	trajectoryMsg->SetMessageHeader(header);
	trajectoryMsg->AllocatePack();

	socket->Receive(trajectoryMsg->GetPackBodyPointer(), trajectoryMsg->GetPackBodySize());

	int c = trajectoryMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY)
	{
		int nElements = trajectoryMsg->GetNumberOfTrajectoryElement();
		for (int i = 0; i < nElements; i++)
		{
			igtl::TrajectoryElement::Pointer trajectoryElement;
			trajectoryMsg->GetTrajectoryElement(i, trajectoryElement);

			igtlUint8 rgba[4];
			trajectoryElement->GetRGBA(rgba);

			igtlFloat32 entry[3];
			igtlFloat32 target[3];
			trajectoryElement->GetEntryPosition(entry);
			trajectoryElement->GetTargetPosition(target);

			std::cerr << "========== Element #" << i << " ==========" << std::endl;
			std::cerr << " Name      : " << trajectoryElement->GetName() << std::endl;
			std::cerr << " GroupName : " << trajectoryElement->GetGroupName() << std::endl;
			std::cerr << " RGBA      : ( " << (int)rgba[0] << ", " << (int)rgba[1] << ", " << (int)rgba[2] << ", " << (int)rgba[3] << " )" << std::endl;
			std::cerr << " Entry Pt  : ( " << std::fixed << entry[0] << ", " << entry[1] << ", " << entry[2] << " )" << std::endl;
			std::cerr << " Target Pt : ( " << std::fixed << target[0] << ", " << target[1] << ", " << target[2] << " )" << std::endl;
			std::cerr << " Radius    : " << std::fixed << trajectoryElement->GetRadius() << std::endl;
			std::cerr << " Owner     : " << trajectoryElement->GetOwner() << std::endl;
			std::cerr << "================================" << std::endl << std::endl;
		}
	}
	return 1;
}

int ReceiveString(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving STRING data type." << std::endl;

	igtl::StringMessage::Pointer stringMsg;
	stringMsg = igtl::StringMessage::New();
	stringMsg->SetMessageHeader(header);
	stringMsg->AllocatePack();

	socket->Receive(stringMsg->GetPackBodyPointer(), stringMsg->GetPackBodySize());

	int c = stringMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY) 
	{
		std::cerr << "Encoding: " << stringMsg->GetEncoding() << "; "
			<< "String: " << stringMsg->GetString() << std::endl;
	}
	return 1;
}

int ReceiveBind(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving BIND data type." << std::endl;

	igtl::BindMessage::Pointer bindMsg;
	bindMsg = igtl::BindMessage::New();
	bindMsg->SetMessageHeader(header);
	bindMsg->AllocatePack();

	socket->Receive(bindMsg->GetPackBodyPointer(), bindMsg->GetPackBodySize());

	int c = bindMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY) 
	{
		int n = bindMsg->GetNumberOfChildMessages();

		for (int i = 0; i < n; i++)
		{
			if (strcmp(bindMsg->GetChildMessageType(i), "STRING") == 0)
			{
				igtl::StringMessage::Pointer stringMsg;
				stringMsg = igtl::StringMessage::New();
				bindMsg->GetChildMessage(i, stringMsg);
				stringMsg->Unpack(0);
				std::cerr << "Message type: STRING" << std::endl;
				std::cerr << "Message name: " << stringMsg->GetDeviceName() << std::endl;
				std::cerr << "Encoding: " << stringMsg->GetEncoding() << "; "
					<< "String: " << stringMsg->GetString() << std::endl;
			}
			else if (strcmp(bindMsg->GetChildMessageType(i), "TRANSFORM") == 0)
			{
				igtl::TransformMessage::Pointer transMsg;
				transMsg = igtl::TransformMessage::New();
				bindMsg->GetChildMessage(i, transMsg);
				transMsg->Unpack(0);
				std::cerr << "Message type: TRANSFORM" << std::endl;
				std::cerr << "Message name: " << transMsg->GetDeviceName() << std::endl;
				igtl::Matrix4x4 matrix;
				transMsg->GetMatrix(matrix);
				igtl::PrintMatrix(matrix);
			}
		}
	}
	return 1;
}

int ReceiveCapability(igtl::Socket * socket, igtl::MessageHeader * header)
{
	std::cerr << "Receiving CAPABILITY data type." << std::endl;

	igtl::CapabilityMessage::Pointer capabilMsg;
	capabilMsg = igtl::CapabilityMessage::New();
	capabilMsg->SetMessageHeader(header);
	capabilMsg->AllocatePack();

	socket->Receive(capabilMsg->GetPackBodyPointer(), capabilMsg->GetPackBodySize());

	int c = capabilMsg->Unpack(1);

	if (c & igtl::MessageHeader::UNPACK_BODY) 
	{
		int nTypes = capabilMsg->GetNumberOfTypes();
		for (int i = 0; i < nTypes; i++)
		{
			std::cerr << "Typename #" << i << ": " << capabilMsg->GetType(i) << std::endl;
		}
	}
	return 1;
}
#endif
