#include <Geofence.h>
#include "SelectDialog.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	Geofence g;
	if (argc == 2)
	{
		if (!g.Initialise(argv[1]))
		{
			QMessageBox::warning(0, "Geofence select", g.m_ErrorMessage, QMessageBox::Ok, QMessageBox::Ok);
			return 0;
		}
	}
	else
	{
		QString fileName = QFileDialog::getOpenFileName(0, "Open file", "", "KML Files (*.kml)");
		if (!g.Initialise(fileName))
		{
			QMessageBox::warning(0, "Geofence select", g.m_ErrorMessage, QMessageBox::Ok, QMessageBox::Ok);
			return 0;
		}
	}
		
	SelectDialog w(g);
	w.show();

	return a.exec();
}
