#include "Element.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextStream>

Element::Element(const QDomElement& e)
	:	m_TagName(e.tagName())
{}

Element*Element::Create(const QDomElement& e) { return new Element(e); }

void Element::Initialise()
{
}

void Element::Dump()
{
	foreach(Element* e, m_Children)
	{
		e->Dump();
	}
}

void Element::FillTree(QTreeWidget* treeWidget, QTreeWidgetItem* parentItem)
{	
	foreach(Element* e, m_Children)
	{
		e->FillTree(treeWidget, parentItem);
	}
}

Element*Element::FindChildWithTagName(const QString& tagName)
{
	foreach(Element* e, m_Children)
	{
		if (e->m_TagName == tagName)
			return e;
	}
	
	return 0;
}

void Element::FindChildrenWithTagName(const QString& tagName, QList<Element*>& results)
{
	foreach(Element* e, m_Children)
	{
		if (e->m_TagName == tagName)
		{
			results.append(e);
		}
		else
		{
			e->FindChildrenWithTagName(tagName, results);
		}
	}
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
	
	Initialise();
}

Element*Element::CreateNewElement(QDomElement e, QMap<QString, pfnCreateElement>& createMap)
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

NamedElement::NamedElement(const QDomElement& e)
	:	Element(e),
	  m_Name(ExtractCharacterData(e, "name"))
{
}

Location::Location(const QDomElement& e)
:	m_Longitude(ExtractCharacterData(e, "longitude").toDouble()),
	  m_Latitude(ExtractCharacterData(e, "latitude").toDouble()),
	  m_Altitude(ExtractCharacterData(e, "altitude").toDouble())
{}

Location::Location(const QString& coordinate)
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

Location::Location()
	:	m_Longitude(0.0),
	  m_Latitude(0.0),
	  m_Altitude(0.0)
{
}

Location::Location(const Location& copy)
	:	m_Longitude(copy.m_Longitude),
	  m_Latitude(copy.m_Latitude),
	  m_Altitude(copy.m_Altitude)
{
}

QString Location::toString() const
{
	QString result;
	QTextStream s(&result);
	s << m_Longitude << ", " << m_Latitude << ", " << m_Altitude;
	return result;	
}

QDebug&operator<<(QDebug& d, const Location& l)
{
	d << l.m_Longitude << ", " << l.m_Latitude << ", " << l.m_Altitude;
	return d;
}

Folder::Folder(const QDomElement& e)
	:	NamedElement(e)
{}

Element*Folder::Create(const QDomElement& e) { return new Folder(e); }

Coordinates::Coordinates(const QDomElement& e)
{
	QString coordinateString(ExtractCharacterData(e, "coordinates"));
	QStringList splitCoordinates = coordinateString.split(QRegExp("\\s"));
	
	foreach (QString coordinate, splitCoordinates)
	{
		Location l(coordinate);
		m_Coordinates.append(l);
	}
}

QString Coordinates::toString() const
{
	QString result;
	QTextStream s(&result);

	foreach (const Location& l, m_Coordinates)
	{
		s << "(" << l.toString() << "), ";
	}

	return result;	
}

QDebug& operator<<(QDebug& d, const Coordinates& c)
{
	foreach (const Location& l, c.m_Coordinates)
	{
		d << "(" << l << "), ";
	}
	
	return d;
}

LineString::LineString(const QDomElement& e)
	:	Element(e),
	  m_Coordinates(e),
	  m_AltitudeMode(ExtractCharacterData(e, "altitudeMode"))
{}

Element*LineString::Create(const QDomElement& e) { return new LineString(e); }

LinearRing::LinearRing(const QDomElement& e)
	:	Element(e),
	  m_Coordinates(e)
{}

Element*LinearRing::Create(const QDomElement& e) { return new LinearRing(e); }

outerBoundaryIs::outerBoundaryIs(const QDomElement& e)
	:	Element(e),
	  m_LinearRing(0)
{}

Element*outerBoundaryIs::Create(const QDomElement& e) { return new outerBoundaryIs(e); }

void outerBoundaryIs::Initialise()
{
	m_LinearRing = dynamic_cast<LinearRing*>(FindChildWithTagName("LinearRing"));
}

LookAt::LookAt(const QDomElement& e)
	:	Element(e),
	  m_Location(e),
	  m_Heading(ExtractCharacterData(e, "heading").toInt()),
	  m_Tilt(ExtractCharacterData(e, "tilt").toInt()),
	  m_Range(ExtractCharacterData(e, "range").toDouble())
{}

Element*LookAt::Create(const QDomElement& e) { return new LookAt(e); }

Polygon::Polygon(const QDomElement& e)
	:	Element(e),
	  m_outerBoundaryIs(0)
{}

Element*Polygon::Create(const QDomElement& e) { return new Polygon(e); }

void Polygon::Initialise()
{
	m_outerBoundaryIs = dynamic_cast<outerBoundaryIs*>(FindChildWithTagName("outerBoundaryIs"));
}

Point::Point(const QDomElement& e)
	:	Element(e),
	  m_Coordinates(e)
{}

Element*Point::Create(const QDomElement& e) { return new Point(e); }

Placemark::Placemark(const QDomElement& e)
	:	NamedElement(e),
	  m_LookAt(0)
{}

Element*Placemark::Create(const QDomElement& e) { return new Placemark(e); }

void Placemark::Initialise()
{
	m_LookAt = dynamic_cast<LookAt*>(FindChildWithTagName("LookAt"));
	m_Polygon = dynamic_cast<Polygon*>(FindChildWithTagName("Polygon"));
	m_Point = dynamic_cast<Point*>(FindChildWithTagName("Point"));
	m_LineString = dynamic_cast<LineString*>(FindChildWithTagName("LineString"));
}

void Placemark::Dump()
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

void Placemark::FillTree(QTreeWidget* treeWidget, QTreeWidgetItem*)
{
	QTreeWidgetItem* parent = new QTreeWidgetItem(treeWidget, QStringList() << m_Name);
	
	if (m_LookAt != 0)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList() << "LookAt: " << m_LookAt->m_Location.toString());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(1, Qt::Unchecked);
	}
	
	if (m_Polygon != 0)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList() << "Polygon: " << m_Polygon->m_outerBoundaryIs->m_LinearRing->m_Coordinates.toString());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(1, Qt::Unchecked);
	}
	
	if (m_Point != 0)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList() << "Point: " << m_Point->m_Coordinates.toString());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(1, Qt::Unchecked);
	}
	
	if (m_LineString != 0)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList() << "LineString: " << m_LineString->m_Coordinates.toString());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(1, Qt::Unchecked);
	}
}
