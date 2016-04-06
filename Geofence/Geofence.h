#ifndef GEOFENCE_H
#define GEOFENCE_H

#include <QObject>

class QDomElement;
class Geofence : public QObject
{
	Q_OBJECT
public:
	explicit Geofence(QObject *parent = 0);

protected:
	void DumpElement(QDomElement e, int indent);
	
signals:
	
public slots:
};

#endif // GEOFENCE_H
