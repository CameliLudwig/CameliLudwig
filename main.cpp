#include <QApplication>
#include "UserAdminDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    UserAdminDialog dlg;
    dlg.show();

    return a.exec();
}
