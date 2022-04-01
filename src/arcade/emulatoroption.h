#ifndef EMULATOROPTIONS_H
#define EMULATOROPTIONS_H

#include "macros.h"

class EmulatorOption
{
public:
	QString name, shortname, dvalue, value;
	int type;

	explicit EmulatorOption(QString n, QString sn, QString t, QString dv, QString v) {
		name = n;
		shortname = sn;
		type = QMC2_ARCADE_EMUOPT_UNKNOWN;
		switch ( t.at(0).toLatin1() ) {
		case 'b':
			type = QMC2_ARCADE_EMUOPT_BOOL;
			break;
		case 'i':
			type = QMC2_ARCADE_EMUOPT_INT;
			break;
		case 'f':
			if ( t == "float" || t == "float1" )
				type = QMC2_ARCADE_EMUOPT_FLOAT;
			else if ( t == "file" )
				type = QMC2_ARCADE_EMUOPT_FILE;
			else if ( t == "float2" )
				type = QMC2_ARCADE_EMUOPT_FLOAT2;
			else if ( t == "float3" )
				type = QMC2_ARCADE_EMUOPT_FLOAT3;
			break;
		case 's':
			type = QMC2_ARCADE_EMUOPT_STRING;
			break;
		case 'd':
			type = QMC2_ARCADE_EMUOPT_FOLDER;
			break;
		case 'c':
			type = QMC2_ARCADE_EMUOPT_COMBO;
			break;
		default: type = QMC2_ARCADE_EMUOPT_UNKNOWN;
			break;
		}
		dvalue = dv;
		value = v;
	}
	static bool lessThan(const EmulatorOption &o1, const EmulatorOption &o2) {
		return o1.name < o2.name;
	}
};

#endif
