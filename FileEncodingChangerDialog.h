#ifndef _FileEncodingChangeDialog_H_
#define _FileEncodingChangeDialog_H_

#include <QDir>

#include "ui_FileEncodingChangerDialog.h"

class QString;
class QStringList;

class FileEncodingChangerDialog : public QDialog, public Ui::FileEncodingChangerDialog
{
	Q_OBJECT

public:
	FileEncodingChangerDialog(QWidget* parent=0);
	
private slots:
	void browse();
	void find();
	void showFiles(const QStringList& files);	

private:
	QStringList findFiles	(const QStringList &files, const QString &text);

	QDir currentDir;
};

#endif