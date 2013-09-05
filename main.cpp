#include <QApplication>

#include "FileEncodingChangerDialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QDialog* dialog = new FileEncodingChangerDialog;
    dialog->show();
    
    return app.exec();
}
