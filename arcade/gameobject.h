#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QObject>

class GameObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(int romState READ romState WRITE setRomState NOTIFY romStateChanged)

public:
    explicit GameObject(QString, QString, int, QObject *parent = 0);
    virtual ~GameObject();

signals:
    void idChanged();
    void descriptionChanged();
    void romStateChanged();
    
public slots:
    // setters
    void setId(QString newId) { mId = newId; }
    void setDescription(QString newDescription) { mDescription = newDescription; }
    void setRomState(int newRomState) { mRomState = newRomState; }

    // getters
    QString id() { return mId; }
    QString description() { return mDescription; }
    int romState() { return mRomState; }

private:
    QString mId;
    QString mDescription;
    int mRomState;
};

#endif
