#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <QSet>
#include <QTextStream>
#include <QHash>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList args = a.arguments();
    QString vw_filename = args[1];
    QString csv_filename = (args.count() < 3)? vw_filename+".csv":args[2];

    QFile vw_f;
    vw_f.setFileName(vw_filename);
    if (!vw_f.open(QIODevice::ReadOnly)) return -1;

    QFile csv_f;
    csv_f.setFileName(csv_filename);
    if (!csv_f.open(QIODevice::WriteOnly|QIODevice::Truncate)) return -2;

    QTextStream vw_s;
    vw_s.setDevice(&vw_f);

    QSet<QString> features;
    int max_header_cnt = 1; //label only

    while(!vw_s.atEnd())
    {
        QStringList buf = vw_s.readLine().split(' ', QString::SkipEmptyParts);
        bool header_end = false;
        QString last_namespace;
        for (int i = 1; i < buf.count(); i++)
        {
            const QString& word = buf[i];
            if (word.contains('|'))
            {
                if (!header_end)
                {
                    header_end = true;
                    if (i > max_header_cnt) max_header_cnt = i;
                }
                last_namespace = word.right(word.count() - word.lastIndexOf('|') -1);
            } else
                if (header_end) {
                    QString fetaure = last_namespace + '^'+ word.split(':')[0];
                    features.insert(fetaure);
                }
        }
    }

    QString header = "label";
    for (int i = 2; i <= max_header_cnt; i++) header += ",hdr_val"+QString::number(i);


    QStringList ft = QStringList::fromSet(features);
    qSort(ft.begin(), ft.end());
    header += ',' + ft.join(',');

    QHash<QString, int> ft_hash;
    for (int i = 0; i < ft.count(); i++)
        ft_hash[ft[i]] = max_header_cnt+i;


    if (!vw_s.seek(0)) return -3;

    QTextStream csv_s;
    csv_s.setDevice(&csv_f);

    csv_s << header << '\n';

    QStringList res; res.reserve(max_header_cnt + ft.count());
    for (int i = 0; i < max_header_cnt + ft.count(); i++)
        res.append(QString());
    ft.clear();

    while(!vw_s.atEnd())
    {
        QStringList buf = vw_s.readLine().split(' ', QString::SkipEmptyParts);
        bool header_end = false;
        QString last_namespace;

        for (int i = 0; i < res.count(); i++)
            if (!res[i].isEmpty()) res[i] = "0";

        for (int i = 0; i < buf.count(); i++)
        {
            const QString& word = buf[i];
            if (word.contains('|'))
            {
                if (!header_end)
                {
                    header_end = true;
                    if (i > max_header_cnt) max_header_cnt = i;
                }
                last_namespace = word.right(word.count() - word.lastIndexOf('|') -1);
            } else
                if (header_end) {
                    QStringList wl = word.split(':');
                    QString feature = last_namespace + '^'+ wl[0];
//                    Q_ASSERT(ft_hash[feature]!=02);
                    res[ft_hash[feature]] = (wl.count() < 2 || wl[1].isEmpty())?"1":wl[1];
                } else res[i] = word;
        }

        csv_s << res.join(',') << '\n';
    }

    return 0;
}
