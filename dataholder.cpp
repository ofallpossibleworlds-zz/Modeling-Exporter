#include <QFile>
#include <QTextStream>
#include <QtDebug>
#include "dataholder.h"

//Convenience function to get number in trace.
//This function is passed the string of a trace to operate on, and moves
//through it, discarding everything but the number in order to determine
//the number of the trace.
int DataHolder::getTraceNum(QString trace)
{
    QTextStream checker;
    checker.setString(&trace);
    char check;
    bool reading = false;
    int num = 0;
    checker >> check;
    while (!checker.atEnd()) {
        if (check == '#') {
            reading = true;
        } else if (check == ' ') {
            reading = false;
        } else if (reading) {
            int tempNum = (check-'0');
            num = (num * 10) + tempNum;
        }
        checker >> check;
    }
    return num;
}

//Convenience function to get template for traces.
//This function removes the number from the traces so it can be altered.
QStringList DataHolder::getTraceTemplates()
{
    QTextStream checker;
    QStringList traceTemplates;
    for (int i = m_signalHeader.length(); i > 0; i--) {
        checker.setString(&m_traces[m_traces.length()-i]);
        QString temp;
        char check;
        bool reading = true;
        while (!checker.atEnd()) {
            checker >> check;
            if  (check == '#') {
                reading = false;
                temp.append(check);
            } else if (check == ' ') {
                reading = true;
                temp.append(check);
            } else if (reading) {
                temp.append(check);
            }

        }
        traceTemplates.append(temp);
    }
    return traceTemplates;
}

//Convenience function to write new trace name.
//This function writes new trace numbers for the average function.
QStringList DataHolder::writeTraces(QStringList templates, int traceNum)
{
    QTextStream checker;
    QStringList writtenTraces;
    for (int i = 0; i < templates.length(); i++) {
        checker.setString(&templates[i]);
        char check;
        QString temp;
        while (!checker.atEnd()) {
            checker >> check;
            temp.append(check);
            if (check == '#') {
                QString tempNum = QString("%1").arg(traceNum);
                temp.append(tempNum);
            }
        }
        writtenTraces.append(temp);
    }
    return writtenTraces;
}

//Fixes header line count.
//This function reads through the header and fixes the number of columns
//which is stored in it so the data can be read by clampfit.
void DataHolder::fixHeader()
{
    QTextStream fixer;
    QString temp;
    fixer.setString(&m_header);
    temp = fixer.readLine();
    temp.append('\n');
    char check;
    fixer >> check;
    bool reading = false;
    bool written = false;
    int num = 0;
    while (check != '\n') {
        if (check == '\t') {
            reading = true;
            temp.append(check);
        } else if (check == ' ') {
            reading = false;
            if (!written) {
                QString tempNum = QString("%1").arg(m_traces.length());
                temp.append(tempNum);
                written = true;
            }
            temp.append(check);
        } else if (reading) {
            int tempNum = (check-'0');
            num = (num * 10) + tempNum;
        } else {
            temp.append(check);
        }
        fixer >> check;
    }
    while (!fixer.atEnd()) {
        temp.append(check);
        fixer >> check;
    }
    m_header = temp;
}

//Convenience function to find index of first active trace.
int DataHolder::findActiveTrace(int signalNum)
{
    for (int i = 0; i < m_tracesActive.length(); i++) {
        if (signal()[i].contains(signalHeader()[signalNum-1])) {
            if (m_tracesActive[i]) {
                return i;
            }
        }
    }
    return -1;
}

//Finds single voltage.
//This method reads through the data of a trace and saves areas where the
//data is flat, and returns them. It ignores areas where the data is changing
//rapidly, as including them can lead to vastly different lists, which hinders
//the process of other functions for automated voltage finding.
//
//PLEASE NOTE:
//If the automated voltage finder needs the sensativity adjusted, please alter
//it in the dataholder.h file, and not here. Look for "VOLTAGE_VARIENCE" and
//"CHANGING_VARIENCE", as well as "BACKWARDS_CHECK" which determines how many
//prior data points the voltage is compared to in order to determine if the
//voltage is changing.

//PLEASE NOTE:
//If the automated voltage finder needs the sensativity adjusted, please alter
//it in the dataholder.h file, and not here. Look for "VOLTAGE_VARIENCE" and
//"CHANGING_VARIENCE".
QList<float> DataHolder::findVoltage(int traceNum, int signalNum)
{
    QList<float> voltages;
    for (int i = 1; i < signal().length(); i++) {
        if (signal()[i].contains(signalHeader()[signalNum])) {
            if (i == traceNum) {
                voltages.append(m_times[i][0]);
                bool similar = false;
                int index = 0;
                bool debugHelper = false;
                for (int j = 0; j < m_times[i].length(); j++) {
                    if ((m_times[i][j]
                         > voltages[index]-VOLTAGE_VARIENCE)
                            && m_times[i][j]
                            < voltages[index]+VOLTAGE_VARIENCE){
                        similar = true;
                        voltages[index] =
                                ((voltages[index]+m_times[i][j])/2);
                    }
                    if (!similar) {
                        bool notChanging = true;
                        bool checked = false;

                        for (int k = j-BACKWARDS_CHECK; k < j; k++) {
                            if (k > 0 && k < m_times[i].length()) {
                                checked = true;
                                if (m_times[i][j] < 1 && m_times[i][j] > -1) {
                                    debugHelper = true;
                                }
                                if (m_times[i][j] < 0) {
                                    if (((m_times[i][j]
                                          < m_times[i][k]-CHANGING_VARIENCE)
                                         || m_times[i][j]
                                         > m_times[i][k]+CHANGING_VARIENCE)) {

                                        notChanging = false;
                                        break;
                                    }
                                } else {
                                    if (((m_times[i][j]
                                          > (m_times[i][k]+CHANGING_VARIENCE)
                                          || m_times[i][j]
                                          < m_times[i][k]-CHANGING_VARIENCE))){

                                        notChanging = false;
                                        break;
                                    }
                                }
                            }
                        }
                        if (notChanging && checked) {
                            voltages.append(m_times[i][j]);
                            index++;
                        }
                    }
                    similar = false;
                }
            }
        }
    }
    return voltages;
}

//Compares lists of voltages to isolate those that differ.
//This (recursive) function compares lists (ONLY OF THE SAME LENGTH. TO ENSURE
//THIS REQUIREMENT IS MET, PLEASE USE "voltageFixer".) of voltages. It reads
//through the lists, and if the voltages don't match within a certain
//acceptable varience, it returns them, on the basis that most of the files
//only have significantly different varience at the point where their voltage
//splits.
//
//PLEASE NOTE:
//If the automated voltage finder needs the sensativity adjusted, please alter
//it in the dataholder.h file, and not here. Look for "VOLTAGE_VARIENCE" and
//"CHANGING_VARIENCE".
QList<float> DataHolder::voltageChecker(QList<QList<float>> voltageList)
{
    QList<float> temp;

    //Check lists
    bool listMatch = true;
    for (int i = 0; i < voltageList.length(); i++) {
        temp.append(voltageList[i][0]);
        for (int j = i; j < voltageList.length(); j++) {
            if (voltageList[i][0]
                    < voltageList[j][0]-VOLTAGE_VARIENCE
                    || voltageList[i][0]
                    > voltageList[j][0]+VOLTAGE_VARIENCE) {
                listMatch = false;
            }
        }
    }
    if (voltageList[0].length() <= 1) {
        if (listMatch) {
            temp.clear();
        }
        return temp;
    } else {
        if (listMatch) {
            for (int i = 0; i < voltageList.length(); i++) {
                voltageList[i].removeFirst();
            }
            return voltageChecker(voltageList);
        } else {
            return temp;
        }
    }
}

//Makes lists of voltages the same length.
//This is a convenience function that can be called to ensure all lists are the
//same length so that the voltage checker can function properly.
QList<QList<float> > DataHolder::voltageFixer(QList<QList<float>> voltageList)
{
    QList<QList<float>> temp(voltageList);
    for (int i = 0; i < temp.length(); i++) {
        for (int j = 0; j < temp.length(); j++) {
            for (int k = 0; k < temp[i].length(), k < temp[j].length(); k++) {
                if (temp[i].length() < temp[j].length()) {
                    if (temp[i][k] < temp[j][k]-VOLTAGE_VARIENCE
                            ||temp[i][k] > temp[j][k]+VOLTAGE_VARIENCE) {
                        if (k > 0) {
                            temp[i].insert(k, temp[i][k-1]);
                        } else {
                            temp[i].insert(k, temp[i][k]);
                        }
                    }
                }
            }
        }
    }
    return temp;
}

//Constructor.
DataHolder::DataHolder()
{
}

//Destructor.
DataHolder::~DataHolder()
{
}

//Copy Constructor.
DataHolder::DataHolder(const DataHolder &rhs):
    m_fileName(rhs.fileName()),
    m_header(rhs.header()),
    m_signalHeader(rhs.signalHeader()),
    m_signal(rhs.signal()),
    m_traces(rhs.traces()),
    m_tracesActive(rhs.tracesActive()),
    m_times(rhs.times())
{
    //Nothing to do.
}

//Equality operator.
const DataHolder &DataHolder::operator=(const DataHolder &rhs)
{
    if (this == &rhs) {
        return *this;
    } else {
        DataHolder temp(rhs);
        std::swap(m_fileName, temp.fileName());
        std::swap(m_header, temp.header());
        std::swap(m_signalHeader, temp.signalHeader());
        std::swap(m_signal, temp.signalHeader());
        std::swap(m_traces, temp.traces());
        std::swap(m_tracesActive, temp.tracesActive());
        std::swap(m_times, temp.times());
        return *this;
    }
}

//Gets File URL
QString DataHolder::fileName() const
{
    return m_fileName;
}

//Gets data
QList<QList<float>> DataHolder::times() const
{
    return m_times;
}

//Gets Traces
QList<QString> DataHolder::traces() const
{
    return m_traces;
}

//Gets unused header
QString DataHolder::header() const
{
    return m_header;
}

//Gets Signal Names for Table
QStringList DataHolder::signal() const
{
    return m_signal;
}

//Gets Signal Names
QStringList DataHolder::signalHeader() const
{
    return m_signalHeader;
}

//Gets active traces.
QList<bool> DataHolder::tracesActive() const
{
    return m_tracesActive;
}

//Loads all data from file if m_fileName is already declared
void DataHolder::loadData() {
    //Loads text file
    if (m_fileName.endsWith(".txt") || m_fileName.endsWith(".atf")) {
        //Creates file and ensures it exists.
        QFile file(m_fileName);
        if (file.exists()) {
            //Opens file.
            if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
                qDebug() << "Error";
                return;
            }
            QTextStream inFile(&file);

            //Sets Header
            m_header = "";
            for (size_t i = 0; i < 8; i++) {
                m_header.append(inFile.readLine()).append("\n");
            }

            //Sets signal Header
            char readIn;
            inFile >> readIn;
            bool open = false;
            int counter = 0;
            QString title = "";
            while (readIn != '\n') {
                if (open) {
                    if (readIn == ',' || readIn == '"') {
                        m_signalHeader.append(title);
                        counter++;
                        title = "";
                    } else if (readIn == '=') {
                        //Do nothing
                    } else {
                        title = title + readIn;
                    }
                } else {
                    if (readIn == '=') {
                        open = true;
                    }
                }
                inFile >> readIn;
            }

            //Sets signal
            readIn;
            inFile >> readIn;
            open = false;
            counter = 0;
            title = "";
            while (readIn != '\n') {
                if (open) {
                    if (readIn == '"') {
                        open = false;
                        m_signal.append(title);
                        counter++;
                        title = "";
                    } else if (readIn == '=') {
                        //Do nothing
                    } else {
                        title = title + readIn;
                    }
                } else {
                    if (readIn == '"') {
                        open = true;
                    }
                }
                inFile >> readIn;
            }

            //Sets traces
            inFile >> readIn;
            open = false;
            counter = 0;
            title = "";
            while (readIn != '\n') {
                if (open) {
                    if (readIn == '"') {
                        open = false;
                        m_traces.append(title);
                        counter++;
                        title = "";
                    } else if (readIn == '=') {
                        //Do nothing
                    } else {
                        title = title + readIn;
                    }
                } else {
                    if (readIn == '"') {
                        open = true;
                    }
                }
                inFile >> readIn;
            }

            //Loads data
            float data = 0;
            while (!inFile.atEnd()) {
                for (int i = 0; i < m_traces.length(); i++) {
                    inFile >> data;
                    if (!inFile.atEnd()){
                        if (m_times.size() < m_traces.length()) {
                            m_times.append(*(new QList<float>));;
                        }
                        m_times[i].append(data);
                    }
                }
            }

            for (int i = 0; i < m_traces.length(); i++) {
                m_tracesActive.append(true);
            }
        } else {
            qDebug() << "ERROR LOADING FILE";
        }
    }
}

//Overload of loadData for new URL
void DataHolder::loadData(QString urlName) {
    m_fileName = urlName;
    loadData();
}

//Saves data to given file.
void DataHolder::saveData(QString urlName)
{
    m_fileName = urlName;
    QFile file(urlName);
    if (!file.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text)) {
        qDebug() << "ERROR SAVING FILE";
        return;
    }
    QTextStream outFile(&file);

    //Writes header
    outFile << m_header;

    //Writes signalHeader
    outFile << "\n\"SignalsExported=" << m_signalHeader[0];
    for (int i = 1; i < m_signalHeader.length(); i++) {
        outFile << "," << m_signalHeader[i];
    }
    outFile << "\"\n";

    //Writes signal
    outFile << "\"" << m_signal[0] << "=\"\t";
    for (int i = 1; i < m_signal.length(); i++) {
        outFile << "\"" << m_signal[i] << "\"\t";
    }
    outFile << "\n";

    //Writes traces
    for (int i = 0; i < m_traces.length(); i++) {
        outFile << "\"" << m_traces[i] << "\"\t";
    }
    outFile << "\n";

    //Writes data
    for (int i = 0; i < m_times[0].length(); i++) {
        for (int j = 0; j < m_times.length(); j++) {
            outFile << m_times[j][i] << "\t";
        }
        outFile << "\n";
    }
    file.close();
}

//Combines two DataHolders.
//
//PLEASE NOTE: This method can only accurately combine very similar data.
//Attempts to use this method to combine dissimilar data are likely to fail,
//and may negatively affect the program's function, or the base file if saved.
//It is protected within the application's normal functionality and should
//function safely within the paramenters it recieves.
void DataHolder::combine(const DataHolder &other)
{
    for (int i = 1; i < other.signal().length(); i++) {
        m_signal.append(other.signal()[i]);
    }
    int num = getTraceNum(m_traces.last());
    int traceSets
            = (other.traces().length()-1)/(other.signalHeader().length()-1);
    num++;

    QStringList oldTraces = getTraceTemplates();
    QStringList newTraces;

    for (int i = 0; i < traceSets; i++) {
        newTraces.append(writeTraces(oldTraces, num));
        num++;
    }
    m_traces.append(newTraces);
    QList<bool> temp(other.tracesActive());
    temp.removeFirst();
    m_tracesActive.append(temp);

    int repetitions = (other.times().length());
    for (int i = 1; i < repetitions; i++) {
        m_times.append(other.times()[i]);
    }

    fixHeader();

}

//Averages signals of active traces.
void DataHolder::average(int signalNum) {
    //signalNum is passed 0-based index
    signalNum++;

    int firstTrace = findActiveTrace(signalNum);

    if (firstTrace != -1) {

        //Averages all active traces.
        QList<float> temp;
        for (int j = 0; j < m_times[0].length(); j++) {
            float average = 0;
            int divisor = 0;
            for (int i = 1; i < signal().length(); i++) {
                if (signal()[i].contains(signalHeader()[signalNum-1])
                        && tracesActive()[i]) {
                    average = average + m_times[i][j];
                    divisor++;
                }
            }
            average = average/divisor;
            temp.append(average);
        }

        //Removes averaged traces.
        for (int i = firstTrace+1; i < signal().length(); i++) {
            if (signal()[i].contains(signalHeader()[signalNum-1])
                    && tracesActive()[i]) {
                m_signal.removeAt(i);
                m_traces.removeAt(i);
                m_times.removeAt(i);
                m_tracesActive.removeAt(i);
                i--;
            }
        }

        //Fixes other items.
        m_times.replace(firstTrace, temp);

        //m_signalHeader[signalNum-1].append("[Averaged]");

        m_signal[signalNum].append("[Averaged]");

        fixHeader();
    }
}

//Attempts to find voltage of trace.
//
//PLEASE NOTE: While this function works very well, it's not perfect, and will
//not always find the correct traces. Please be careful when using this
//function, and check that the correct voltages are returned on the prompt.
QList<float> DataHolder::findVoltages(int signalNum)
{
    QList<QList<float>> temp;
    for (int i = signalNum; i < signal().length(); i++) {
        if (signal()[i].contains(signalHeader()[signalNum])
                && tracesActive()[i]) {
            temp.append(findVoltage(i, signalNum));
        }
    }
    temp = voltageFixer(temp);
    return voltageChecker(temp);
}

//Sets active traces.
void DataHolder::setTracesActive(QList<bool> tracesActive)
{
    if (m_tracesActive == tracesActive)
        return;

    m_tracesActive = tracesActive;
    emit tracesActiveChanged(tracesActive);
}


//Sets File Name
void DataHolder::setFileName(QString fileName)
{
    if (m_fileName == fileName)
        return;

    m_fileName = fileName;
    emit fileNameChanged(fileName);
}

//Sets Data
void DataHolder::settimes(QList<QList<float>> times)
{
    if (m_times == times)
        return;

    m_times = times;
    emit timesChanged(times);
}

//Sets Traces
void DataHolder::setTraces(QList<QString> traces)
{
    if (m_traces == traces)
        return;

    m_traces = traces;
    emit tracesChanged(traces);
}

//Sets Unused Header
void DataHolder::setheader(QString header)
{
    if (m_header == header)
        return;

    m_header = header;
    emit headerChanged(header);
}

//Sets Signal
void DataHolder::setsignal(QStringList signal)
{
    if (m_signal == signal)
        return;

    m_signal = signal;
    emit signalChanged(signal);
}

//Sets signal header
void DataHolder::setsignalHeader(QStringList signalHeader)
{
    if (m_signalHeader == signalHeader)
        return;

    m_signalHeader = signalHeader;
    emit signalHeaderChanged(signalHeader);
}
