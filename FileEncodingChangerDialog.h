#ifndef _FileEncodingChangeDialog_H_
#define _FileEncodingChangeDialog_H_

#include <QDir>

#include "ui_FileEncodingChangerDialog.h"

class QString;
class QStringList;

typedef QList<QTextCodec *> QCodecList;

struct convertRes {
	QString file;
	QString fromCodec;
	QString toCodec;
	QString result;
};

typedef QList<convertRes> ConvertResult;

class FileEncodingChangerDialog : public QDialog, public Ui::FileEncodingChangerDialog
{
	Q_OBJECT

public:
	FileEncodingChangerDialog(QWidget* parent=0);
	
private slots:
	void browse();
	void showFiles(const QStringList& files);	

	void convertFiles();
	void notImplYet();
private:
	void writeRestoreScript(const QStringList& files);
	void findCodecs(QCodecList& codecs, QString preferedEncoding);
	void setCodecList(QComboBox* cb, const QCodecList &list);

	QStringList findFiles	(const QStringList &files, const QString &text);
	void changeEncoding(QString from, QString to,QString path, bool makeBackup);
	void setCharacterEncodings();
	
	QDir currentDir;
	QList<QTextCodec *> fromCodecs;
	QList<QTextCodec *> toCodecs;
};

#endif