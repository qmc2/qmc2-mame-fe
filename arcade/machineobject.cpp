#include "machineobject.h"

MachineObject::MachineObject(QString myId, QString myParentId, QString myDescription, int myRomState, QObject *parent)
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
