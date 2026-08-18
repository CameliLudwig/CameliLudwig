#include "pdfreportgenerator.h"
#include <QPrinter>
#include <QTextDocument>
#include <QDate>
#include <QFileInfo>
#include <QDebug>

PDFReportGenerator::PDFReportGenerator()
{
    initializeDefaults();
}

PDFReportGenerator::~PDFReportGenerator()
{
}

void PDFReportGenerator::initializeDefaults()
{
    m_deviceName = "";
    m_partNumber = "";
    m_testDate = QDate::currentDate().toString("yyyy-MM-dd");
    m_inspector = "";
    m_companyInfo = QString::fromUtf8("中国人民解放军第二七六五厂");

    m_tableHeaders << QString::fromUtf8("序号") << QString::fromUtf8("检测项目")
                   << QString::fromUtf8("技术要求") << QString::fromUtf8("检测结果")
                   << QString::fromUtf8("单位") << QString::fromUtf8("备注");
    m_columnWidths << 50 << 150 << 120 << 100 << 60 << 120;
    m_borderWidth = 1;
    m_cellPadding = 5;
    m_headerBgColor = "#E6F3FF";
    m_alternateRowColor = "#F9F9F9";

    m_marginLeft = 15;
    m_marginTop = 15;
    m_marginRight = 15;
    m_marginBottom = 15;
    m_landscape = false;

    m_reportTitle = QString::fromUtf8("综合告警检测设备  检测报告");
}

void PDFReportGenerator::setDeviceName(const QString &name) { m_deviceName = name; }
void PDFReportGenerator::setPartNumber(const QString &partNumber) { m_partNumber = partNumber; }
void PDFReportGenerator::setTestDate(const QString &date) { m_testDate = date; }
void PDFReportGenerator::setInspector(const QString &inspector) { m_inspector = inspector; }
void PDFReportGenerator::setCompanyInfo(const QString &company) { m_companyInfo = company; }
void PDFReportGenerator::setReportTitle(const QString &title) { m_reportTitle = title; }

void PDFReportGenerator::addReportRow(const ReportRow &row) { m_reportRows.append(row); }
void PDFReportGenerator::clearReportRows() { m_reportRows.clear(); }

void PDFReportGenerator::setTableHeaders(const QStringList &headers) { m_tableHeaders = headers; }
void PDFReportGenerator::setTableColumnWidths(const QList<int> &widths) { m_columnWidths = widths; }
void PDFReportGenerator::addTestItem(const TestItem &item) { m_testItems.append(item); }
void PDFReportGenerator::removeTestItem(int index) { if (index >= 0 && index < m_testItems.size()) m_testItems.removeAt(index); }
void PDFReportGenerator::updateTestItem(int index, const TestItem &item) { if (index >= 0 && index < m_testItems.size()) m_testItems[index] = item; }
TestItem PDFReportGenerator::getTestItem(int index) const { if (index >= 0 && index < m_testItems.size()) return m_testItems[index]; return TestItem(); }
QList<TestItem> PDFReportGenerator::getAllTestItems() const { return m_testItems; }
void PDFReportGenerator::clearAllTestItems() { m_testItems.clear(); }
void PDFReportGenerator::setTableBorderWidth(int width) { m_borderWidth = width; }
void PDFReportGenerator::setTableCellPadding(int padding) { m_cellPadding = padding; }
void PDFReportGenerator::setHeaderBackgroundColor(const QString &color) { m_headerBgColor = color; }
void PDFReportGenerator::setAlternateRowColor(const QString &color) { m_alternateRowColor = color; }
void PDFReportGenerator::setPageMargins(int left, int top, int right, int bottom) { m_marginLeft = left; m_marginTop = top; m_marginRight = right; m_marginBottom = bottom; }
void PDFReportGenerator::setPageOrientation(bool landscape) { m_landscape = landscape; }

// ===== 原始表格PDF（保留兼容）=====
bool PDFReportGenerator::generatePDF(const QString &fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPrinter::A4);
    if (m_landscape) printer.setOrientation(QPrinter::Landscape);
    else printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom, QPrinter::Millimeter);
    QTextDocument document;
    document.setHtml(generateHTMLContent());
    document.print(&printer);
    return true;
}

QString PDFReportGenerator::generateHTMLContent()
{
    QString html;
    html += "<!DOCTYPE html><html><head><meta charset='UTF-8'><style>";
    html += "body { font-family: SimSun, serif; margin: 0; padding: 20px; }";
    html += ".header { text-align: center; margin-bottom: 30px; }";
    html += ".company { font-size: 18px; font-weight: bold; margin-bottom: 10px; }";
    html += ".title { font-size: 16px; font-weight: bold; margin-bottom: 20px; }";
    html += ".info { margin-bottom: 20px; }";
    html += ".info-item { display: inline-block; margin-right: 30px; margin-bottom: 10px; }";
    html += "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; }";
    html += "th, td { border: " + QString::number(m_borderWidth) + "px solid #000; padding: "
            + QString::number(m_cellPadding) + "px; text-align: center; }";
    html += "th { background-color: " + m_headerBgColor + "; font-weight: bold; }";
    html += ".alt-row { background-color: " + m_alternateRowColor + "; }";
    html += ".signature { margin-top: 30px; }";
    html += ".signature-item { display: inline-block; margin-right: 50px; }";
    html += "</style></head><body>";
    html += "<div class='header'><div class='company'>" + m_companyInfo + "</div>";
    html += "<div class='title'>检测报告</div></div>";
    html += "<div class='info'>";
    html += "<div class='info-item'>设备名称：" + m_deviceName + "</div>";
    html += "<div class='info-item'>零件号：" + m_partNumber + "</div>";
    html += "<div class='info-item'>检测日期：" + m_testDate + "</div>";
    html += "<div class='info-item'>检测员：" + m_inspector + "</div></div>";
    html += "<table><tr>";
    for (const QString &header : m_tableHeaders) html += "<th>" + header + "</th>";
    html += "</tr>";
    for (int i = 0; i < m_testItems.size(); ++i) {
        const TestItem &item = m_testItems[i];
        QString rc = (i % 2 == 1) ? " class='alt-row'" : "";
        html += "<tr" + rc + "><td>" + QString::number(i + 1) + "</td><td>" + item.itemName
                + "</td><td>" + item.specification + "</td><td>" + item.testResult
                + "</td><td>" + item.unit + "</td><td>" + item.remarks + "</td></tr>";
    }
    html += "</table>";
    html += "<div class='signature'>";
    html += "<div class='signature-item'>检测员签名：_______________</div>";
    html += "<div class='signature-item'>审核员签名：_______________</div>";
    html += "<div class='signature-item'>日期：_______________</div></div>";
    html += "</body></html>";
    return html;
}

// ===================================================================
//  正式检测报告 — 严格按照图片格式
// ===================================================================
QString PDFReportGenerator::generateFormalHTML()
{
    QString html;
    html += "<!DOCTYPE html><html><head><meta charset='UTF-8'><style>";
    html += "*{margin:0;padding:0;box-sizing:border-box;}";
    html += "body{font-family:SimSun,'Microsoft YaHei',serif;font-size:13pt;line-height:1.6;color:#000;}";

    // 外层边框
    html += ".page-border{border:2.5px solid #000;padding:8mm 10mm;min-height:92%;}";
    // 注意：QTextDocument 对中文字体 bold 会产生重影，一律不用 bold
    html += ".org-name{text-align:center;font-size:18pt;margin-bottom:2mm;}";
    html += ".report-title{text-align:center;font-size:17pt;margin-bottom:5mm;}";

    // 主数据表 — 不用 bold，避免中文重影
    html += ".main-table{width:100%;border-collapse:collapse;margin-bottom:8mm;font-size:12pt;}";
    html += ".main-table th{border:2px solid #000;padding:3mm 2mm;text-align:center;";
    html += "font-size:12pt;background-color:#D6EAF8;}";
    html += ".main-table td{border:2px solid #000;padding:3mm 2mm;text-align:center;font-size:11pt;}";
    html += ".main-table .alt-row{background-color:#F9FBFD;}";
    // 状态颜色：合格=绿，不合格=红（不用 bold）
    html += ".st-pass{color:#1a6e1a;}";
    html += ".st-fail{color:#c0392b;}";

    // 备注
    html += ".note{font-size:11pt;margin:5mm 0;}";
    html += "</style></head><body>";

    // ===== 页面内容 =====
    html += "<div class='page-border'>";

    // 单位名称
    html += "<div class='org-name'>" + m_companyInfo + "</div>";

    // 粗双横线：用表格 + 黑底单元格（QTextDocument 绝对渲染）
    html += "<table width='100%' cellspacing='0' cellpadding='0' border='0' style='margin-bottom:5mm;'>";
    html += "<tr><td style='background-color:#000;font-size:2pt;line-height:2pt;'>&nbsp;</td></tr>";
    html += "<tr><td style='font-size:1pt;line-height:1pt;'>&nbsp;</td></tr>";
    html += "<tr><td style='background-color:#000;font-size:2pt;line-height:2pt;'>&nbsp;</td></tr>";
    html += "</table>";
    html += "<br><br>";

    // 报告标题
    html += "<div class='report-title'>" + m_reportTitle + "</div>";

    // 信息区：每字段一行，绝不换行
    html += "<p style='font-size:12pt;margin:2mm 0;'>设备名称：" + m_deviceName + "</p>";
    html += "<p style='font-size:12pt;margin:2mm 0;'>部件编号：" + m_partNumber + "</p>";
    html += "<p style='font-size:12pt;margin:2mm 0;'>测试人员：" + m_inspector + "</p>";
    html += "<p style='font-size:12pt;margin:2mm 0;'>测试时间：" + m_testDate + "</p>";

    // 测试时间与表格间空行
    html += "<br><br><br>";

    // 主数据表
    html += "<table class='main-table'>";
    html += "<tr><th>序号</th><th>测试项目</th><th>指标范围</th><th>测试结果</th><th>状态</th></tr>";

    for (int i = 0; i < m_reportRows.size(); ++i) {
        const ReportRow &r = m_reportRows[i];
        QString rc = (i % 2 == 1) ? " class='alt-row'" : "";
        QString stClass = (r.status == QString::fromUtf8("合格")) ? "st-pass" : "st-fail";

        html += "<tr" + rc + "><td>" + QString::number(i + 1) + "</td>";
        html += "<td>" + r.itemName + "</td>";
        html += "<td>" + r.specRange + "</td>";
        html += "<td>" + r.testResult + "</td>";
        html += "<td class='" + stClass + "'>" + r.status + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    // 表格下方备注
    html += "<br>";
    html += "<p style='font-size:11pt;margin:5mm 0;'>注：部件状态均「合格」才可判定该检测部件合格。</p>";

    html += "</div>"; // .page-border
    html += "</body></html>";
    return html;
}

bool PDFReportGenerator::generateFormalReport(const QString &fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(12, 12, 12, 12, QPrinter::Millimeter);

    QTextDocument document;
    document.setHtml(generateFormalHTML());
    document.print(&printer);

    qDebug() << "[PDF] 正式检测报告已生成:" << fileName;
    return true;
}

// ===== 综合报告：合并多个报告行，去掉日志 =====
bool PDFReportGenerator::mergeReports(const QString &outputFile,
                                       const QList<ReportRow> &allRows,
                                       const QString &companyInfo,
                                       const QString &deviceName,
                                       const QString &partNumber,
                                       const QString &inspector,
                                       const QString &testDate)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputFile);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);
    printer.setPageMargins(12, 12, 12, 12, QPrinter::Millimeter);

    QString html;
    html += "<!DOCTYPE html><html><head><meta charset='UTF-8'><style>";
    html += "*{margin:0;padding:0;box-sizing:border-box;}";
    html += "body{font-family:SimSun,'Microsoft YaHei',serif;font-size:13pt;line-height:1.6;color:#000;}";
    html += ".page-border{border:2.5px solid #000;padding:8mm 10mm;min-height:92%;}";
    html += ".org-name{text-align:center;font-size:18pt;margin-bottom:2mm;}";
    html += ".report-title{text-align:center;font-size:17pt;margin-bottom:5mm;}";
    html += ".main-table{width:100%;border-collapse:collapse;margin-bottom:8mm;font-size:13pt;}";
    html += ".main-table th{border:2px solid #000;padding:3mm 2mm;text-align:center;";
    html += "font-size:12pt;background-color:#D6EAF8;}";
    html += ".main-table td{border:2px solid #000;padding:3mm 2mm;text-align:center;font-size:11pt;}";
    html += ".main-table .alt-row{background-color:#F9FBFD;}";
    html += ".st-pass{color:#1a6e1a;}";
    html += ".st-fail{color:#c0392b;}";
    html += ".note{font-size:11pt;margin:5mm 0;}";
    html += "</style></head><body>";

    html += "<div class='page-border'>";

    // 单位名称
    html += "<div class='org-name'>" + companyInfo + "</div>";
    // 粗双横线：两行等粗
    html += "<table width='100%' cellspacing='0' cellpadding='0' border='0' style='margin-bottom:5mm;'>";
    html += "<tr><td style='background-color:#000;font-size:2pt;line-height:2pt;'>&nbsp;</td></tr>";
    html += "<tr><td style='font-size:1pt;line-height:1pt;'>&nbsp;</td></tr>";
    html += "<tr><td style='background-color:#000;font-size:2pt;line-height:2pt;'>&nbsp;</td></tr>";
    html += "</table>";
    html += "<br><br>";

    // 标题
    html += "<div class='report-title'>" + QString::fromUtf8("综合告警检测设备  检测报告") + "</div>";

    // 信息区：每字段一行
    html += "<p style='font-size:12pt;margin:2mm 0;'>设备名称：" + deviceName + "</p>";
    html += "<p style='font-size:12pt;margin:2mm 0;'>部件编号：" + partNumber + "</p>";
    html += "<p style='font-size:12pt;margin:2mm 0;'>测试人员：" + inspector + "</p>";
    html += "<p style='font-size:12pt;margin:2mm 0;'>测试时间：" + testDate + "</p>";
    html += "<br><br><br>";

    // 合并表格
    html += "<table class='main-table'>";
    html += "<tr><th>序号</th><th>测试项目</th><th>指标范围</th><th>测试结果</th><th>状态</th></tr>";

    for (int i = 0; i < allRows.size(); ++i) {
        const ReportRow &r = allRows[i];
        QString rc = (i % 2 == 1) ? " class='alt-row'" : "";
        QString stClass = (r.status == QString::fromUtf8("合格")) ? "st-pass" : "st-fail";

        html += "<tr" + rc + "><td>" + QString::number(i + 1) + "</td>";
        html += "<td>" + r.itemName + "</td>";
        html += "<td>" + r.specRange + "</td>";
        html += "<td>" + r.testResult + "</td>";
        html += "<td class='" + stClass + "'>" + r.status + "</td></tr>";
    }
    html += "</table>";

    // 表格下方备注
    html += "<br>";
    html += "<p style='font-size:11pt;margin:5mm 0;'>注：部件状态均「合格」才可判定该检测部件合格。</p>";

    html += "</div>";
    html += "</body></html>";

    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);

    qDebug() << "[PDF] 综合检测报告已生成:" << outputFile;
    return true;
}
