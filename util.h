#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <opencv/cv.h>

class Util : public QObject
{
    Q_OBJECT
public:
    explicit Util(QObject *parent = 0);

signals:

public slots:

public:
    static void write2File(int channel, IplImage* frame);

};

#endif // UTIL_H
