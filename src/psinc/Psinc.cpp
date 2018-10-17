#include "psinc/Psinc.h"

#define PSINC_VERSION "0.1.22"


namespace psinc
{
	// constexpr const char *Month(const char *date)
	// {
	// 	switch (date[0]) {
	// 		case 'J':	switch (date[1]) {
	// 			case 'a': return "01";
	// 			case 'u': return date[2] == 'n' ? "06" : "07";
	// 		}
	// 		case 'F':	return "02";
	// 		case 'M':	return date[2] == 'r' ? "03" : "05";
	// 		case 'A':	return date[1] == 'p' ? "04" : "08";
	// 		case 'S':	return "09";
	// 		case 'O':	return "10";
	// 		case 'N':	return "11";
	// 		case 'D':	return "12";
	// 		default:	return "00";
	// 	}
	// }


	const char *Version()
	{
		return PSINC_VERSION " (built " __DATE__ " " __TIME__ ")";
	}
}

