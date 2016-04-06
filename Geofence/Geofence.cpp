#include <QDomDocument>
#include <QFile>
#include <QDebug>

#include "Geofence.h"

Geofence::Geofence(QObject *parent) : QObject(parent)
{
	QDomDocument doc("mydocument");
	QFile file("UAVCMedicalExpress2016SampleMission.kml");
	if (!file.open(QIODevice::ReadOnly))
		return;

	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}

	file.close();

	// print out the element names of all elements that are direct children
	// of the outermost element.
	QDomElement docElem = doc.documentElement();
	DumpElement(docElem, 0);
}

class Element
{
public :

protected :

};

class NamedElement : public Element
{
public :

protected :

};

class Location
{
public :
	Location();
	Location(const Location&);
	Location& operator=(const Location&);

	double m_Longitude;
	double m_Latitude;
	double m_Altitude;
};

class Folder : public NamedElement
{
public :

protected :

};

class Placemark : public NamedElement
{
public :

protected :

};

class LineString : public Element
{
public :

protected :

};

class LookAt : public Element
{
public :

protected :
	Location m_Location;
	double m_Heading;
	double m_Tilt;
	double m_Range;
};

class Polygon : public Element
{
public :

protected :

};

class Point : public Element
{
public :

protected :

};


void Geofence::DumpElement(QDomElement e, int indent)
{
	{
		QDebug d = qDebug().noquote().nospace();
		d << QString(indent, ' ') << e.tagName();

		QDomNode n = e.firstChild();
		while(!n.isNull())
		{
			QDomCharacterData c = n.toCharacterData();
			if (!c.isNull())
			{
				d << "=" << c.data();
			}

			n = n.nextSibling();
		}
	}

	QDomNode n = e.firstChild();
	while(!n.isNull())
	{
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull())
		{
			// the node really is an element.
			DumpElement(e, indent+1);
		}

		n = n.nextSibling();
	}
}

