#ifndef GEOFENCE_H
#define GEOFENCE_H

#include <QObject>
#include <QMap>

#include "Element.h"

class QDomNode;
class QDomElement;
class Element;
class QTreeWidget;

class Geofence : public QObject
{
	Q_OBJECT
public:
	explicit Geofence(QObject *parent = 0);
	bool Initialise(const QString& pathName);
	
	void FillTree(QTreeWidget *treeWidget);

protected:
	void EnumerateElements(QDomElement e, int indent, Element* parent, QMap<QString, pfnCreateElement>& createMap);
	
signals:
	
public slots:

public :
	QString m_ErrorMessage;

protected :
	Element* m_ParentElement;
};

#endif // GEOFENCE_H
