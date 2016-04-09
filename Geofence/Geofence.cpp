#include <QDomDocument>
#include <QFile>
#include <QDebug>

#include "Geofence.h"

Geofence::Geofence(QObject *parent)
:	QObject(parent),
	m_ParentElement(0)
{
}

bool Geofence::Initialise(const QString& pathName)
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
		m_ErrorMessage = "Unable to open the file: ";
		m_ErrorMessage += pathName;
		return false;
	}

	QString errorMessage;
	int line, column;
	if (!doc.setContent(&file, &errorMessage, &line, &column))
	{
		file.close();
		m_ErrorMessage = "Unable to parse the xml file: ";
		m_ErrorMessage += pathName;
		m_ErrorMessage += " error: ";
		m_ErrorMessage += errorMessage;
		m_ErrorMessage += " at line: ";
		m_ErrorMessage += QString::number(line);
		m_ErrorMessage += " column: ";
		m_ErrorMessage += QString::number(column);
		return false;
	}

	file.close();

	// print out the element names of all elements that are direct children
	// of the outermost element.
	QDomElement docElem = doc.documentElement();
	m_ParentElement = new Element(docElem);
	m_ParentElement->EnumerateElements(docElem, 0, elementTypes);

	QList<Element*> allPlacemarks;
	m_ParentElement->FindChildrenWithTagName("Placemark", allPlacemarks);

	foreach(Element* placemark, allPlacemarks)
	{
		placemark->Dump();
	}
	
	return true;
}

void Geofence::FillTree(QTreeWidget* treeWidget)
{
	if (m_ParentElement != 0)
	{
		m_ParentElement->FillTree(treeWidget, 0);
	}
}
