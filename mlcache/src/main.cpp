#include <QCoreApplication>
#include <QCommandLineParser>
#include "mlcache.h"
#include "mlcommon.h"
#include <QTextStream>

// MLUtils: copied from prv

uint64_t parseDuration(const QString& duration) {
    uint64_t result = 0;
    // d, h, m, s
    static QList<QPair<char, int> > factors = {
        {'d', 24*3600},
        {'h', 3600},
        {'m', 60},
        {'s', 1}
    };
    QStringList components = duration.split(' ');
    int factorIdx = 0;
    while (factorIdx < factors.length()) {
        if (components.isEmpty())
            break;
        QString component = components.first();
        if (component.endsWith(factors[factorIdx].first)) {
            component.chop(1);
            result += component.toLongLong()*factors[factorIdx].second;
            components.removeFirst();
        } else
            factorIdx++;
    }

    if (result == 0)
        result = duration.toLongLong();
    return result;
}

namespace MLUtils {
class ApplicationCommand {
public:
    enum { ShowHelp = -1000 };
    virtual QString commandName() const = 0;
    virtual QString description() const { return QString(); }
    virtual void prepareParser(QCommandLineParser&) {}
    virtual ~ApplicationCommand() {}
    virtual int execute(const QCommandLineParser&) { return 0; }
};

class ApplicationCommandParser {
public:
    ApplicationCommandParser() {}
    bool process(const QCoreApplication& app)
    {
        m_result = 0;
        QCommandLineParser parser;
        parser.addPositionalArgument("command", "Command to execute");
        parser.addHelpOption();
        // TODO: build description from list of commands
        QString desc;
        const QLatin1Char nl('\n');
        desc += nl;
        desc += "Commands:" + nl;
        int longestCommandName = 0;
        foreach (ApplicationCommand* cmd, m_commands) {
            longestCommandName = qMax(longestCommandName, cmd->commandName().size());
        }

        foreach (ApplicationCommand* cmd, m_commands) {
            desc += cmd->commandName();
            if (cmd->description().isEmpty()) {
                desc += nl;
            }
            else {
                int spaces = longestCommandName - cmd->commandName().size() + 1;
                desc += QString(spaces, ' ');
                desc += '\t' + cmd->description() + nl;
            }
        }
        parser.setApplicationDescription(desc);
        parser.parse(app.arguments());
        if (parser.positionalArguments().isEmpty()) {
            parser.showHelp(1);
            return false;
        }
        QString command = parser.positionalArguments().first();
        foreach (ApplicationCommand* cmd, m_commands) {
            if (cmd->commandName() == command) {
                parser.clearPositionalArguments();
                parser.setApplicationDescription(QString());
                cmd->prepareParser(parser);
                parser.process(app);
                m_result = cmd->execute(parser);
                if (m_result == ApplicationCommand::ShowHelp)
                    parser.showHelp(m_result);
                return true;
            }
        }
        // command not found
        parser.showHelp(0);
        return false;
    }

    ~ApplicationCommandParser()
    {
        qDeleteAll(m_commands);
    }

    void addCommand(ApplicationCommand* cmd)
    {
        m_commands << cmd;
    }
    int result() const { return m_result; }

private:
    QList<ApplicationCommand*> m_commands;
    int m_result = 0;
};
}

namespace MLCacheCommands {

class SimpleCommand : public MLUtils::ApplicationCommand {
public:
    SimpleCommand(const QString& n, const QString& d = QString()) : name(n), desc(d) {
    }
    QString commandName() const { return name; }
    QString description() const { return desc; }
private:
    QString name, desc;
};

class GCCommand : public SimpleCommand {
public:
    GCCommand() : SimpleCommand("gc", "Garbage collect cache") {}

    void prepareParser(QCommandLineParser& parser) {
//        parser.addHelpOption();
        parser.addOption(QCommandLineOption("verbose", "Be verbose"));
    }

    int execute(const QCommandLineParser& parser) {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        if (!args.isEmpty()) {
            return ShowHelp; // show help
        }

        QString temporary_path = MLUtil::tempPath();
        MLCache cache(temporary_path);
        cache.gc();
        return 0;
    }
};

class LSCommand : public SimpleCommand {
public:
    LSCommand() : SimpleCommand("ls", "List files in cache"){}
    int execute(const QCommandLineParser& parser) {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        QString temporary_path = MLUtil::tempPath();
        MLCache cache(temporary_path);
        QStringList files = cache.entryList();
        QTextStream qout(stdout, QIODevice::WriteOnly|QIODevice::Text);
        int idx = 0;
        foreach (QString file, files) {
            qout << file;
            idx = idx+1 % 8;
            if (idx == 0) qout << endl; else qout << '\t';
        }
        return 0;
    }
};

class CreateCommand : public SimpleCommand {
public:
    CreateCommand() : SimpleCommand("create", "Create a new temporary file in cache") {}
    void prepareParser(QCommandLineParser& parser) {
        // name
        parser.addPositionalArgument("fileName", "Name of the file to be created");
        // --duration
        parser.addOption({"duration", "File validity duration", "duration", 0});
        // --pid
        //parser.addOption({"pid", "Process identifier "});
    }
    int execute(const QCommandLineParser& parser)
    {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        QString temporary_path = MLUtil::tempPath();
        MLCache cache(temporary_path);
        foreach(QString name, args) {
            MLCacheFile* file = cache.file(name, parseDuration(parser.value("duration")));
            delete file; // prevent memory leak
        }
        return 0;
    }
};

class UpdateCommand : public SimpleCommand {
public:
    UpdateCommand() : SimpleCommand("update", "Update file in cache") {}
    void prepareParser(QCommandLineParser& parser) {
        parser.addPositionalArgument("fileName", "Name of the file to be updated");
        // --duration
        parser.addOption({"duration", "File validity duration", "duration", 0});
        // --pid
        parser.addOption({"pid", "Process identifier "});
    }
    int execute(const QCommandLineParser &parser) {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        QString temporary_path = MLUtil::tempPath();
        MLCache cache(temporary_path);
        uint64_t duration = parser.isSet("duration") ? parseDuration(parser.value("duration")) : 0;
        uint32_t pid = parser.isSet("pid") ? parser.value("pid").toInt() : 0;
        foreach(QString name, args) {
            MLCacheFile* file = cache.file(name);
            if (duration)
                file->setDuration(duration);
            if (pid)
                file->setPid(pid);
            delete file; // prevent memory leak
        }
        return 0;
    }
};

}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    // initialize cache manager

    MLUtils::ApplicationCommandParser cmdParser;
    cmdParser.addCommand(new MLCacheCommands::GCCommand);
    cmdParser.addCommand(new MLCacheCommands::LSCommand);
    cmdParser.addCommand(new MLCacheCommands::CreateCommand);
    if (!cmdParser.process(app)) {
        return cmdParser.result();
    }
    return cmdParser.result();
}
