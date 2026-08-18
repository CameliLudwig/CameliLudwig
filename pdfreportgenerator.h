#ifndef PDFREPORTGENERATOR_H
#define PDFREPORTGENERATOR_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QMap>

// 原始表格测试项目（保留兼容）
struct TestItem {
    QString itemName;
    QString specification;
    QString testResult;
    QString unit;
    QString remarks;

    TestItem() = default;
    TestItem(const QString &name, const QString &spec, const QString &result,
             const QString &u, const QString &rem)
        : itemName(name), specification(spec), testResult(result), unit(u), remarks(rem) {}
};

// 检测报告表格行（图片格式：序号|测试项目|指标范围|测试结果|状态）
struct ReportRow {
    QString itemName;    // 测试项目
    QString specRange;   // 指标范围
    QString testResult;  // 测试结果
    QString status;      // 状态：正常/异常/告警

    ReportRow() = default;
    ReportRow(const QString &name, const QString &spec, const QString &result, const QString &st)
        : itemName(name), specRange(spec), testResult(result), status(st) {}
};

class PDFReportGenerator
{
public:
    PDFReportGenerator();
    ~PDFReportGenerator();

    // 基本信息设置
    void setDeviceName(const QString &name);
    void setPartNumber(const QString &partNumber);
    void setTestDate(const QString &date);
    void setInspector(const QString &inspector);
    void setCompanyInfo(const QString &company);
    void setReportTitle(const QString &title);

    // 检测报告表格行
    void addReportRow(const ReportRow &row);
    void clearReportRows();
    QList<ReportRow> reportRows() const { return m_reportRows; }

    // 原始表格（保留兼容）
    void setTableHeaders(const QStringList &headers);
    void setTableColumnWidths(const QList<int> &widths);
    void addTestItem(const TestItem &item);
    void removeTestItem(int index);
    void updateTestItem(int index, const TestItem &item);
    TestItem getTestItem(int index) const;
    QList<TestItem> getAllTestItems() const;
    void clearAllTestItems();

    // 表格样式设置
    void setTableBorderWidth(int width);
    void setTableCellPadding(int padding);
    void setHeaderBackgroundColor(const QString &color);
    void setAlternateRowColor(const QString &color);

    // 页面设置
    void setPageMargins(int left, int top, int right, int bottom);
    void setPageOrientation(bool landscape = false);

    // 生成原始表格PDF
    bool generatePDF(const QString &fileName);

    // 生成正式检测报告PDF（按图片格式）
    bool generateFormalReport(const QString &fileName);

    // 综合报告：合并多个报告为一个（去掉日志，只保留结果表）
    static bool mergeReports(const QString &outputFile,
                             const QList<ReportRow> &allRows,
                             const QString &companyInfo,
                             const QString &deviceName,
                             const QString &partNumber,
                             const QString &inspector,
                             const QString &testDate);

    // 获取当前设置
    QString getDeviceName() const { return m_deviceName; }
    QString getPartNumber() const { return m_partNumber; }
    QString getTestDate() const { return m_testDate; }
    QString getInspector() const { return m_inspector; }
    QStringList getTableHeaders() const { return m_tableHeaders; }

private:
    // 基本信息
    QString m_deviceName;
    QString m_partNumber;
    QString m_testDate;
    QString m_inspector;
    QString m_companyInfo;

    // 原始表格
    QList<TestItem> m_testItems;
    QStringList m_tableHeaders;
    QList<int> m_columnWidths;
    int m_borderWidth;
    int m_cellPadding;
    QString m_headerBgColor;
    QString m_alternateRowColor;

    // 页面设置
    int m_marginLeft, m_marginTop, m_marginRight, m_marginBottom;
    bool m_landscape;

    // 报告标题
    QString m_reportTitle;

    // 检测报告表格行（新格式）
    QList<ReportRow> m_reportRows;

    // 私有方法
    QString generateHTMLContent();
    QString generateFormalHTML();
    void initializeDefaults();
};

#endif // PDFREPORTGENERATOR_H
