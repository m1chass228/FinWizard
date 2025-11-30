#include "algorithmregistry.h"
#include "utils.h"
void algo1() {
    QXlsx::Format center;
    QXlsx::Format right;

    center.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    right.setHorizontalAlignment(QXlsx::Format::AlignRight);

    QXlsx::Format fmt_header = center;
    fmt_header.setBorderStyle(QXlsx::Format::BorderMedium); // жирная рамка со всех сторон
    fmt_header.setFontBold(true);

    QXlsx::Format style = right;
    style.setFillPattern(QXlsx::Format::PatternSolid);
    style.setPatternForegroundColor(QColor("#8cc2f5"));

    std::string dir = "Марьино_Коломенская";
    fs::path dir_path = fs::path(get_desktop_path()) / base_folder / dir ;

    QXlsx::Document output_xlsx(QString::fromStdString((dir_on_desktop / "Марьино_Коломенская_сводный_отчет.xlsx").string())); // создаем

    output_xlsx.write("A1", "Дата", fmt_header);
    output_xlsx.write("B1", "Сотрудники", fmt_header);
    output_xlsx.write("C1", "Всего", fmt_header);
    output_xlsx.write("D1", "Вет.услуги", fmt_header);
    output_xlsx.write("E1", "Дерматолог", fmt_header);
    output_xlsx.write("F1", "Ратолог", fmt_header);
    output_xlsx.write("G1", "Стоматолог", fmt_header);
    output_xlsx.write("H1", "Кардиолог", fmt_header);
    output_xlsx.write("I1", "Орнитолог", fmt_header);
    output_xlsx.write("J1", "Ортопед", fmt_header);
    output_xlsx.write("K1", "Сумма з/п УС (пока нету)", fmt_header);
    output_xlsx.write("L1", "Анализы", fmt_header);
    output_xlsx.write("M1", "Ритуал", fmt_header);
    output_xlsx.write("N1", "Аптека", fmt_header);
    output_xlsx.write("O1", "Кол-во счетов", fmt_header);
    output_xlsx.write("P1", "Кол-во чеков", fmt_header);

    output_xlsx.setColumnWidth(2, 35);  // сотрудники
    output_xlsx.setColumnWidth(3, 10);  // всего
    output_xlsx.setColumnWidth(4, 15);  // вет.услуги
    output_xlsx.setColumnWidth(5, 15);  // дерматолог
    output_xlsx.setColumnWidth(6, 15);  // ратолог
    output_xlsx.setColumnWidth(7, 15);  // стоматолог
    output_xlsx.setColumnWidth(8, 15);  // кардиолог
    output_xlsx.setColumnWidth(9, 15);  // орнитолог
    output_xlsx.setColumnWidth(10, 15); // ортопед
    output_xlsx.setColumnWidth(11, 15);  // Сумма з/п УС (пока нету)
    output_xlsx.setColumnWidth(12, 15); // анализы
    output_xlsx.setColumnWidth(13, 15); // ритуал
    output_xlsx.setColumnWidth(14, 15); // аптека
    output_xlsx.setColumnWidth(15, 15); // кол-во счетов
    output_xlsx.setColumnWidth(16, 15); // кол-во чеков

    int rows_indent = 2;
    for (auto entry : xlsx_in_path(dir_path)) {

        qDebug() << QString::fromStdString(entry.string());
        QXlsx::Document input_xlsx(QString::fromStdString(entry.string())); // открываем

        // Достаем Дату
        QVariant value = input_xlsx.read("E1");
        if (!value.isNull()) {
            QString d = value.toString();
            if (d.endsWith('.')) d.chop(1);
            output_xlsx.write(rows_indent, 1, d, right);
        }
        // Достаем Сотрудники
        QStringList allnames;
        for (int row=6; row <= 8; row++) {
            for (int col=1; col <=4; col++) {
                value = input_xlsx.read(row, col);
                if (!value.isNull()) {
                    QString text = value.toString().trimmed();
                    if (!text.isEmpty()) {
                        allnames << text;
                    }
                }
            }
        }
        QString combined = allnames.join(", ");
        output_xlsx.write(rows_indent, 2, combined, right);
        // Достаем Всего
        value = input_xlsx.read("D10");
        if (!value.isNull()) output_xlsx.write(rows_indent, 3, value, right);
        // Достаем Вет.Услуги
        value = input_xlsx.read("A11");
        if (!value.isNull()) output_xlsx.write(rows_indent, 4, value, right);
        // Достаем Дерматолог
        value = input_xlsx.read("D18");
        if (!value.isNull()) output_xlsx.write(rows_indent, 5, value, right);
        // Достаем Ратолог
        value = input_xlsx.read("D16");
        if (!value.isNull()) output_xlsx.write(rows_indent, 6, value, right);
        // Достаем Стоматолог
        value = input_xlsx.read("D13");
        if (!value.isNull()) output_xlsx.write(rows_indent, 7, value, right);
        // Достаем Кардиолог
        value = input_xlsx.read("D15");
        if (!value.isNull()) output_xlsx.write(rows_indent, 8, value, right);
        // Достаем Орнитолог
        value = input_xlsx.read("D19");
        if (!value.isNull()) output_xlsx.write(rows_indent, 9, value, right);
        // Достаем Ортопед
        value = input_xlsx.read("D17");
        if (!value.isNull()) output_xlsx.write(rows_indent, 10, value, right);
        // Сумма з/п УС

        // Анализы
        double sum_value = 0.0;
        value = input_xlsx.read("G19");
        if (!value.isNull() && value.canConvert<double>()) {
            sum_value += value.toDouble();
        }
        value = input_xlsx.read("G20");
        if (!value.isNull() && value.canConvert<double>()) {
            sum_value += value.toDouble();
        }
        output_xlsx.write(rows_indent, 12, sum_value, right);
        // Достаем Ритуал
        value = input_xlsx.read("G21");
        if (!value.isNull()) output_xlsx.write(rows_indent, 13, value, right);
        // Достаем Аптека
        value = input_xlsx.read("C10");
        if (!value.isNull()) output_xlsx.write(rows_indent, 14, value, right);
        // Достаем Кол-во счетов
        value = input_xlsx.read("H59");
        if (!value.isNull()) output_xlsx.write(rows_indent, 15, value, right);
        // Достаем Кол-во чеков
        value = input_xlsx.read("J59");
        if (!value.isNull()) output_xlsx.write(rows_indent, 16, value, right);

        output_xlsx.save();
        rows_indent++;
    }
    qDebug() << "Run algo 1";
    QXlsx::Document input_xlsx;

}
REGISTER_ALGORITHM(1, "Марьино Коломенская сводный отчет", "Марьино_Коломенская", algo1);

