#include "machineobject.h"

MachineObject::MachineObject(const QString &myId, const QString &myParentId, const QString &myDescription, int myRomState, QObject *parent)
	: QObject(parent)
{
	setId(myId);
	setParentId(myParentId);
	setDescription(myDescription);
	setRomState(myRomState);
}

MachineObject::~MachineObject()
{
}
