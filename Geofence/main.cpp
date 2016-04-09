#include "Geofence.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if (argc == 2)
	{
		Geofence g;
		g.Initialise(argv[1]);
	}
	else
	{
		qDebug() << "Usage: " << argv[0] << " [kml filename]";
		return -1;
	}

	return 0;
}
