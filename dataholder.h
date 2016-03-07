#ifndef DATAHOLDER_H
#define DATAHOLDER_H

#include <QObject>
#include <QStringList>

class DataHolder : public QObject
{
    Q_OBJECT
    //The definition of the filename.
    Q_PROPERTY(QString fileName
               READ fileName
               WRITE setFileName
               NOTIFY fileNameChanged)
    //The definition of the header.
    Q_PROPERTY(QString header
               READ header
               WRITE setheader
               NOTIFY headerChanged)
    //The definition of the signal header.
    Q_PROPERTY(QStringList signalHeader
               READ signalHeader
               WRITE setsignalHeader
               NOTIFY signalHeaderChanged)
    //The definition of the signals.
    Q_PROPERTY(QStringList signal
               READ signal
               WRITE setsignal
               NOTIFY signalChanged)
    //The definition of the traces.
    Q_PROPERTY(QList<QString> traces
               READ traces
               WRITE setTraces
               NOTIFY tracesChanged)
    //The definition of the active traces.
    Q_PROPERTY(QList<bool> tracesActive
               READ tracesActive
               WRITE setTracesActive
               NOTIFY tracesActiveChanged)
    //The definition of the data.
    Q_PROPERTY(QList<QList<float>> times
               READ times
               WRITE settimes
               NOTIFY timesChanged)


    //A variable denoting how much the voltage can vary and be considered
    //similar enough to not register as a new voltage.
    const float VOLTAGE_VARIENCE = (float)(2.5);

    //The amount a by which the voltage must change to have the voltage be
    //considered "actively changing" which is ignored by the program.
    const float CHANGING_VARIENCE = (float)(5.0);

    //How many data points backwards the program checks to see if the voltage
    //is changing.
    const int BACKWARDS_CHECK = 10;

private:
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////VARIABLES///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    //File URL
    QString m_fileName;

    //Unused header
    QString m_header;

    //Signal Names
    QStringList m_signalHeader;

    //Signal Names for Table
    QStringList m_signal;

    //Traces
    QList<QString> m_traces;

    //Traces active.
    QList<bool> m_tracesActive;

    //Data
    QList<QList<float>> m_times;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////FUNCTIONS///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    //Convenience function to get number in trace.
    int getTraceNum(QString trace);

    //Convenience function to get template for traces.
    QStringList getTraceTemplates();

    //Convenience function to write new trace name.
    QStringList writeTraces(QStringList templates, int traceNum);

    //Fixes header line count.
    void fixHeader();

    //Convenience function to find index of first active trace.
    int findActiveTrace(int signalNum);

    //Finds single voltage.
    QList<float> findVoltage(int traceNum, int signalNum);

    //Compares lists of voltages to isolate those that differ.
    QList<float> voltageChecker(QList<QList<float>> voltageList);

    //Makes lists of voltages the same length.
    QList<QList<float>> voltageFixer(QList<QList<float>> voltageList);

public:

    //Constructor.
    DataHolder();

    //Destructor.
    ~DataHolder();

    //Copy Constructor.
    DataHolder(const DataHolder & rhs);

    //Equality operator.
    const DataHolder& operator= (const DataHolder& rhs);

    //Gets File URL
    QString fileName() const;

    //Gets data
    QList<QList<float>> times() const;

    //Gets Traces
    QList<QString> traces() const;
    
    //Gets unused header
    QString header() const;

    //Gets Signal Names for Table
    QStringList signal() const;

    //Gets Signal Names
    QStringList signalHeader() const;

    //Gets active traces.
    QList<bool> tracesActive() const;


    //Loads all data from file if m_fileName is already declared
    Q_INVOKABLE void loadData();

    //Overload of loadData for new URL
    Q_INVOKABLE void loadData(QString urlName);

    //Saves data to given file.
    Q_INVOKABLE void saveData(QString urlName);

    //Combines two DataHolders.
    Q_INVOKABLE void combine(const DataHolder & other);

    //Averages signals of active traces.
    Q_INVOKABLE void average(int signalNum);

    //Attempts to find voltage of trace.
    Q_INVOKABLE QList<float> findVoltages(int signalNum);

public slots:
    //Sets active traces.
    void setTracesActive(QList<bool> tracesActive);

private slots:
    //Sets File Name
    void setFileName(QString fileName);

    //Sets Data
    void settimes(QList<QList<float>> times);

    //Sets Traces
    void setTraces(QList<QString> traces);
    
    //Sets Unused Header
    void setheader(QString header);

    //Sets Signal
    void setsignal(QStringList signal);

    //Sets signal header
    void setsignalHeader(QStringList signalHeader);

signals:
    //Signals if the file URL has been changed
    void fileNameChanged(QString fileName);

    //Signals if times (the data) have been changed
    void timesChanged(QList<QList<float>> times);

    //Signals if traces have been changed
    void tracesChanged(QList<QString> traces);

    //Signals if unused header has been changed
    void headerChanged(QString header);

    //Signals if signal has been changed
    void signalChanged(QStringList signal);

    //Signals if signalHeader has been changed
    void signalHeaderChanged(QStringList signalHeader);

    //Signals if active traces have been changed
    void tracesActiveChanged(QList<bool> tracesActive);
};

#endif // DATAHOLDER_H
