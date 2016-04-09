#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include <QDialog>
#include <Geofence.h>

namespace Ui {
class Dialog;
}

class SelectDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit SelectDialog(Geofence& fence, QWidget *parent = 0);
	~SelectDialog();
	
private:
	Ui::Dialog *ui;
};

#endif // SELECTDIALOG_H
