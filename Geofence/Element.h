#ifndef ELEMENT_H
#define ELEMENT_H

#include <QDomDocument>
#include <QFile>
#include <QDebug>

class Element;
class QTreeWidget;
class QTreeWidgetItem;
typedef Element* (*pfnCreateElement)(const QDomElement& e);

class Element
{
public :
	Element(const QDomElement& e);

	static Element* Create(const QDomElement& e);
	void EnumerateElements(QDomElement e, int indent, QMap<QString, pfnCreateElement>& createMap);
	virtual void Initialise();
	virtual void Dump();
	virtual void FillTree(QTreeWidget* treeWidget, QTreeWidgetItem* parentItem);

	Element* FindChildWithTagName(const QString& tagName);
	void FindChildrenWithTagName(const QString& tagName, QList<Element*>& results);

protected :
	static Element* CreateNewElement(QDomElement e, QMap<QString, pfnCreateElement>& createMap);

protected :
	QList<Element*> m_Children;
	QString m_TagName;
};

class NamedElement : public Element
{
protected :
	NamedElement(const QDomElement& e);

protected :
	QString m_Name;
};

class Location
{
public :
	Location(const QDomElement& e);
	Location(const QString& coordinate);

	Location();

	Location(const Location& copy);

	Location& operator=(const Location&);
	
	QString toString() const;

	double m_Longitude;
	double m_Latitude;
	double m_Altitude;
};

QDebug& operator<<(QDebug& d, const Location& l);

class Folder : public NamedElement
{
public :
	Folder(const QDomElement& e);

	static Element* Create(const QDomElement& e);
};

class Coordinates
{
public :
	Coordinates(const QDomElement& e);
	QString toString() const;

public :
	QList<Location> m_Coordinates;
};

QDebug& operator<<(QDebug& d, const Coordinates& c);

class LineString : public Element
{
public :
	LineString(const QDomElement& e);

	static Element* Create(const QDomElement& e);

public :
	Coordinates m_Coordinates;
	QString m_AltitudeMode;
};

class LinearRing : public Element
{
public :
	LinearRing(const QDomElement& e);

	static Element* Create(const QDomElement& e);

public :
	Coordinates m_Coordinates;
};

class outerBoundaryIs : public Element
{
public :
	outerBoundaryIs(const QDomElement& e);

	static Element* Create(const QDomElement& e);
	virtual void Initialise();

public :
	LinearRing* m_LinearRing;
};

class LookAt : public Element
{
public :
	LookAt(const QDomElement& e);

	static Element* Create(const QDomElement& e);

public :
	Location m_Location;
	int m_Heading;
	int m_Tilt;
	double m_Range;
};

class Polygon : public Element
{
public :
	Polygon(const QDomElement& e);

	static Element* Create(const QDomElement& e);
	virtual void Initialise();

public :
	outerBoundaryIs* m_outerBoundaryIs;
};

class Point : public Element
{
public :
	Point(const QDomElement& e);

	static Element* Create(const QDomElement& e);

public :
	Coordinates m_Coordinates;
};

class Placemark : public NamedElement
{
public :
	Placemark(const QDomElement& e);

	static Element* Create(const QDomElement& e);
	virtual void Initialise();

	virtual void Dump();
	virtual void FillTree(QTreeWidget* treeWidget, QTreeWidgetItem*);

protected :
	LookAt* m_LookAt;
	Polygon* m_Polygon;
	Point* m_Point;
	LineString* m_LineString;
};

QString ExtractCharacterData(const QDomElement& e, const QString& tagName);

#endif // ELEMENT_H
