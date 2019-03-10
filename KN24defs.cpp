/**
 * @file    KN24defs.cpp
 * @date    Created on: 22 жовт. 2018 р.
 * @author  Author: v7r
 */

#include <iostream>
#include <iomanip>
#include <cstring>

#include "KN24defs.h"

TPrm::TPrm() :
		IDgrp(0), IDPrm(0), Power(0), kAmp(0) {

	for (int i = 0; i < sizeof(Name) /sizeof(Name[0]) ; ++i) {
		Name[i] = 0;
	}
	for (int i = 0; i < sizeof(EdIzm) / sizeof(EdIzm[0]); ++i) {
		EdIzm[i] = 0;
	}
	for (int i = 0; i < sizeof(unused) / sizeof(unused[0]); ++i) {
		unused[i] = 0;
	}
}

headOfLogFileSave_type::headOfLogFileSave_type() :
		CRC(0), Ntic(0), Nvar(0), TimeOneTick(0), LogInd(0), Sz(0), DeviceType(
				0), UnixTime(0) {

	for (int i = 0; i < sizeof(DeviceName) / sizeof(DeviceName[0]); ++i) {
		DeviceName[i] = 0;
	}
	for (int i = 0; i < sizeof(VendorName) / sizeof(VendorName[0]); ++i) {
		VendorName[i] = 0;
	}

	for (int i = 0; i < sizeof(PrmLog) / sizeof(PrmLog[0]); ++i) {
		PrmLog[i] = TPrm();
	}
	for (int i = 0; i < sizeof(ErrStatLor) / sizeof(ErrStatLor[0]); ++i) {
		ErrStatLor[i] = 0;
	}

}

headOfLogFile_type::headOfLogFile_type() :
		Ntic(0), Nvar(0), TimeOneTick(0), LogInd(0), Sz(0), rez2(0), rez3(0) {
	for (int i = 0; i < sizeof(pntPrm) / sizeof(pntPrm[0]); i++) {
		pntPrm[i] = 0;
	}
	for (int i = 0; i < sizeof(kAmp) / sizeof(kAmp[0]); i++) {
		kAmp[i] = 0;
	}
	for (int i = 0; i < sizeof(res) / sizeof(res[0]); i++) {
		res[i] = 0;
	}

}

//////////////////////////////////////////////////////////////////////
/// Overloaded operators for debug structures
//////////////////////////////////////////////////////////////////////
/**
 * @brief Stream output operator for headOfParamDescFile_typ
 */
std::ostream& operator <<(std::ostream&stream,
		const headOfParamDescFile_type& fl) {

	return stream << "[debug] headOfParamDescFile_type: " << std::endl
			<< "FileCRC        " << std::hex << (int) fl.FileCrc << std::dec
			<< std::endl << "Sign           " << (int) fl.Sign << std::endl
			<< "SizeOfFileDesc " << (int) fl.SizeOfFileDescr << std::endl
			<< "MenuStart      " << (int) fl.MenuStart << std::endl
			<< "PrmStart       " << (int) fl.PrmStart << std::endl
			<< "QuantityMenu   " << (int) fl.QuantityMenu << std::endl
			<< "QuantityPrm    " << (int) fl.QuantityPrm << std::endl
			<< "VendorName[32] " << fl.VendorName << std::endl
			<< "DeviceName[32] " << fl.DeviceName << std::endl
			<< "DeviceType     " << (int) fl.DeviceType << std::endl
			<< "TopOfJrnFlt    " << (int) fl.TopOfJrnFlt << std::endl
			<< "NumOfRecFlt    " << (int) fl.NumOfRecFlt << std::endl
			<< "TopOfJrnAl     " << (int) fl.TopOfJrnAl << std::endl
			<< "NumOfRecAl     " << (int) fl.NumOfRecAl << std::endl
			<< "AdrListPrmJrn  " << (int) fl.AdrListPrmJrn << std::endl
			<< "NumPrmInJrn    " << (int) fl.NumPrmInJrn << std::endl
			<< "QuantityUst    " << (int) fl.QuantityUst << std::endl
			<< "JrnStart       " << (int) fl.JrnStart << std::endl
			<< "QuantityJrn    " << (int) fl.QuantityJrn << std::endl
			<< "Res[8]         " << (int) fl.Res[0] << std::endl
			<< "NomProtocol    " << (int) fl.NomProtocol << std::endl
			<< "SpecialPrm     " << (int) fl.SpecialPrm << std::endl
			<< "QuantySpeclPrm " << (int) fl.QuantitySpecialPrm << std::endl
			<< "AdrPrmTDrive   " << (int) fl.AdrTDrivePrm << std::endl
			<< "HeaderCrc      " << std::hex << (int) fl.HeaderCrc << std::dec;
}

std::ostream& operator<<(std::ostream& stream, const DescriptorPrm_type& prm) {
	return stream << "[debug] Descriptor_prm_type: " << "\nIDGrp:     "
			<< (int) prm.IDGrp << "\nIDPrm:     " << (int) prm.IDPrm
			<< "\nUnit:      " << (int) prm.Unit
			/*<< "\nFlgPrm     " << prm.FlgPrm*/
			<< "\nLowLim     " << prm.LowLim << "\nHighLim    " << prm.HighLim
			<< "\nOfsTxt     " << prm.OfsTxt << "\nDefaultSet "
			<< prm.DefaultSet << "\nName[16]   " << prm.Name << "\nLenHlp     "
			<< prm.LenHlp << "\nOfsHlp     " << prm.OfsHlp << "\nFunctional "
			<< prm.Functional << "\nAdrVal     " << prm.AdrVal
			<< "\nVisible    " << prm.Visible << "\ntmp        " << prm.tmp;
}

using namespace std;
std::ostream& operator<<(std::ostream& stream, const headOfLogFile_type& head) {
	stream << "\n[debug] Log header output :" << std::endl << "Ntic:        "
			<< head.Ntic << endl << "Nvar:        " << head.Nvar << endl
			<< "LogInd:      " << head.LogInd << endl << "Sz:          "
			<< head.Sz << endl << "TimeOneTick: " << head.TimeOneTick;

	stream << endl << "pntPrm: " << endl;
	for (int i = 0; i < 9; ++i) {
		stream << head.pntPrm[i] << " ";
	}
	stream << endl << "kAmp: " << endl;
	for (int i = 0; i < 9; ++i) {
		stream << head.kAmp[i] << " ";
	}
	return stream;

}

