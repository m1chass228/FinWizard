#include "algorithmregistry.h"
#include "utils.h"
void algo1() {
    QString python_path = "/home/igor/ui_raz_ZP_qt_2/venv/bin/python";
    QString pythonScript = "/home/igor/raz_ZP_qt/ui_raz_ZP_qt_2/algorithms/marjino_kolomenskaya_svodni/marjino_kolomenskaya_svodni.py";

    std::string dir = "Марьино_Коломенская";
    fs::path dir_path = fs::path(get_desktop_path()) / base_folder / dir ;
    QString output_xlsx = QString::fromStdString((dir_on_desktop / "Марьино_Коломенская_сводный_отчет.xlsx").string()); // создаем

    QStringList file_names = {};                    // достаем пути к xlsx файлам и передаем в python
    for (auto entry : xlsx_in_path(dir_path)) {
        file_names << "\"" + QString::fromStdString(entry.string()) + "\"";
    }
    QString pyList = "[" + file_names.join(", ") + "]";

    qDebug() << "Run algo 1";
    QProcess process;

    process.start(python_path, QStringList() << pythonScript << pyList << output_xlsx);      // запускаем python
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput().trimmed();
    qDebug() << "Python out: " << output;

}
REGISTER_ALGORITHM(1, "Марьино Коломенская сводный отчет", "Марьино_Коломенская", algo1);

