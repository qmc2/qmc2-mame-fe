#include "gameobject.h"

GameObject::GameObject(QString myId, QString myDescription, int myRomState, QObject *parent)
    : QObject(parent)
{
    setId(myId);
    setDescription(myDescription);
    setRomState(myRomState);
}

GameObject::~GameObject()
{
}
