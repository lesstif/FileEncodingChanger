#include <QtGui>
#include <QMessageBox>
#include <QObject>

#include "FileEncodingChangerDialog.h"

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

FileEncodingChangerDialog::FileEncodingChangerDialog(QWidget* parent) : QDialog(parent)
{
	setupUi(this);

//	layout()->setSizeConstraint(QLayout::SetFixedSize);

	patternComboBox->addItem("*.cpp;*.h;*.java;*.xml;");
	directoryComboBox->addItem(currentDir.currentPath());

	findCodecs(fromCodecs, "EUC-KR");
	setCodecList(fromComboBox, fromCodecs);

	findCodecs(toCodecs, "UTF-8");	
	setCodecList(toComboBox, toCodecs);
}

void FileEncodingChangerDialog::setCodecList(QComboBox* cb,const QCodecList& list)
{
    cb->clear();
    foreach (QTextCodec *codec, list)
        cb->addItem(codec->name(), codec->mibEnum());
	cb->setCurrentIndex(cb->count() - 1);
}

void FileEncodingChangerDialog::findCodecs(QCodecList& codecs, QString preferedEncoding)
{
    QMap<QString, QTextCodec *> codecMap;
    QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");
	
    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

		if (sortKey.compare(preferedEncoding, Qt::CaseInsensitive)) {
            rank = 1;
        } else if (sortKey.startsWith("UTF-8")) {
            rank = 2;
        } 
		else if (sortKey.startsWith("UTF-16")) {
            rank = 3;
        } else if (iso8859RegExp.exactMatch(sortKey)) {
            if (iso8859RegExp.cap(1).size() == 1)
                rank = 4;
            else
                rank = 5;
        } else {
            rank = 6;
        }
        sortKey.prepend(QChar('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    codecs = codecMap.values();
}

#if 0
void FileEncodingChangerDialog::setCharacterEncodings()
{
	for (int i = 0; i < (sizeof(CHARACTER_SET_TABLES)/sizeof(*CHARACTER_SET_TABLES)); i++)
	{
		fromComboBox->addItem(CHARACTER_SET_TABLES[i]);
		toComboBox->addItem(CHARACTER_SET_TABLES[i]);
	}
}
#endif 

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

	QDirIterator iterator(currentDir.absolutePath(),  filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

	while(iterator.hasNext())
	{
		iterator.next();
		qDebug() << iterator.filePath();

		files.append(iterator.filePath());
	}

	writeRestoreScript(files);
	showFiles(files);
	
}

void FileEncodingChangerDialog::writeRestoreScript(const QStringList& files)
{
	QDir root = QDir(directoryComboBox->currentText());
	QFile file(root.absolutePath() + "\\restore.bat");
	
	if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
		QTextStream out(&file);
	
		out << "@ECHO ON" << endl << endl;
		for(int i = 0; i < files.length(); i++) {
			QString str = QString("MOVE \"%1\" \"%2\"").arg(files[i] + ".orig.1").arg(files[i]);

			out << str << endl;
		}
		out << endl;

		file.close();
	}
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
			
			QString fromCodecName = fromComboBox->currentText();
			QString toCoecName = toComboBox->currentText();
			if (autoDetectCheck->isChecked()) 
			{
				//TODO
			}

			in.setCodec(fromCodecName.toAscii());

#ifdef _DEBUG
			while(!in.atEnd())
			{
				QString line = in.readLine();
				qDebug() << "toAscii:" << line.toAscii();
				qDebug() << "toLocal8Bit:" << line.toLocal8Bit();
				qDebug() << "toUtf8:" << line.toUtf8();
			}
			file.seek(0);
#endif			

			QString qs = in.readAll();

			file.close();

			if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
				QTextStream out(&file);
				out.setCodec(toCoecName.toAscii());
				out << qs;
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

		//TODO 
		QTableWidgetItem *fromItem = new QTableWidgetItem(fromComboBox->currentText());
		QTableWidgetItem *toItem = new QTableWidgetItem(toComboBox->currentText());
		QTableWidgetItem *resultItem = new QTableWidgetItem("Success");

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
