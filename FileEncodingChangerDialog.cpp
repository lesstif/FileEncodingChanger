#include <QtGui>
#include <QMessageBox>
#include <QObject>

#include "FileEncodingChangerDialog.h"

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

const char* CHARACTER_SET_TABLES[] = {
	"EUC-KR",
	"UTF-8",
	"UTF-16",
};

FileEncodingChangerDialog::FileEncodingChangerDialog(QWidget* parent) : QDialog(parent)
{
	setupUi(this);

//	layout()->setSizeConstraint(QLayout::SetFixedSize);

//	setColumnRange('A', 'Z');
	setCharacterEncodings();

	patternComboBox->addItem("*.cpp;*.h;*.java;*.xml;");
	directoryComboBox->addItem(currentDir.currentPath());

}

void FileEncodingChangerDialog::setCharacterEncodings()
{
	for (int i = 0; i < (sizeof(CHARACTER_SET_TABLES)/sizeof(*CHARACTER_SET_TABLES)); i++)
	{
		fromComboBox->addItem(CHARACTER_SET_TABLES[i]);
		toComboBox->addItem(CHARACTER_SET_TABLES[i]);
	}
}

void FileEncodingChangerDialog::browse()
{
 	QString directory = QFileDialog::getExistingDirectory(this,
                               tr("Find Files"), QDir::currentPath());

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
    }
}

void FileEncodingChangerDialog::convertFiles()
{
	filesTable->reset();

	QString fileName = patternComboBox->currentText();

	currentDir.setPath(directoryComboBox->currentText());

	updateComboBox(patternComboBox);
    //updateComboBox(textComboBox);
    updateComboBox(directoryComboBox);

	QStringList files;
	if (fileName.isEmpty())
		fileName = "*";

	QStringList filters = fileName.split(';',QString::KeepEmptyParts);

	files = currentDir.entryList(filters,
                       QDir::Files | QDir::NoSymLinks);

	QString text = "";

	files = findFiles(files, text);
	showFiles(files);
}

void FileEncodingChangerDialog::changeEncoding(QString from, QString to,QString path, bool makeBackup)
{
	QFile file(path);
	if (!file.exists())
		return;

	if (makeBackup)
	{
		QFile tmp;
		for (int i = 1; i < 100; i++)
		{
			QString bf = QString("%1.orig.%2").arg(path).arg(i);
			tmp.setFileName(bf);
			if (!tmp.exists())
			{
				tmp.copy(path, bf);
				break;
			}
		}
	}

	if (file.open(QIODevice::ReadOnly )) {
            QTextStream in(&file);
			
			in.setCodec("EUC-KR");
			while(!in.atEnd())
			{
				QString line = in.readLine();
				qDebug() << "toAscii:" << line.toAscii();
				qDebug() << "toLocal8Bit:" << line.toLocal8Bit();
				qDebug() << "toUtf8:" << line.toUtf8();
			}
			file.seek(0);

			QString qs = in.readAll();

			QByteArray qa = qs.toUtf8();
			
			file.close();

			if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
				QTextStream out(&file);
				out.setCodec("UTF-8");
				out << qa;
				file.close();
			}
     }
	
}

void FileEncodingChangerDialog::notImplYet()
{
		QMessageBox::warning(NULL, QObject::tr("Alert"),
			tr("Sorry Not Implemented!"),
			QMessageBox::Ok);

		autoDetectCheck->setChecked(false);
}

#if 0
void FileEncodingChangerDialog::setColumnRange(QChar first, QChar last)
{
	primaryColumnCombo->clear();
	secondaryColumnCombo->clear();
	tertiaryColumnCombo->clear();

	secondaryColumnCombo->addItem(tr("None"));
	tertiaryColumnCombo->addItem(tr("NOne"));
	primaryColumnCombo->setMinimumSize(
		secondaryColumnCombo->sizeHint());

	QChar ch = first;

	while (ch <= last) {
		primaryColumnCombo->addItem(QString(ch));
		secondaryColumnCombo->addItem(QString(ch));
		tertiaryColumnCombo->addItem(QString(ch));

		ch = ch.unicode() + 1;
	}
}
#endif

void FileEncodingChangerDialog::showFiles(const QStringList &files)
{

    for (int i = 0; i < files.size(); ++i) {
		changeEncoding(fromComboBox->currentText(), toComboBox->currentText(), currentDir.absoluteFilePath(files[i]), makeBackupCheck->isChecked());

        QFile file(currentDir.absoluteFilePath(files[i]));
        qint64 size = QFileInfo(file).size();

        QTableWidgetItem *fileNameItem = new QTableWidgetItem(files[i]);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(tr("%1 KB")
                                             .arg(int((size + 1023) / 1024)));
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        QTableWidgetItem *fromItem = new QTableWidgetItem("EUC-KR");
		QTableWidgetItem *toItem = new QTableWidgetItem("UTF-8");
		QTableWidgetItem *resultItem = new QTableWidgetItem("Fail");

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, fromItem);
        filesTable->setItem(row, 2, toItem);        
        filesTable->setItem(row, 3, resultItem);
        filesTable->setItem(row, 4, sizeItem);
    }
	filesTable->resizeColumnsToContents();

    filesFoundLabel->setText(tr("%1 file(s) converted").arg(files.size()) +
                             (" (Double click on a file to open it)"));

}

QStringList FileEncodingChangerDialog::findFiles(const QStringList &files, const QString &text)
{
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, files.size());
    progressDialog.setWindowTitle(tr("Find Files"));

    QStringList foundFiles;

    for (int i = 0; i < files.size(); ++i) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Searching file number %1 of %2...")
                                    .arg(i).arg(files.size()));
        qApp->processEvents();

        if (progressDialog.wasCanceled())
            break;

        QFile file(currentDir.absoluteFilePath(files[i]));

		if (file.exists())
			foundFiles << files[i];
#if 0
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            QTextStream in(&file);
            while (!in.atEnd()) {
                if (progressDialog.wasCanceled())
                    break;
                line = in.readLine();
                if (line.contains(text)) {
                    foundFiles << files[i];
                    break;
                }
            }
        }
#endif
    }
    return foundFiles;
}