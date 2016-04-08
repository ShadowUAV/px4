#ifndef GEOFENCE_H
#define GEOFENCE_H

#include <QObject>
#include <QMap>

class QDomNode;
class QDomElement;
class Element;

typedef Element* (*pfnCreateElement)(const QDomElement& e);
class Geofence : public QObject
{
	Q_OBJECT
public:
	explicit Geofence(const QString& pathName, QObject *parent = 0);

protected:
	void EnumerateElements(QDomElement e, int indent, Element* parent, QMap<QString, pfnCreateElement>& createMap);
	
signals:
	
public slots:

protected :
};

#endif // GEOFENCE_H
