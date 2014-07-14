#include "gameobject.h"

GameObject::GameObject(QString myId, QString myParentId, QString myDescription, int myRomState, QObject *parent)
    : QObject(parent)
{
    setId(myId);
    setParentId(myParentId);
    setDescription(myDescription);
    setRomState(myRomState);
}

GameObject::~GameObject()
{
}
