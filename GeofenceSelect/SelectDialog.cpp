#include "SelectDialog.h"
#include "ui_Dialog.h"

SelectDialog::SelectDialog(Geofence& fence, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog)
{
	ui->setupUi(this);
	
	QTreeWidget *treeWidget = ui->Placemarks;

	treeWidget->setColumnCount(2);
	
	fence.FillTree(treeWidget);

	treeWidget->resizeColumnToContents(0);
	treeWidget->resizeColumnToContents(1);
}

SelectDialog::~SelectDialog()
{
	delete ui;
}

