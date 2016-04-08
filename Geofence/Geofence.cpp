#include <QDomDocument>
#include <QFile>
#include <QDebug>

#include "Geofence.h"

QString ExtractCharacterData(const QDomElement& e, const QString& tagName)
{
	QString result;
	QDomElement nameElement = e.firstChildElement(tagName);
	if (!nameElement.isNull())
	{
		QDomNode n = nameElement.firstChild();
		while(!n.isNull())
		{
			QDomCharacterData c = n.toCharacterData();
			if (!c.isNull())
			{
				result += c.data();
			}

			n = n.nextSibling();
		}
	}

	return result.trimmed();
}

class Element
{
public :
	Element(const QDomElement& e)
	:	m_TagName(e.tagName())
	{}

	static Element* Create(const QDomElement& e) { return new Element(e); }
	void EnumerateElements(QDomElement e, int indent, QMap<QString, pfnCreateElement>& createMap);
	virtual void FindChildren();
	virtual void Dump()
	{
		foreach(Element* e, m_Children)
		{
			e->Dump();
		}
	}

	Element* FindChildWithTagName(const QString& tagName)
	{
		foreach(Element* e, m_Children)
		{
			if (e->m_TagName == tagName)
				return e;
		}

		return 0;
	}

protected :
	static Element* CreateNewElement(QDomElement e, QMap<QString, pfnCreateElement>& createMap);

protected :
	QList<Element*> m_Children;
	QString m_TagName;
};

class NamedElement : public Element
{
protected :
	NamedElement(const QDomElement& e)
	:	Element(e),
		m_Name(ExtractCharacterData(e, "name"))
	{
	}

protected :
	QString m_Name;
};

class Location
{
public :
	Location(const QDomElement& e)
	:	m_Longitude(ExtractCharacterData(e, "longitude").toDouble()),
		m_Latitude(ExtractCharacterData(e, "latitude").toDouble()),
		m_Altitude(ExtractCharacterData(e, "altitude").toDouble())
	{}

	Location(const QString& coordinate)
	{
		QStringList splitCoordinate = coordinate.split(",");

		int index = 0;
		foreach(QString value, splitCoordinate)
		{
			switch (index)
			{
			case 0 : m_Longitude = value.toDouble(); break;
			case 1 : m_Latitude = value.toDouble(); break;
			case 2 : m_Altitude = value.toDouble(); break;
			}

			index++;
		}
	}

	Location()
	:	m_Longitude(0.0),
		m_Latitude(0.0),
		m_Altitude(0.0)
	{
	}

	Location(const Location& copy)
	:	m_Longitude(copy.m_Longitude),
		m_Latitude(copy.m_Latitude),
		m_Altitude(copy.m_Altitude)
	{
	}

	Location& operator=(const Location&);

	double m_Longitude;
	double m_Latitude;
	double m_Altitude;
};

QDebug& operator<<(QDebug& d, const Location& l)
{
	d << l.m_Longitude << ", " << l.m_Latitude << ", " << l.m_Altitude;
	return d;
}

class Folder : public NamedElement
{
public :
	Folder(const QDomElement& e)
	:	NamedElement(e)
	{}

	static Element* Create(const QDomElement& e) { return new Folder(e); }

protected :
};

class Coordinates
{
public :
	Coordinates(const QDomElement& e)
	{
		QString coordinateString(ExtractCharacterData(e, "coordinates"));
		QStringList splitCoordinates = coordinateString.split(QRegExp("\\s"));

		foreach (QString coordinate, splitCoordinates)
		{
			Location l(coordinate);
			m_Coordinates.append(l);
		}
	}

public :
	QList<Location> m_Coordinates;
};

QDebug& operator<<(QDebug& d, const Coordinates& c)
{
	foreach (const Location& l, c.m_Coordinates)
	{
		d << "(" << l << "), ";
	}

	return d;
}

class LineString : public Element
{
public :
	LineString(const QDomElement& e)
	:	Element(e),
		m_Coordinates(e),
		m_AltitudeMode(ExtractCharacterData(e, "altitudeMode"))
	{}

	static Element* Create(const QDomElement& e) { return new LineString(e); }

public :
	Coordinates m_Coordinates;
	QString m_AltitudeMode;
};

class LinearRing : public Element
{
public :
	LinearRing(const QDomElement& e)
	:	Element(e),
		m_Coordinates(e)
	{}

	static Element* Create(const QDomElement& e) { return new LinearRing(e); }

public :
	Coordinates m_Coordinates;
};

class outerBoundaryIs : public Element
{
public :
	outerBoundaryIs(const QDomElement& e)
	:	Element(e),
		m_LinearRing(0)
	{}

	static Element* Create(const QDomElement& e) { return new outerBoundaryIs(e); }
	virtual void FindChildren()
	{
		m_LinearRing = dynamic_cast<LinearRing*>(FindChildWithTagName("LinearRing"));
	}

public :
	LinearRing* m_LinearRing;
};

class LookAt : public Element
{
public :
	LookAt(const QDomElement& e)
	:	Element(e),
		m_Location(e),
		m_Heading(ExtractCharacterData(e, "heading").toInt()),
		m_Tilt(ExtractCharacterData(e, "tilt").toInt()),
		m_Range(ExtractCharacterData(e, "range").toDouble())
	{}

	static Element* Create(const QDomElement& e) { return new LookAt(e); }

public :
	Location m_Location;
	int m_Heading;
	int m_Tilt;
	double m_Range;
};

class Polygon : public Element
{
public :
	Polygon(const QDomElement& e)
	:	Element(e),
		m_outerBoundaryIs(0)
	{}

	static Element* Create(const QDomElement& e) { return new Polygon(e); }
	virtual void FindChildren()
	{
		m_outerBoundaryIs = dynamic_cast<outerBoundaryIs*>(FindChildWithTagName("outerBoundaryIs"));
	}

public :
	outerBoundaryIs* m_outerBoundaryIs;
};

class Point : public Element
{
public :
	Point(const QDomElement& e)
	:	Element(e),
		m_Coordinates(e)
	{}

	static Element* Create(const QDomElement& e) { return new Point(e); }

public :
	Coordinates m_Coordinates;
};

class Placemark : public NamedElement
{
public :
	Placemark(const QDomElement& e)
	:	NamedElement(e),
		m_LookAt(0)
	{}

	static Element* Create(const QDomElement& e) { return new Placemark(e); }
	virtual void FindChildren()
	{
		m_LookAt = dynamic_cast<LookAt*>(FindChildWithTagName("LookAt"));
		m_Polygon = dynamic_cast<Polygon*>(FindChildWithTagName("Polygon"));
		m_Point = dynamic_cast<Point*>(FindChildWithTagName("Point"));
		m_LineString = dynamic_cast<LineString*>(FindChildWithTagName("LineString"));
	}

	virtual void Dump()
	{
		QDebug d = qDebug().noquote().nospace();

		d << "Placemark: " << m_Name << ": " << endl;
		if (m_LookAt != 0)
		{
			d << "LookAt: (" << m_LookAt->m_Location << ")" << endl;
		}

		if (m_Polygon != 0)
		{
			d << "Polygon outerBoundaryIs LinearRing coordinates: " << m_Polygon->m_outerBoundaryIs->m_LinearRing->m_Coordinates << endl;
		}

		if (m_Point != 0)
		{
			d << "Point coordinates: " << m_Point->m_Coordinates << endl;
		}

		if (m_LineString != 0)
		{
			d << "LineString coordinates: " << m_LineString->m_Coordinates << endl;
		}
	}

protected :
	LookAt* m_LookAt;
	Polygon* m_Polygon;
	Point* m_Point;
	LineString* m_LineString;
};

Element* Element::CreateNewElement(QDomElement e, QMap<QString, pfnCreateElement>& createMap)
{
	QMap<QString, pfnCreateElement>::iterator i = createMap.find(e.tagName());
	if (i != createMap.end())
	{
		pfnCreateElement fnCreateElement = i.value();
		if (fnCreateElement != 0)
		{
			Element* element = fnCreateElement(e);
			if (element != 0)
			{
				return element;
			}
		}
	}

	return new Element(e);
}

void Element::EnumerateElements(QDomElement e, int indent, QMap<QString, pfnCreateElement>& createMap)
{
	/* {
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
	} */

	QDomNode n = e.firstChild();
	while(!n.isNull())
	{
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull())
		{
			// the node really is an element.
			Element* element = CreateNewElement(e, createMap);
			if (element != 0)
			{
				m_Children.append(element);
				element->EnumerateElements(e, indent + 1, createMap);
			}
		}

		n = n.nextSibling();
	}

	FindChildren();
}

void Element::FindChildren()
{
}

Geofence::Geofence(const QString& pathName, QObject *parent) : QObject(parent)
{
	// Map from an element tag name to an element create function
	QMap<QString, pfnCreateElement> elementTypes;

	elementTypes["Folder"] = &Folder::Create;
	elementTypes["Placemark"] = &Placemark::Create;
	elementTypes["LineString"] = &LineString::Create;
	elementTypes["LinearRing"] = &LinearRing::Create;
	elementTypes["outerBoundaryIs"] = &outerBoundaryIs::Create;
	elementTypes["LookAt"] = &LookAt::Create;
	elementTypes["Placemark"] = &Placemark::Create;
	elementTypes["Polygon"] = &Polygon::Create;
	elementTypes["Point"] = &Point::Create;

	QDomDocument doc("mydocument");
	QFile file(pathName);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Unable to open the file: " << pathName;
		return;
	}

	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}

	file.close();

	// print out the element names of all elements that are direct children
	// of the outermost element.
	QDomElement docElem = doc.documentElement();
	Element* parentElement = new Element(docElem);
	parentElement->EnumerateElements(docElem, 0, elementTypes);
	parentElement->Dump();
}
