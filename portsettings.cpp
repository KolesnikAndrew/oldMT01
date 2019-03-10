/**
 * @file      portsettings.cpp
 * @brief     Описание файла
 * @details   Детальное описание файла (необязательное поле)
 * @note      Заметка (необязательное поле)
 * @author    Инженер-программист Пясецкий Владимир 
 * @copyright © TRIOLCORP, 2017
 */

#include <portsettings.h>
#include <iostream>

using namespace std;

PortSettings::~PortSettings()
{
    std::cerr << "[debug]: Port settings finished" << std::endl;
}
//debug info
std::ostream& operator<<(std::ostream& stream, const PortSettings& instance)
{
    return instance.print(stream);
}

std::ostream& PortSettings::print(std::ostream& stream) const{
	 return stream << "[debug]: PortSettings. ";

}
//debug info

RtuPortSettings::RtuPortSettings() :
        baud(115200), parity('N'), dataBit(8), stopBit(1), byteTimeout( { 0,
                500000 }), responseTimeout( { 0, 500000 })
{
    std::clog << "[debug]: RTU Port set" << std::endl;
}
std::ostream& RtuPortSettings::print(std::ostream& stream) const{

	return stream  << "[debug]: baud:                    " << baud << endl
			<< "[debug]: parity:                  " << parity << endl
			<< "[debug]: dataBit:                 " << dataBit << endl
			<< "[debug]: stopBit:                 " << stopBit << endl
			<< "[debug]: byteTimeout.tv_sec:      " << byteTimeout.tv_sec << endl
			<< "[debug]: byteTimeout.tv_usec:     " << byteTimeout.tv_usec << endl
			<< "[debug]: responseTimeout.tv_sec:  " << responseTimeout.tv_sec << endl
			<< "[debug]: responseTimeout.tv_usec: " << responseTimeout.tv_usec << endl
			<< "[debug]: portName               : " << portName << endl;
}


TcpPortSettings::TcpPortSettings() :
        netAddress("0.0.0.0"), servicePort(61502), modbusMode("master"), netMask("255.0.0.0"), netGateway ("0.0.0.0")
{
    std::clog << "[debug]: TCP Port set" << std::endl;
}


std::ostream& TcpPortSettings::print(std::ostream& stream) const {
	return stream   << "[debug]: netAddress:    " << netAddress << endl
					<< "[debug]: netMask:       " << netMask << endl
					<< "[debug]: netGateway:    " << netGateway << endl
					<< "[debug]: servicePort:   " << servicePort << endl
					<< "[debug]: modbusMode:    " << modbusMode << endl
					<< "[debug]: portName:      " << portName << endl;
}


