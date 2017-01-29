/*
 * typeConvert.cpp
 *
 *  Created on: 2016-3-29
 *      Author: vmuser
 */
#include "B_Interface.h"
namespace BInt {
bool getEnumType (const char * c, EnumType &t) {
	int val = atoi(c);
	switch(val) {
	case 0:	t = EnumType::STATION;	return true;
	case 1:	t = EnumType::DEVICE;	return true;
	case 2:	t = EnumType::DI;		return true;
	case 3:	t = EnumType::AI;		return true;
	case 4:	t = EnumType::DO;		return true;
	case 5:	t = EnumType::AO;		return true;
	case 9 :t = EnumType::AREA;		return true;
	default:
		return false;
	}
}
EnumState getEnumState (const char * c) {
	int val = atoi(c);
	switch(val) {
	case 0:	return EnumState::NOALARM;
	case 1:	return EnumState::CRITICAL;
	case 2:	return EnumState::MAJOR;
	case 3:	return EnumState::MINOR;
	case 4:	return EnumState::HINT;
	case 5:	return EnumState::OPEVENT;
	default:return EnumState::INVALID;
	}
}
ostream& operator << (ostream& out,EnumType tp) {
    switch(tp) {
        case EnumType::STATION:
            out << "STATION ";break;
        case EnumType::DEVICE:
            out << "DEVICE ";break;
        case EnumType::DI:
            out << "DI ";break;
        case EnumType::AI:
            out << "AI ";break;
        case EnumType::DO:
            out << "DO ";break;
        case EnumType::AO:
            out << "AO ";break;
       case EnumType::AREA:
            out << "AREA ";break;
        default:
        	cout << "Bad type of EnumType";
        	break;
    }
    return out;
}
ostream& operator << (ostream& out,EnumState st) {
    switch(st) {
        case EnumState::NOALARM:
            out << "NOALARM ";break;
        case EnumState::CRITICAL:
            out << "CRITICAL ";break;
        case EnumState::MAJOR:
            out << "MAJOR ";break;
        case EnumState::MINOR:
            out << "MINOR";break;
        case EnumState::HINT:
            out << "HINT ";break;
        case EnumState::OPEVENT:
            out << "OPEVENT";break;
        default:
        	cout << "Bad type of EnumState";
        	break;
    }
    return out;
}
ostream& operator << (ostream& out,EnumFlag f) {
    switch(f) {
        case EnumFlag::BEGIN:
            out << "BEGIN ";break;
        case EnumFlag::END:
            out << "END ";break;
        default:
        	cout << "Bad flag of EnumFlag";
        	break;
    }
    return out;
}
};



